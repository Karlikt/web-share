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
%require "2.4.1"
%skeleton "lalr1.cc"
%defines
%locations
%define namespace "Http::HttpRead"
%define parser_class_name "HttpParser"
%parse-param { Http::HttpRead::Reader &scanner }
%parse-param { Http::HttpRead::headData &Headers }
%parse-param { Http::HttpRead::reqData &GetParams }
%parse-param { Http::HttpRead::reqData &Cookies }
%parse-param { std::string &method }
%parse-param { std::string &ReqPath }
%parse-param { std::multimap<float,std::string> hvalue }
%lex-param   { Http::HttpRead::Reader &scanner }

%code requires {
	#include <string>
	#include <cstdlib>
	#include <sstream>
	#include <utility>
	#include <map>

	// We want to return a string
	#define YYSTYPE std::string

	namespace Http {
		namespace HttpRead {
			class Reader;
		
			typedef std::map<std::string, std::multimap<float, std::string> > headData;
			typedef std::map<std::string, std::string> reqData;
		}
	}
}

%code {
	// Prototype for the yylex function
	static int yylex(Http::HttpRead::HttpParser::semantic_type * yylval,
	                 Http::HttpRead::HttpParser::location_type * yylloc,
	                 Http::HttpRead::Reader &scanner);
}

%token HEAD_ASSIGNMENT VALUE_ASSIGNMENT COOKIE_ASSIGNMENT
%token SPACE MULTI NUMERIC ALPHABETIC END_LINE 
%token COOKIES_HEAD FULL_HEAD AUTHORIZATION_HEAD
%token COOKIE_SEPARATOR BEGIN_GET GET_SEPARATOR

%%

input: RequestLine headers
	;
	
headers: headers header
	| header
	;

RequestLine: string SPACE get_req SPACE string END_LINE { method = $1; }

header: full_header END_LINE { hvalue.clear(); }
	| normal_header END_LINE{ Headers[$1] = hvalue; hvalue.clear(); }
	| cookies END_LINE { }
	;

get_req
	: string { ReqPath = $1; }
	| string BEGIN_GET get_params { ReqPath = $1; }
	;
	
cookies
	: COOKIES_HEAD HEAD_ASSIGNMENT SPACE cookie_values 
	;

full_header
	: FULL_HEAD HEAD_ASSIGNMENT SPACE string_full { Headers[$1].insert(std::pair<float,std::string>(0, $4)); }
	| AUTHORIZATION_HEAD HEAD_ASSIGNMENT SPACE string SPACE string_full { Headers[$1].insert(std::pair<float,std::string>(0, $4));
	Headers[$1].insert(std::pair<float,std::string>(1, $6)); }
	;

normal_header: ALPHABETIC HEAD_ASSIGNMENT SPACE value 
	;

string
	: NUMERIC
	| ALPHABETIC
	| string NUMERIC { $$ = $1 + $2;}
	| string ALPHABETIC { $$ = $1 + $2;}
	| string HEAD_ASSIGNMENT { $$ = $1 + $2;}
	;

string_full
	: string
	| string_full string { $$ = $1 + $2;}
	| string_full SPACE { $$ = $1 + $2;}
	| string_full MULTI { $$ = $1 + $2;}
	| string_full symbols { $$ = $1 + $2;}
	;

value
	: base_value
	| value MULTI base_value { $$ = $1 + '|' + $3;}
	| value MULTI SPACE base_value { $$ = $1 + '|' + $4;}
	;

base_value
	: string {  hvalue.insert(std::pair<float,std::string>(1, $1)); }
	| string VALUE_ASSIGNMENT string { hvalue.insert(std::pair<float,std::string>(::atof($3.c_str()), $1));}
	;

get_params
	: get_param
	| get_params GET_SEPARATOR get_param { $$ = $1 + '|' + $3;}
	;

get_param
	: string COOKIE_ASSIGNMENT string { GetParams[$1]=$3;}
	;

cookie_values
	: cookie
	| cookie_values COOKIE_SEPARATOR cookie { $$ = $1 + '|' + $3;}
	| cookie_values COOKIE_SEPARATOR SPACE cookie { $$ = $1 + '|' + $4;}
	;

cookie
	: string COOKIE_ASSIGNMENT string { Cookies[$1]=$3;}
	;

symbols
	: COOKIE_SEPARATOR
	| COOKIE_ASSIGNMENT
	| BEGIN_GET
	| GET_SEPARATOR
	| symbols symbols {$$=$1+$2;}
	;

%%

// Error function throws an exception (std::string) with the location and error message
void Http::HttpRead::HttpParser::error(const Http::HttpRead::HttpParser::location_type &loc,
                                          const std::string &msg) {
	std::ostringstream ret;
	ret << "Parser Error at " << loc << ": " << msg;
	throw ret.str();
}

// Now that we have the Parser declared, we can declare the Scanner and implement
// the yylex function
#include "HttpReader.h"
static int yylex(Http::HttpRead::HttpParser::semantic_type * yylval,
                 Http::HttpRead::HttpParser::location_type * yylloc,
                 Http::HttpRead::Reader &scanner) {
	return scanner.yylex(yylval, yylloc);
}

