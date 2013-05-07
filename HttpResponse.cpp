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
#include "HttpResponse.h"
#include "HttpReq.h"
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <cstdlib>


namespace Http{
	bool BaseResponse::initializeStatusCodes(){
		statusCodes[100] = "Continue";
		statusCodes[101] = "Switching Protocols";
		statusCodes[200] = "OK";
		statusCodes[201] = "Created";
		statusCodes[202] = "Accepted";
		statusCodes[203] = "Non-Authoritative Information";
		statusCodes[204] = "No Content";
		statusCodes[205] = "Reset Content";
		statusCodes[206] = "Partial Content";
		statusCodes[300] = "Multiple Choices";
		statusCodes[301] = "Moved Permanently";
		statusCodes[302] = "Found";
		statusCodes[303] = "See Other";
		statusCodes[304] = "Not Modified";
		statusCodes[305] = "Use Proxy";
		//statusCodes[306] = "(Unused)";
		statusCodes[307] = "Temporary Redirect";
		statusCodes[400] = "Bad Request";
		statusCodes[401] = "Unauthorized";
		statusCodes[402] = "Payment Required";
		statusCodes[403] = "Forbidden";
		statusCodes[404] = "Not Found";
		statusCodes[405] = "Method Not Allowed";
		statusCodes[406] = "Not Acceptable";
		statusCodes[407] = "Proxy Authentication Required";
		statusCodes[408] = "Request Timeout";
		statusCodes[409] = "Conflict";
		statusCodes[410] = "Gone";
		statusCodes[411] = "Length Required";
		statusCodes[412] = "Precondition Failed";
		statusCodes[413] = "Request Entity Too Large";
		statusCodes[414] = "Request-URI Too Long";
		statusCodes[415] = "Unsupported Media Type";
		statusCodes[416] = "Requested Range Not Satisfiable";
		statusCodes[417] = "Expectation Failed";
		statusCodes[500] = "Internal Server Error";
		statusCodes[501] = "Not Implemented";
		statusCodes[502] = "Bad Gateway";
		statusCodes[503] = "Service Unavailable";
		statusCodes[504] = "Gateway Timeout";
		statusCodes[505] = "HTTP Version Not Supported";
		return true;
	}
	BaseResponse::BaseResponse(const HttpRequest *_r):request(_r){
		Version = "HTTP/1.1";
		ResponseCode = 500;
		Message = NULL;
		_fill_date();
		_fill_server_name();
	}
	BaseResponse::~BaseResponse(){
		if(Message && ((std::ifstream*) Message) && ((std::ifstream*) Message)->is_open()){ 
			((std::ifstream*) Message)->close();
		}
		if((std::ifstream*)Message)
			delete (std::ifstream*)Message;
		else if(Message) 
			delete Message;
	}
	void BaseResponse::prepareMessage(){
		int code = request->getConfig()->getRedirect(request->getPath()).first;
		if(code > 0 ){
			ResponseCode = code;
			Headers["Location"] = request->getConfig()->getRedirect(request->getPath()).second;
			Headers["Content-Length"] = "0";
			lenMessage= 0;
			return;
		}
		std::stringstream len;
		std::string length;
		std::string filename = "./"+request->getHeader("Host").begin()->second+"/"+request->getPath();
		Message = (std::iostream*) new std::ifstream(filename.c_str(),std::ios_base::in|std::ios_base::binary);
		if(Message->good()){
			ResponseCode = 200;
		}else{
			ResponseCode = 404;
		}
		if(ResponseCode != 200){
			filename = request->getConfig()->getErrorFile(ResponseCode);
			Message = (std::iostream*) new std::ifstream(filename.c_str(),std::ios_base::in|std::ios_base::binary);
		}
		Message->seekg(0,std::ios_base::end);
		lenMessage= Message->tellg();
		len<<lenMessage;
		len>>length;
		Message->seekg(0,std::ios_base::beg);
		Headers["Content-Length"] = length;
		Headers["Content-Type"] = request->getConfig()->getMimeType(filename);	
		//Headers["Connection"] = "close";
		Headers["Connection"] = "keep-alive";
		if(request->getMethod()=="HEAD"){
			((std::ifstream*)Message)->close();
			delete ((std::ifstream*)Message);
			Message = NULL;
			lenMessage = 0;
		}
	}
	std::string BaseResponse::renderResponse(){
		std::stringstream ret(std::stringstream::in | std::stringstream::out);
		ret.str("");
		ret.clear();
		std::map<std::string, std::string>::iterator mit;
		ret << Version << " " << ResponseCode << " " << statusCodes[ResponseCode] << "\r\n";
		for(mit = Headers.begin(); mit!=Headers.end() ; mit++){
			ret << mit->first << ": " << mit->second << "\r\n";
		}
		ret << "\r\n";
		return ret.str();
	}
	std::iostream & BaseResponse::getMessage(){
		return *Message;
	}
	void BaseResponse::_fill_date(){
		char buf[31];
		time_t rawtime = time(NULL);
		strftime(buf,31,"%a, %d %b %Y %H:%M:%S GMT",gmtime(&rawtime));
		Headers["Date"] = buf;
	}
	void BaseResponse::_fill_server_name(){
		Headers["Server"] = "Karlik's http server web-share";
	}
	bool BaseResponse::initialized = BaseResponse::initializeStatusCodes();
};
