/*
 * Copyright (c) 2011, Karol Trzcionka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Sockets.h"
#include "Threads.h"
#include "HttpReq.h"
#include "HttpResponse.h"
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sstream>
#include <ctime>
#include <cstring>
#include <csignal>
#define min(a,b) ((a<b)?a:b)

Socket::~Socket(){
	close(socketfd);
	std::cerr<<"close socket\n";
}

Socket::Socket(int _socket):socketfd(_socket){
	std::cerr<<"open socket\n";
}

int Socket::send(const std::string &in){
	size_t it=0;
	while(it < in.size()){
		it+=::send(socketfd, in.substr(it).c_str(), in.size()-it,MSG_NOSIGNAL);
	}
	return  0;
}

std::string Socket::recvmess(int length){
	std::string ret("");
	char buf[255];
	int rr,size=0;
	while(size<length){
	rr = recv(socketfd, buf, min(sizeof buf,length-size), 0);
	ret.append(buf, rr);
	size+=rr;
	}
	return ret;
}

int Socket::send(std::iostream &from,int howmany){
	int it=0, it2,s;
	char buf[255];
	if(howmany>0){
		while(it<howmany){
			it2=min(howmany-it,255);
			from.read(buf,it2);
			while(it2>0){
				s=::send(socketfd, buf, it2, (it2<=(howmany-it))?0:MSG_MORE|MSG_NOSIGNAL);
				if(s<0){
					return 100;
				}
				it2-=s;
				it+=s;
			}
		}
	}
	return 0;
}

bool operator<(const Connection &a,const Connection &b){
	return a.timestamp<b.timestamp;
}
const char Connection::endofhead[4]={'\r','\n','\r','\n'}; // manually set to avoid null on the end

Connection::Connection(int _socket):Socket(_socket),timestamp(time(NULL)){	
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_NORMAL);
	pthread_mutex_init(&processing,&attr);
	sockaddr_storage their_addr;
	char ipstr[INET6_ADDRSTRLEN];
	//socklen_t sin_size=INET6_ADDRSTRLEN;
	socklen_t sin_size=sizeof their_addr;
	void *addr;
	getpeername(_socket,(sockaddr *) &their_addr, &sin_size);
	if (their_addr.ss_family == AF_INET) { // IPv4
		sockaddr_in *ipv4 = (sockaddr_in *)&their_addr;
		addr = &(ipv4->sin_addr);
	} else { // IPv6
		sockaddr_in6 *ipv6 = (sockaddr_in6 *)&their_addr;
		addr = &(ipv6->sin6_addr);
	}
	inet_ntop(their_addr.ss_family, addr, ipstr, sizeof ipstr);
	sourceIP = std::string(ipstr);
	//std::cerr << "przyszlo zadanie z "<<sourceIP<<std::endl;
}

Connection::~Connection(){
	pthread_mutex_unlock (&processing);
}

std::map<std::string, std::vector<std::pair<pthread_t, Connection* > > > Connection::connections;
pthread_mutex_t Connection::connections_mutex;

void * Connection::process(void *arg){
	timespec dozab;
	typedef Http::BaseResponse* (response_maker)(const Http::HttpRequest*);
	response_maker *create_response;
	int readpos=0, rr, found, messlen;
	char buf[255];
	std::string message;
	Http::HttpRequest * nowy;
	Http::BaseResponse * resp;
	for(;;){
	dozab.tv_sec = 5;
	dozab.tv_nsec = 0;
		rr = recv(socketfd, buf, sizeof buf, 0); //hangup until read something
		if(rr>0){
			timestamp = time(NULL);
			reqmes.append(buf, rr);
			found = reqmes.find(endofhead,((readpos-3)<0)?0:(readpos-3),4);
			readpos += rr;
			if(std::string::npos!= found){
				pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
				if(pthread_mutex_timedlock (&processing,&dozab)!=0) return NULL;
				input_str.write(reqmes.c_str(), found+4);
				nowy = new Http::HttpRequest(input_str);
				reqmes = reqmes.substr(found+4);
				readpos -= found + 4; //odjeto caly naglowek (found - ile minelismy do \r\n\r\n, później jeszcze odejmujemy długość tego ciągu)
				if(nowy->getHeader("Content-Length").size()>0){
					std::multimap<float,std::string> cos = nowy->getHeader("Content-Length");
					messlen = ::atoi(nowy->getHeader("Content-Length").begin()->second.c_str())-readpos;
				}else{
					messlen = -readpos;
				}
				if(messlen>0)
				{
					message = reqmes + recvmess(messlen); //wczytane - miejsce przed \r\n\r\n + ciag konczacy
					reqmes = "";
				}
				else
				{
					message = reqmes.substr(0,readpos+messlen);
					reqmes = reqmes.substr(readpos+messlen);
				}
				//dynamiczne wybieranie odpowiadajacego
				void *resp_handler = 0;
				if(nowy->getConfig()->getModul() != std::string("default")) resp_handler = dlopen(nowy->getConfig()->getModul().c_str(),RTLD_LAZY);
				std::cerr << dlerror();
				if(resp_handler==0){
					resp = new Http::BaseResponse(nowy);
				}
				else
				{
					create_response = (response_maker*)dlsym(resp_handler,"response_maker");
					resp = create_response(nowy);
				}
				resp -> prepareMessage();
				int blad=0;
				bool zamknij=false;
				send(resp->renderResponse());
				blad=send(resp->getMessage(),resp->getLength());
				//obsłużono, więc usuwamy z requesta
				if(blad!=0 || nowy->getHeader("Connection").begin()->second == std::string("close")) zamknij = true;
				delete nowy;
				delete resp;
				if(resp_handler != 0) dlclose(resp_handler);
				readpos = 0;
				input_str.str("");
				input_str.clear();
				if(zamknij) return NULL;
				pthread_mutex_unlock (&processing);
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
				pthread_yield();
			}
		}else{ //connection closed
			return NULL; //quit -> destructor will close the connection
		}
	}
}

ServerListen::ServerListen(addrinfo *_res):Socket(socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol)),res(_res){
	if(bind(socketfd, _res->ai_addr, _res->ai_addrlen)<0){ 
		std::cerr << "Error occured while opening port\n";
		bound = false;
	}
	else {
		listen(socketfd, 10);
		bound = true;
	}
}


void * ServerListen::runAcceptLoop(void * arg){
	if(!bound) return NULL;
	pthread_t nThreadID1;
	Connection *req;
	for(;;){
		req = new Connection(accept(socketfd, NULL, NULL));
		pthread_create(&nThreadID1, NULL, runThreadConnection, req);
		pthread_yield();
	}
}

