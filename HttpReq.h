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
#pragma once

#include "HttpReader.h"
#include "Config.h"
#include <string>

extern std::map<std::string,Config*> configs;

namespace Http {
	class HttpRequest {
		public:
			inline explicit HttpRequest(std::istream &iniStream) throw(std::string);

			const std::string & getPath() const{
				return reqPath;
			}
			const std::string & getMethod() const{
				return method;
			}
			const std::multimap<float,std::string> getHeader(const std::string &key) const{
				if(Headers.find(key)!=Headers.end())
					return Headers.find(key)->second;
				return std::multimap<float,std::string> ();
			}
			std::string getGetParam(const std::string &key){
				return GetParams[key];
			}
			std::string getCookie(const std::string &key){
				return Cookies[key];
			}
			Config * getConfig() const{
				std::string h = getHeader("Host").begin()->second;
				if(Config::configs.find(h)!=Config::configs.end()) return Config::configs.find(h)->second;
				return Config::configs.find("")->second;
			}
		private:
			HttpRequest();
			HttpRequest(HttpRequest const &rhs);
			HttpRequest &operator=(HttpRequest const &rhs);
			
			HttpRead::headData Headers;
			HttpRead::reqData GetParams;
			HttpRead::reqData Cookies;
			std::string reqPath;
			std::string method;
	};
	
	HttpRequest::HttpRequest(std::istream &iniStream) throw(std::string) {
		HttpRead::Reader scanner(&iniStream);
		HttpRead::HttpParser parser(scanner, Headers, GetParams, Cookies, method, reqPath, std::multimap<float,std::string>());
		try{
			parser.parse();
		}
		catch(std::string error){
			std::cerr << error;
		}
	}
}

