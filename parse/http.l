/*
 * Copyright (c) 2011, Karol Trzcionka

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
%{
	#include "HttpReader.h"
	// used to keep track of location
	#define YY_USER_ACTION yylloc->columns(yyleng);
%}

%option nodefault yyclass="HttpParser" noyywrap c++

%%

%{
	yylloc->step();
%}

"User-Agent"|"Range"|"Cache-Control"|"Referer" { *yylval = yytext; return Http::HttpRead::HttpParser::token::FULL_HEAD; }
"Cookie" { *yylval = yytext; return Http::HttpRead::HttpParser::token::COOKIES_HEAD; }
"Authorization" { *yylval = yytext; return Http::HttpRead::HttpParser::token::AUTHORIZATION_HEAD; }
;[ ]*q[ ]*=          { *yylval = yytext; return Http::HttpRead::HttpParser::token::VALUE_ASSIGNMENT; }
":"          { *yylval = yytext; return Http::HttpRead::HttpParser::token::HEAD_ASSIGNMENT; }
"="   { *yylval = yytext; return Http::HttpRead::HttpParser::token::COOKIE_ASSIGNMENT; }
"?"          { *yylval = yytext; return Http::HttpRead::HttpParser::token::BEGIN_GET; }
"&"          { *yylval = yytext; return Http::HttpRead::HttpParser::token::GET_SEPARATOR; }
" "          { *yylval = yytext; yylloc->step(); return Http::HttpRead::HttpParser::token::SPACE; }
","          { *yylval = yytext; return Http::HttpRead::HttpParser::token::MULTI; }
";"   { *yylval = yytext; return Http::HttpRead::HttpParser::token::COOKIE_SEPARATOR; }
[0-9]+  { *yylval = yytext; return Http::HttpRead::HttpParser::token::NUMERIC; }
[^\r\n\t 0-9:;=,?&]+  { *yylval = yytext; return Http::HttpRead::HttpParser::token::ALPHABETIC; }

"\r\n"+        { yylloc->lines(yyleng); yylloc->step(); return Http::HttpRead::HttpParser::token::END_LINE; }
