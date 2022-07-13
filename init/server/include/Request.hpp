#pragma once

#include <string> //std::string
#include <iostream> //std::ostream
#include <set> //std::set
#include <vector> //std::vector
#include "Message.hpp"

#define INDEX_PATH "./pages/index.html"

class Request : public Message
{
	private:
		std::set<std::string> validMethods;
		std::string method;
		std::string url;
		// std::string protocol;
		void addMethods(void);
		void parseMessage(const std::string&);
		bool isValidMethod(std::string);
		bool isValidHeaderFieldName(const std::string&) const;
		bool isValidHeaderFieldValue(const std::string&) const;
		void toLower(std::string&);
		const std::string createHeaderFieldName(const std::string& message, size_t pos);
		const std::string createHeaderFieldValue(const std::string& message, size_t pos);
		const std::string removeLeadingAndTrailingWhilespaces(const std::string& message, size_t pos);
		void parseStartLine(std::istringstream&);
		void parseHeaderFields(std::istringstream&);
		void parseHeaderFieldLine(const std::string&);
		void parseBody(std::istringstream&);
		void createStartLineTokens(std::vector<std::string>&, const std::string&);
		// void createStartLineTokens(std::vector<std::string>&, const std::string&, const unsigned int&, const std::string&);
		void createHeaderTokens(std::vector<std::string>& tokens, const std::string& message);
		void setBodyFlag(void);
	public:
		Request(const std::string&);
		// Request(const std::string&, int fd);
		~Request(void);

		// void setMethod(const std::string&);
		// void setUrl(const std::string&);
		// void setProtocol(const std::string&);

		const std::string& getMethod(void) const;
		const std::string& getUrl(void) const;
		// const std::string& getProtocol(void) const;

	class InvalidNumberOfTokens : public std::exception
	{
		const char* what() const throw();
	};

	class EmptyMessage : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidMethod : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidHeaderField : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidHeaderFieldName : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidHeaderFieldValue : public std::exception
	{
		const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream&, const Request&);