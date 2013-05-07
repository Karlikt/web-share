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
#include "HttpReq.h"
#include <string>
#include <map>
#include <iostream>
namespace Http{
	class BaseResponse{
		protected:
			static std::map<int,std::string> statusCodes;
			static bool initialized;
			const HttpRequest *request;
			std::string Version;
			int ResponseCode;
			std::map<std::string,std::string> Headers;
			std::iostream *Message;
			int lenMessage;
			void _fill_date();
			void _fill_server_name();
		public:
			explicit BaseResponse(const HttpRequest*);
			~BaseResponse();
			virtual void prepareMessage();
			int getLength(){
				return lenMessage;
			}
			int getStatus(){
				return ResponseCode;
			}
			virtual std::string renderResponse();
			virtual std::iostream & getMessage();
			static bool initializeStatusCodes();
	};
};
