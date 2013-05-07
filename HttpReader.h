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

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL int Http::HttpRead::Reader::yylex()

#include "http.tab.h"

namespace Http {
	namespace HttpRead {
		class Reader : public yyFlexLexer {
			public:
				// 0 means std equivilant (stdin, stdout)
				Reader(std::istream * in = 0, std::ostream * out = 0) : yyFlexLexer(in, out) {}

				inline int yylex(HttpParser::semantic_type * lval,
				                 HttpParser::location_type * lloc);
		
			private:
				// Scanning function created by Flex; make this private to force usage
				// of the overloaded method so we can get a pointer to Bison's yylval
				int yylex();
			
				// point to yylval (provided by Bison in overloaded yylex)
				HttpParser::semantic_type * yylval;
				
				// pointer to yylloc (provided by Bison in overloaded yylex)
				HttpParser::location_type * yylloc;
				
				// block default constructor
				Reader();
				// block default copy constructor
				Reader(Reader const &rhs);
				// block default assignment operator
				Reader &operator=(Reader const &rhs);
		};
		
		// all our overloaded version does is save yylval and yylloc to member variables
		// and invoke the generated scanner
		int Reader::yylex(HttpParser::semantic_type * lval,
		                   HttpParser::location_type * lloc) {
			yylval = lval;
			yylloc = lloc;
			return yylex();
		}

	}
}

