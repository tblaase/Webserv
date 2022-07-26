// Header-protection
#ifndef SOCKETHANDLER_HPP
#define SOCKETHANDLER_HPP
#pragma once

// Includes
#include <string>
#include <iostream>
#include <set>
#include <map>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "Base.hpp"
#include "Config.hpp"

#define MAX_EVENTS 128

// struct ServerStruct
// {
// 	unsigned short fd;
// 	struct sockaddr_in serv_addr;
// };

struct ClientStruct
{
	int fd;
	struct sockaddr_in addr;
};

// classes
class SocketHandler
{
	private:
	// Private Variables
		// Config *_config; // maybe not needed
		std::map<std::string, ConfigStruct> _cluster;

		std::set<int> _ports;
		std::vector<ClientStruct> _clients;
		std::vector<int> _serverFds; // maybe not needed

		struct kevent _evList[MAX_EVENTS];

		std::map<int, int> _serverMap;
		int _kq;
		int _numEvents;
		char *_buffer[1024]; //temp
		int _fd; //temp

		// defines only to not have undefined behaviour
		SocketHandler(const SocketHandler &src);
		SocketHandler &operator=(const SocketHandler &src);

	// Private Members
		void _initPorts(); // read from ConfigStruct into _ports
		void _initMainSockets();
		void _listenMainSockets();
		void _initEventLoop();
		int _addClient(int fd, struct sockaddr_in addr);
		int _removeClient(int fd);
		int _getClient(int fd);

	public:
	// Constructors
		SocketHandler(Config *config);

	// Deconstructors
		~SocketHandler(); // have a loop that closes all used fd's stored in the ServerStruct

	// Overloaded Operators

	// Public Methods
		void getEvents();
		bool acceptConnection(int serverFD);
		bool readFromClient(); // maybe pass the iterator of the loop to here

	// Getter
		int getNumEvents() const;
		std::string getBuffer() const;
		int getFD() const;

	// Setter

};
#endif // SOCKETHANDLER_HPP
