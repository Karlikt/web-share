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
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <ctime>
#include <string>
#include <sstream>
#include <queue>
#include <map>

#include <iostream>

class Socket {
	protected:
		volatile int socketfd;
		int send(const std::string&);
		int send(std::iostream &,int);
		std::string recvmess(int);
	public:
		Socket(){}
		Socket(int);
		virtual ~Socket();
};

class Connection : Socket {
	private:
		static const char endofhead[4];
		time_t timestamp;
		std::string reqmes;
		std::string sourceIP;
		std::stringstream input_str;
		std::map<std::string, std::string> req_headers;
		pthread_mutex_t processing;
		static pthread_mutex_t connections_mutex;
		static std::map<std::string, std::vector<std::pair<pthread_t, Connection* > > > connections;
	public:
		explicit Connection(int);
		~Connection();
		void * process(void*);
		time_t getLastUse() const{
			return timestamp;
		}
		
	friend bool operator<(const Connection &,const Connection &);
	friend void * runThreadConnection(void *);
	friend void delete_connection_handler(void *);
	friend void handleSigInt(int);
};

class ServerListen : Socket {
	private:
		bool bound;
		addrinfo *res;
	public:
		explicit ServerListen(addrinfo *);
		~ServerListen(){}
		void * runAcceptLoop(void*);
};

