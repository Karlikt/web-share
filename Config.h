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
#include <map>
#include <string>
#include <fstream>
#include <utility>

class Config{
	private:
		std::map<std::string,std::string> mimeTypes;
		std::map<std::string,std::pair<int, std::string> > redirect;
		std::map<int,std::string> errorFiles;
		std::string modul;
	public:
		static std::map<std::string,Config*> configs;
		Config();
		Config(std::string);
		~Config();
		Config(const Config&);
		std::string getMimeType(const std::string &) const;
		void setMimeType(const std::string &, const std::string &);
		void setMimeType(std::istream&);
		std::string getErrorFile(const int &) const;
		void setErrorFile(const int &, const std::string &);
		void setErrorFile(std::istream&);
		std::pair<int,std::string> getRedirect(const std::string &) const;
		void setRedirect(const std::string &, int, const std::string &);
		void setRedirect(std::istream&);
		void setModul(std::string);
		std::string getModul() const;
};

