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
#include "Threads.h"
#include "Sockets.h"
#include "csignal"
void delete_connection_handler(void *arg){
	std::vector< std::pair<pthread_t, Connection*> >::iterator i;
	pthread_mutex_lock(&(Connection::connections_mutex));
	if(arg!=0){
		Connection **ptr = (Connection**) arg;
		Connection *con = *ptr;
		if(*ptr!=0){
			for( i=Connection::connections[con->sourceIP].begin() ; i!=Connection::connections[con->sourceIP].end() ; i++){
				if((i->second) == con){
					Connection::connections[con->sourceIP].erase(i);
					break;
				}
			}
			delete con;
		}
	}
	pthread_mutex_unlock(&(Connection::connections_mutex));
}
void * runThreadConnection(void *obj){
	signal(SIGPIPE,SIG_IGN);
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGINT);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
	void * ret=NULL;
	pthread_cleanup_push(&delete_connection_handler,&obj);
	Connection *con = (Connection*)obj;
	time_t czas = time(NULL);
	std::vector< std::pair<pthread_t, Connection*> >::iterator i,del;
	pthread_mutex_lock(&(Connection::connections_mutex));
	if(Connection::connections.find(con->sourceIP)==Connection::connections.end()) 
		Connection::connections[con->sourceIP] = std::vector< std::pair<pthread_t, Connection*> >();
	if(Connection::connections[con->sourceIP].size()>1){
		del = Connection::connections[con->sourceIP].end();
		for( i=Connection::connections[con->sourceIP].begin() ; i!=Connection::connections[con->sourceIP].end() ; i++){
			if(0==pthread_mutex_trylock(&(i->second->processing))){
				if(i->second->timestamp < czas){
					if(del!=Connection::connections[con->sourceIP].end()) pthread_mutex_unlock(&(del->second->processing));
					czas = i->second->timestamp;
					del = i;
				}
			}
		}
		if(del==Connection::connections[con->sourceIP].end()){
			pthread_mutex_unlock(&(Connection::connections_mutex));
			pthread_exit(NULL);
		}
		else
		{
			void * retval=NULL;
			std::cerr<<"za duzo nas\n";
			pthread_cancel(del->first);
			while(retval != PTHREAD_CANCELED) pthread_join(del->first, &retval);
//			Connection::connections[con->sourceIP].erase(del);
		}
	}
	Connection::connections[con->sourceIP].push_back(std::pair<pthread_t, Connection*> (pthread_self(),con));
	pthread_mutex_unlock(&(Connection::connections_mutex));
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	ret =  con->process(NULL);
	pthread_cleanup_pop(1);
	pthread_exit(ret);
}
void delete_socket_handler(void *arg){
	ServerListen **ptr = (ServerListen**) arg;
	delete *ptr;
}
void * runThreadListen(void *obj){
	signal(SIGPIPE,SIG_IGN);
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGINT);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
	void * ret=NULL;
	pthread_cleanup_push(&delete_socket_handler,&obj);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	ret = ((ServerListen*)obj)->runAcceptLoop(NULL);
	pthread_cleanup_pop(1);
	pthread_exit(ret);
}
