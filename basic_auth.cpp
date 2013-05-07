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
#include "basic_auth.h"
#include <map>
#include <string>
#include <cstring>
#include <iostream>

Http::BaseResponse * response_maker(const Http::HttpRequest * req){
	return new BasicAuth(req);
}

std::string Base64Decode(const std::string &input){
	std::string ret("");
	unsigned char t[4];
	int i,c;
	for(i=0, c=0 ; i<input.size() ; i++, c=(c+1)%4){
		if(input[i]>='A' && input[i]<='Z'){
			t[c] = input[i]-'A';
		}else if(input[i]>='a' && input[i]<='z'){
			t[c]= input[i]-'a'+26;
		}else if(input[i]>='0' && input[i]<='9'){
			t[c]=input[i]-'0'+52;
		}else if(input[i]=='+'){
			t[c]=62;
		}else if(input[i]=='/'){
			t[c]=63;
		}else if(input[i]=='='){
			t[c]=0;
		}
		if(c==3){
			ret += (char)(((t[0]<<2)&0xFC) + ((t[1]>>4)&0x03)); //t[0]<<2&11111100 + t[1]>>4&00000011
			ret += (char)(((t[1]<<4)&0xF0) + ((t[2]>>2)&0x0F)); //t[1]<<4&11110000 + t[2]>>2&00001111
			ret += (char)(((t[2]<<6)&0xC0) + ((t[3])&0x3F)); //t[2]<<6&11000000 + t[3]&00111111
		}
	}
	return ret;
}

void BasicAuth::prepareMessage(){
	std::multimap<float,std::string>::const_iterator i=request->getHeader("Authorization").find(0);
	if(request->getHeader("Authorization").size() < 2 || i->second != std::string("Basic")){
		ResponseCode = 401;
		Headers["WWW-Authenticate"] = "Basic realm=\"Secure Area\"";
		Headers["Content-Length"] = "0";
		lenMessage = 0;
		return;
	}
	else
	{
		i=request->getHeader("Authorization").find(1);
		printf("Zalogowano z danymi: %s, input: %s\n", Base64Decode(i->second).c_str(), i->second.c_str());
	}
	BaseResponse::prepareMessage();
}
std::string BasicAuth::renderResponse(){
	return BaseResponse::renderResponse();
}
std::iostream & BasicAuth::getMessage(){
	return BaseResponse::getMessage();
}

