#ifndef SERVER_HPP
#define SERVER_HPP

/* system includes */
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <sys/event.h>
#include <map>
#include <csignal> // check if forbidden !!!!!!!!!!

/* our includes */
#include "Response.hpp"
#include "Request.hpp"
#include "Cgi.hpp"
#include "SingleServerConfig.hpp"

#include "Base.hpp"

// Forbidden includes
#include <errno.h>

struct client
{
	int fd;
	struct sockaddr_in addr;
};

#define MAX_EVENTS 128

static volatile int keep_running = 1;

class Server
{
	public:
		Server();
		Server(int port);
		~Server(void);

		void stop(void);

		int get_client(int fd);

		int add_client(int fd, struct sockaddr_in addr);

		int remove_client(int fd);

		void run_event_loop(int kq);

		void run(void);

		void handle_static_request(const std::string&, int);
		void handleGET(int, int, const std::string&);
		void handlePOST(int, int, const Request&);
		void handleERROR(int, int);
	private:
		size_t _port;
		size_t _server_fd;
		std::vector <client> _clients;
		Response Response;
};

#endif