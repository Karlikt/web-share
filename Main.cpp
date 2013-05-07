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
#include "HttpResponse.h"
#include "http.tab.h"
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <map>
#include <fstream>

//initialize here because of plugins
std::map<std::string,Config*> Config::configs;
std::map<int,std::string> Http::BaseResponse::statusCodes;
std::map<std::string, std::map<std::string, std::pair<ServerListen*, pthread_t > > > bound;

void handleSigInt(int s){
	printf("Catch signal: %d\n",s);
	std::map<std::string, std::vector<std::pair<pthread_t, Connection* > > >::iterator it;
	int i;
	void *retval = NULL;
	for(it=Connection::connections.begin(); it!=Connection::connections.end(); it++){
		for(i=0 ; i<(it->second.size()) ; i++){
			pthread_cancel(it->second[i].first);
			retval = NULL;
			while(retval != PTHREAD_CANCELED) pthread_join(it->second[i].first, &retval);
		}
	}
	std::map<std::string, std::map<std::string, std::pair<ServerListen*, pthread_t > > >::iterator it2;
	std::map<std::string, std::pair<ServerListen*, pthread_t > >::iterator it3;
	int zw;
	for(it2 = bound.begin() ; it2!=bound.end() ; it2++) 
		for(it3 = it2->second.begin() ; it3 != it2->second.end() ; it3++){
			pthread_cancel(it3->second.second);
			retval = NULL;
			while(retval != PTHREAD_CANCELED && 
					pthread_join(it3->second.second, &retval)==0) 
				;
		}

	printf("All threads finished.\n");

	std::map<std::string,Config*>::iterator it4;
	for(it4 = Config::configs.begin() ; it4!=Config::configs.end() ; it4++){
		delete it4->second;
	}
	exit(0);
}

void reloadConfig(void){
	using std::ifstream;
	if(Config::configs.find("")!=Config::configs.end()) delete Config::configs[""];
	Config::configs[""] = new Config("./generalconfig/");
	std::string host, modul;
	ifstream *cfg;
	ifstream *lista = new ifstream("hosts");
	*lista >> host >> modul;
	Config *temp;
	while(lista->is_open() && lista->good()){
		if(Config::configs.find(host)!=Config::configs.end()) delete Config::configs[host];
		Config::configs[host] = temp = new Config(*(Config::configs[""]));
		temp->setModul(modul);
		cfg = new ifstream((host+"/mimetypes.cfg").c_str());
		if(cfg->is_open() && cfg->good()){
			temp->setMimeType(*cfg);
		}
		cfg->close();
		delete cfg;

		cfg = new ifstream((host+"/errorfiles.cfg").c_str());
		if(cfg->is_open() && cfg->good()){
			temp->setErrorFile(*cfg);
		}
		cfg->close();
		delete cfg;

		cfg = new ifstream((host+"/redirect.cfg").c_str());
		if(cfg->is_open() && cfg->good()){
			temp->setRedirect(*cfg);
		}
		cfg->close();
		delete cfg;
		*lista >> host >> modul;
	}
	delete lista;
}

int main(int argc, const char* argv[]){
	signal(SIGPIPE,SIG_IGN);
	reloadConfig();	
	int splitted, sig;
	char temp[INET6_ADDRSTRLEN];
	std::string hostname,port;
	std::map<std::string,Config*>::iterator host_it;
	addrinfo hints, *res;
	pthread_t listenthread;
	void * addr;
	bound = std::map<std::string, std::map<std::string, std::pair<ServerListen*, pthread_t > > >();
	host_it = Config::configs.begin();
	for(host_it++ ; host_it != Config::configs.end() ; host_it++){
		memset(&hints, 0, sizeof hints);
		hints.ai_family = PF_UNSPEC;  // use IPv4 or IPv6, whichever od AF_UNSPEC
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
		splitted = host_it->first.rfind(':');
		if(splitted==std::string::npos){
			port = "80";
		}else{
			port = host_it->first.substr(splitted+1);
		}
		hostname = host_it->first.substr(0,splitted);
		getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res);
		if (res->ai_family == AF_INET) { // IPv4
			sockaddr_in *ipv4 = (sockaddr_in *)res->ai_addr;
			addr = &(ipv4->sin_addr);
		} else { // IPv6
			sockaddr_in6 *ipv6 = (sockaddr_in6 *)res->ai_addr;
			addr = &(ipv6->sin6_addr);
		}
		inet_ntop(res->ai_family,addr,temp, sizeof temp);
		if(bound.find(temp) == bound.end() || bound[temp].find(port) == bound[temp].end())
		{
			bound[temp][port].first = new ServerListen(res);
			pthread_create(&listenthread, NULL, runThreadListen, bound[temp][port].first);
			bound[temp][port].second = listenthread;
		}
		freeaddrinfo(res);
	}
	signal(SIGINT, handleSigInt);
//	for(;;){
//		sleep(1<<31); //wait "infinity" time
//		pthread_yield();
//	}
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGINT);
	sigwait(&set, &sig);
	handleSigInt(sig);
	return 0;
}
