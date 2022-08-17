#include "Cgi/Cgi.hpp"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

Cgi::Cgi(Request &request, ConfigStruct configStruct): _selfExecuting(false), _confStruct(configStruct)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "Cgi Constructor called for " << this << RESET << std::endl;
	#endif
	_env = NULL;
	_method = request.getMethod();
	string url = request.getDecodedTarget();
	string extension = url.substr(url.find_last_of("."));

	if (url.find("/cgi/") != string::npos)
		_selfExecuting = true;

	int queryStart = url.find("?");
	if (_selfExecuting)
	{
		int scriptNameStart = url.find("/cgi/") + 5;
		int scriptNameEnd = url.find("/", scriptNameStart);
		if (scriptNameEnd == (int)string::npos)
			scriptNameEnd = url.find("?", scriptNameStart);
		int queryStart = url.find("?");
		_scriptName = url.substr(scriptNameStart, scriptNameEnd - scriptNameStart);
		_pathInfo = (scriptNameEnd != (int)string::npos ) ? url.substr(scriptNameEnd + 1, queryStart - scriptNameEnd - 1) : "";
		_queryString = (queryStart != (int)string::npos )? url.substr(queryStart + 1, string::npos) : "";
		setEnv(request);
	}
	// "Accept");
	// "Host");
	// "Referer");
	// "Content-Length");
	// "Origin"); // client address, POST only
	// "Referer"); //client page where request was made from
	// "Content-Length");
	// "REMOTE_ADDR"); //refere
	// "REMOTE_HOST"); //if not provided same as REMOTE_ADDR
	// "REQUEST_METHOD"); //case-sensitive GET, POST, UPDATE
	// "SCRIPT_NAME");
	// "SERVER_NAME");
}

Cgi::~Cgi()
{
	int i = 0;
	while(_env[i])
		delete _env[i++];
	delete _env;
	#ifdef SHOW_CONSTRUCTION
		std::cout << RED << "Cgi Deconstructor called for " << this << RESET << std::endl;
	#endif
}

void Cgi::setEnv(Request &request) // think about changing this to return a const char **
{
	const std::map<std::string, std::string>& reqHeaders = request.getHeaderFields();
	string envVars[] = { "REQUEST_METHOD", "GATEWAY_INTERFACE", "PATH_INFO", "PATH_TRANSLATED", "SCRIPT_NAME",
		"QUERY_STRING", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "CONTENT_TYPE", "CONTENT_LENGTH", "REMOTE_HOST", "REMOTE_ADDR", };

	_env = new char*[ENV_LEN];
	_env[ENV_LEN - 1] = NULL;

	int i = 0;
	_env[i] = strdup((envVars[i] + "=" + _method).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "CGI/1.1").c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + _pathInfo).c_str()); i++;
	_env[i] = _pathInfo.empty() ? strdup((envVars[i] + "=" + "").c_str()) : strdup((envVars[i] + "=" + "./" + _pathInfo).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "./" + _scriptName).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + _queryString).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "localhost").c_str()); i++; // please do not hardcode the hostname since it can be very different
	_env[i] = strdup((envVars[i] + "=" + "8080").c_str()); i++; // please do not hardcode the port since it can be very different
	_env[i] = strdup((envVars[i] + "=" + "HTTP/1.1").c_str()); i++;
	if (_method == "POST")
	{
		_env[i] = strdup((envVars[i] + "=" + reqHeaders.find("Content-Type")->second).c_str()); i++; // the header fields are parsed in a way so they only have lowercase letters!!!!
		_env[i] = strdup((envVars[i] + "=" + reqHeaders.find("Content-Length")->second).c_str()); i++; // same as above
		_env[i] = strdup((envVars[i] + "=" + reqHeaders.find("Origin")->second).c_str()); i++; // same as above
		_env[i] = strdup((envVars[i] + "=" + reqHeaders.find("Origin")->second).c_str()); i++; // same as above
	}

	while(_env[i])
		_env[i++] = NULL;
}

void Cgi::phpHandler(Request &req)
{
	string docRoot = "/Users/tlemma/Documents/Webserv/main/init/server/pages"; // don't hardcode this since this can change depending on the conf file !!!!! and this path contains the username which will mess up when running on the evaluators pc
	string filePath = req.getUrl();
	_pathInfo = docRoot + filePath;
	cout << "url :" << _pathInfo << endl;

	_env = new char*[2];
	_env[0] = strdup(_pathInfo.data());
	_env[1] = NULL;

	_isPhp = true;
}

void Cgi::blaHandler(Request &req) // please use #ifdef for all the 42 tester only stuff
{
	_docRoot = "/Users/aenglert/testfolder/webserv/42tester"; // this contains the username!!!!!!!!!!!
	string filePath = req.getUrl();
	_pathInfo = _docRoot + filePath;
	cout << "url :" << _pathInfo << endl;

	_env = new char*[2];
	_env[0] = strdup(_pathInfo.data());
	_env[1] = NULL;

	_isTester = true;
}

void Cgi::passAsInput(void)
{
	#ifdef FORTYTWO_TESTER
		int fileFd = open("./42tester/YoupiBanane/youpi.bla", O_RDONLY);
		if (fileFd == -1)
		{
			cout << "path not found" << endl;
			return;
		}
		dup2(fileFd, STDIN_FILENO);
		close(fileFd);
	#endif
}

void Cgi::cgi_response(int fd)
{
	std::string file;
	std::string executable;
	char **args;
	int pipefd[2];

	args = NULL;
	executable = _scriptName;
	cout << "executable: " << executable << endl;
	if (!_selfExecuting)
		passAsInput();
	std::cout << GREEN << "Executing CGI..." << std::endl;
	file = "index.php"; // what if the filename changes???
	int stdout_init = dup(STDOUT_FILENO);
	pipe(pipefd);
	dup2(pipefd[1], STDOUT_FILENO);
	close(pipefd[1]);
	int pid = fork();
	if (pid == 0)
	{
		chdir("cgi-bin"); // what if dirname changes????
		if (execve(executable.c_str(), args, _env) == -1)
		{
			std::cout << "error executing cgi" << std::endl;
		}

		exit(0);
	}
	wait(NULL);
	dup2(stdout_init, STDOUT_FILENO);
	std::string header = "HTTP/1.1 200 OK\r\n";
	send(fd, header.c_str(), header.size(), 0);
	char buf[1024];
	buf[1023] = '\0';
	int n;
	while((n = read(pipefd[0], buf, 1023)) > 0)
	{
		#ifdef SHOW_LOG
			cout << "cgi output: " << buf << endl;
		#endif
		send(fd, buf, n, 0);
	}
	close(pipefd[0]);
	// close(fd);
}