#ifndef __CGI_HPP__
#define __CGI_HPP__

#include "Base.hpp"
#include "Request.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

using std::string;
using std::map;
using std::vector;
using std::cout;
using std::endl;

#define ENV_LEN 14

class Cgi
{
	private:
	// defines only to not have undefined behaviour
		Cgi();
		Cgi(const Cgi&);
		Cgi& operator=(const Cgi&);

	// private variables
		std::map<string, string> _env;
		string _method;
		string _scriptName;
		string _pathInfo;
		string _queryString;
		bool	_isPhp;
		bool	_scriptExists;
		std::map<string, string> _handlers;
		ConfigStruct _confStruct;
	public:
		Cgi(Request &request, ConfigStruct confStruct);

		~Cgi();

		void printEnv();
		void setEnv(Request &request);
		void cgi_response(int fd);
		void phpHandler(Request &req);
		void printEnv();
};
#endif