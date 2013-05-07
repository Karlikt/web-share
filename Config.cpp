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
#include "Config.h"
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <utility>

using std::string;

Config::Config(string confdir){
	using std::ifstream;
	modul = std::string("default");
	ifstream *cfg;
	cfg = new ifstream((confdir+"mimetypes.cfg").c_str());
	if(cfg->is_open() && cfg->good()){
		setMimeType(*cfg);
	}
	cfg->close();
	delete cfg;

	cfg = new ifstream((confdir+"errorfiles.cfg").c_str());
	if(cfg->is_open() && cfg->good()){
		setErrorFile(*cfg);
	}
	cfg->close();
	delete cfg;

	cfg = new ifstream((confdir+"redirect.cfg").c_str());
	if(cfg->is_open() && cfg->good()){
		setRedirect(*cfg);
	}
	cfg->close();
	delete cfg;
}
Config::~Config(){}
Config::Config(const Config &c){
	mimeTypes = c.mimeTypes;
	errorFiles = c.errorFiles;
	redirect = c.redirect;
	modul = c.modul;
}
std::string Config::getMimeType(const string &filename) const{
	string file;
	size_t found = filename.rfind('/');
	if (found!=string::npos) file = filename.substr(found+1);
	else file = filename;
	found = file.rfind('.');
	if (found!=string::npos) file = file.substr(found+1);
	std::map<string,string>::const_iterator f;
	f=mimeTypes.find(file);
	if(f!=mimeTypes.end()) return f->second;
	return "text/plain";
}
void Config::setMimeType(const std::string &file, const std::string &type){
	mimeTypes[file]=type;
}
void Config::setMimeType(std::istream &in){
	string file, type;
	in >> file >> type;
	while(in.good()){
		mimeTypes[file]=type;
		in >> file >> type;
	}
}
std::string Config::getErrorFile(const int &code) const{
	std::map<int,string>::const_iterator f;
	f=errorFiles.find(code);
	if(f!=errorFiles.end()) return f->second;
	return "";
}
void Config::setErrorFile(const int &code, const string &file){
	errorFiles[code]=file;
}
void Config::setErrorFile(std::istream &in){
	int code;
	string file;
	in >> code >> file;
	while(in.good()){
		errorFiles[code]=file;
		in >> code >> file;
	}
}
std::pair<int, std::string> Config::getRedirect(const string &file) const{
	std::map<string,std::pair<int,string> >::const_iterator f;
	f=redirect.find(file);
	if(f!=redirect.end()) return f->second;
	return std::pair<int,std::string>(0,"");
}
void Config::setRedirect(const std::string &from, int code, const std::string &to){
	redirect[from].first=code;
	redirect[from].second=to;
}
void Config::setRedirect(std::istream &in){
	string from, to;
	int code;
	in >> code >> from >> to;
	while(in.good()){
		redirect[from].first=code;
		redirect[from].second=to;
		in >> code >> from >> to;
	}
}

void Config::setModul(std::string m){
	modul = m;
}

std::string Config::getModul() const{
	return modul;
}
