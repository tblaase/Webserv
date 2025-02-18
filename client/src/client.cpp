#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

// Colors and Printing
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define BOLD "\033[1m"
#define UNDERLINED "\033[4m"

int main()
{
	int numConnections = 1; // us this to determine how many interrupted connection you want to have
	// while (true) // use this to bomb a server
	while (numConnections > 0)
	{

	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout << RED << "Error creating socket" << std::endl << RESET;
		return 1;
	}
	// int val = 1;
	// setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, 4);

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serv_addr.sin_zero, '0', sizeof(serv_addr.sin_zero));

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cout << RED << "Error connecting to socket" << std::endl << RESET;
		// return 1;
	}
	else
		std::cout << GREEN << "Connected to socket: " << sockfd << std::endl << RESET;
	// char buffer[10000] = {0};
	std::string buffer = "POST /uploads/upload_test.txt HTTP/1.1\r\nHost: localhost:8080\r\nContent-Length: 18\r\n\r\nThis is a test from the body!!!!\r\n\r\n";
	char read_buffer[10000] = {0};
	int msg_size = buffer.length();
		if (write(sockfd, buffer.c_str(), msg_size) < 0)
		{
			std::cout << RED << "Error writing to socket" << std::endl << RESET;
			return 1;
		}
		else
			std::cout << GREEN << "Request send to server" << RESET << std::endl;
		usleep(100); // now only the headd gets to the client
		int n = read(sockfd, read_buffer, sizeof(read_buffer));
		if (n < 0)
		{
			std::cout << RED << "Error reading from socket" << std::endl << RESET;
			return 1;
		}
		else
		{
			read_buffer[n] = '\0';
			std::string readString = read_buffer;
			std::cout << GREEN << "Message received:\n>" << RESET << readString << GREEN << "<" << std::endl << RESET;
		}
		close(sockfd);
		// memset(buffer, 0, sizeof(buffer));
		--numConnections;
		usleep(10000); // change this to bomb it faster
	}

	return 0;
}