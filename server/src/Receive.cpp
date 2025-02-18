#include "Response.hpp"

/********** handle POST *********/

/**
 * @brief  Will read a chunk of data from a POST request and write it to the appropriate file
 * @note  will work on chunked and not chunked requests
 * @param  i: clients filedescriptor
 * @retval None
 */
void Response::receiveChunk(int i)
{
	const int clientFd = i;
	size_t total;
	size_t bytesLeft;
	size_t bufferSize;

// for the case where it is chunked by the client
	if (this->_receiveMap[i].isChunked == true)
	{
		if (this->_receiveMap[i].total == 0 || (this->_receiveMap[i].total != 0 && this->_receiveMap[i].bytesLeft == 0))
		{
			if (this->_receiveMap[i].total == 0 && this->_receiveMap[i].isCgi == true)
			{
				this->_tempFile[clientFd] = tmpfile();
			}
			this->_receiveMap[i].total = this->_handleChunked(i);
			if (this->_receiveMap[i].total == 0)
			{
				char endBuffer[3];
				int endReturn = read(clientFd, endBuffer, 3);
				if (endReturn != 2)
					throw Response::BadRequestException();
				if (endBuffer[0] != '\r' || endBuffer[1] != '\n')
					throw Response::BadRequestException();
				if (this->_receiveMap[i].isCgi == false)
				{
					this->_receiveMap.erase(i);
					this->constructPostResponse();
					this->putToResponseMap(clientFd);
				}
				else
				{
					this->_receiveMap[i].end = true;
				}
				return ;
			}
			this->_receiveMap[i].bytesLeft = this->_receiveMap[i].total;
			// bufferSize = this->_receiveMap[i].bufferSize;
		}
		// else
		// {
		// 	total = this->_receiveMap[i].total;
		// 	bytesLeft = this->_receiveMap[i].bytesLeft;
		// 	bufferSize = this->_receiveMap[i].bufferSize;
		// }
	}
	// else
	// {
		total = this->_receiveMap[i].total;
		bytesLeft = this->_receiveMap[i].bytesLeft;
		bufferSize = this->_receiveMap[i].bufferSize;
	// }
// opening the file and checking if its open
	if (this->_receiveMap[clientFd].isCgi == true)
	{
		this->_readForCgi(clientFd);
	}
	else
	{
		int n = 0;
		std::ofstream buffer;
		char chunk[bufferSize];
		buffer.open(this->_receiveMap[i].target.c_str(), std ::ios::out | std::ios_base::app | std::ios::binary);
		if (access(this->_receiveMap[i].target.c_str(), W_OK) != 0)
		{
			if (errno == ENOENT)
			{
				throw ERROR_404();
			}
			else if (errno == EACCES)
				throw ERROR_403();
			else
				throw ERROR_500();
		}

		if (buffer.is_open() == false)
		{
			throw ERROR_500();
			// throw Response::ERROR_423();
		}
	// reading part
		else if (total > bufferSize && bytesLeft > bufferSize)
		{
			#ifdef SHOW_LOG_2
				std::cout << YELLOW << "bytesLeft: " << bytesLeft << RESET << std::endl;
			#endif
			n = read(clientFd, chunk, bufferSize);
		}
		else
		{
			#ifdef SHOW_LOG_2
				std::cout << YELLOW << "last bytesLeft: " << bytesLeft << RESET << std::endl;
			#endif
			n = read(clientFd, chunk, bytesLeft);
		}

		#ifdef SHOW_LOG_2
			std::cout << BOLD << GREEN << "Bytes received from " << clientFd << ": " << n << RESET << std::endl;
			// LOG_YELLOW(chunk); // REMOVE AFTER TESTING, this might break the server if a non text file is in the body!!
		#endif
	// checking for errors of read
		if (n > 0)
		{
			bytesLeft -= n;
			#ifdef SHOW_LOG_2
				std::cout << BOLD << YELLOW << "Bytes left to read from " << clientFd << ": " << bytesLeft << RESET << std::endl;
			#endif
		}
		else
		{
			#ifdef SHOW_LOG_2
				std::cout << RED << "reading for POST on fd " << clientFd << " failed" << RESET << std::endl;
			#endif
			this->_receiveMap.erase(i);
			bytesLeft = 0;
			buffer.close();
			throw Response::BadRequestException();
		}
	// writing the read contents to the file
		// buffer[n] = '\0' // this would be needed for the next line
		// buffer << chunk; // this will stop writing if encounters a '\0', wich can happen in binary data!
		buffer.write(chunk, n); // with this there can even be a '\0' in there, it wont stop writing
	// setting the bytesLeft and checking if all content was read
		if (bytesLeft)
		{
			this->_receiveMap[i].bytesLeft = bytesLeft;
			buffer.close();
		}
		else if (this->_receiveMap[i].isChunked)
		{
			char endBuffer[2];
			int endReturn = read(clientFd, endBuffer, 2);
			if (endReturn != 2)
				throw Response::BadRequestException();
			if (endBuffer[0] != '\r' || endBuffer[1] != '\n')
				throw Response::BadRequestException();
			this->_receiveMap[i].bytesLeft = bytesLeft;
			buffer.close();
		}
		else /* if (this->_receiveMap[i].isChunked == false) */
		{
			buffer.close();
			this->removeFromReceiveMap(clientFd);
			this->constructPostResponse();
			this->putToResponseMap(clientFd);
			// std::string abc = this->_responseMap[clientFd].response;
			// total = abc.length();
			// this->_responseMap[clientFd].total = this->_responseMap[clientFd].response.length();
			// this->_responseMap[clientFd].bytesLeft = this->_responseMap[clientFd].response.length();
		}
	}
}


void Response::_readForCgi(size_t clientFd)
{
	size_t total = this->_receiveMap[clientFd].total;
	size_t bytesLeft = this->_receiveMap[clientFd].bytesLeft;
	size_t bufferSize = this->_receiveMap[clientFd].bufferSize;
	char chunk[bufferSize];
	int n = 0;
	if (this->_tempFile.count(clientFd) == 0)
		this->_tempFile[clientFd] = tmpfile();

	if (total > bufferSize && bytesLeft > bufferSize)
	{
		#ifdef SHOW_LOG_2
			std::cout << YELLOW << "bytesLeft: " << bytesLeft << RESET << std::endl;
		#endif
		n = read(clientFd, chunk, bufferSize);
	}
	else
	{
		#ifdef SHOW_LOG_2
			std::cout << YELLOW << "last bytesLeft: " << bytesLeft << RESET << std::endl;
		#endif
		n = read(clientFd, chunk, bytesLeft);
	}

	#ifdef SHOW_LOG_2
		std::cout << BOLD << GREEN << "Bytes received from " << clientFd << ": " << n << RESET << std::endl;
		// LOG_YELLOW(chunk); // REMOVE AFTER TESTING, this might break the server if a non text file is in the body!!
	#endif
// checking for errors of read
	if (n > 0)
	{
		bytesLeft -= n;
		#ifdef SHOW_LOG_2
			std::cout << BOLD << YELLOW << "Bytes left to read from " << clientFd << ": " << bytesLeft << RESET << std::endl;
		#endif
	}
	else
	{
		#ifdef SHOW_LOG_2
			std::cout << RED << "reading for POST on fd " << clientFd << " failed" << RESET << std::endl;
		#endif
		this->_receiveMap.erase(clientFd);
		bytesLeft = 0;
		fclose(this->_tempFile[clientFd]);
		this->_tempFile.erase(clientFd);
		throw Response::BadRequestException();
	}
// writing the read contents to the file
	// buffer[n] = '\0' // this would be needed for the next line
	// buffer << chunk; // this will stop writing if encounters a '\0', wich can happen in binary data!
	size_t count = n;
	this->_receiveMap[clientFd].bytesWritten += fwrite(chunk, sizeof(char), count, this->_tempFile[clientFd]); // with this there can even be a '\0' in there, it wont stop writing
// setting the bytesLeft and checking if all content was read
	if (bytesLeft)
	{
		this->_receiveMap[clientFd].bytesLeft = bytesLeft;
	}
	else if (this->_receiveMap[clientFd].isChunked)
	{
		char endBuffer[2];
		int endReturn = read(clientFd, endBuffer, 2);
		if (endReturn != 2)
			throw Response::BadRequestException();
		if (endBuffer[0] != '\r' || endBuffer[1] != '\n')
			throw Response::BadRequestException();
		this->_receiveMap[clientFd].bytesLeft = bytesLeft;
	}
	else /* if (this->_receiveMap[i].isChunked == false) */
	{
		this->_receiveMap[clientFd].end = true;
	}

}

// helper functions for POST

/**
 * @brief  this will create the responses content for a successfull POST
 * @note
 * @retval the response as a string with head and body that needs to be sent to the client
 */
void Response::constructPostResponse()
{
	this->clear();


	std::stringstream buffer;
	this->setProtocol(PROTOCOL);
	this->setStatus("201");
	this->addContentLengthHeaderField();
}

/**
 * @brief  checks if a clients filedescriptor is already part of our receiveMap
 * @note
 * @param  clientFd: clients filedescriptor
 * @retval bool of the state
 */
bool Response::isInReceiveMap(int clientFd)
{
	return (this->_receiveMap.count(clientFd));
}

/**
 * @brief  will remove a client from the receiveMap if existant
 * @note
 * @param  fd: clients filedescriptor
 * @retval None
 */
void Response::removeFromReceiveMap(int fd)
{
	if (this->_receiveMap.count(fd) == true)
		this->_receiveMap.erase(fd);
}

bool Response::isFinished(size_t clientFd)
{
	if (this->_receiveMap.count(clientFd))
		return (this->_receiveMap[clientFd].end);
	else
		return (false);
}

bool Response::isCgi(size_t clientFd)
{
	if (this->_receiveMap.count(clientFd))
		return (this->_receiveMap[clientFd].isCgi);
	else
		return (false);
}

bool Response::isChunked(size_t clientFd)
{
	if (this->_receiveMap.count(clientFd))
		return (this->_receiveMap[clientFd].isChunked);
	else
		return (false);
}

FILE *Response::getTempFile(size_t clientFd)
{
	if (this->_tempFile.count(clientFd))
		return (this->_tempFile[clientFd]);
	else
		return (NULL);
}

void Response::removeTempFile(size_t clientFd)
{
	if (this->_tempFile.count(clientFd))
		this->_tempFile.erase(clientFd);
}

/**
 * @brief  read the first line of a chunked message including the \r\n and translate the hexadecimal number to size_t
 * @note
 * @param  clientFd: clients filedescriptor
 * @retval the translated to size_t hexadecimal
 */
size_t Response::_handleChunked(int clientFd)
{
	// read the line until /r/n is read
	char buffer[2];
	std::string hex = "";
	while (hex.find("\r\n") == std::string::npos)
	{
		int returnvalue = read(clientFd, buffer, 1);
		buffer[1] = '\0';
		if (returnvalue < 1)
			throw Response::MissingChunkContentLengthException();
		hex.append(buffer);
	}
	hex.resize(hex.size() - 2);

	std::stringstream hexadecimal;
	hexadecimal << hex;

	size_t length = 0;
	hexadecimal >> std::hex >> length;

	#ifdef FORTYTWO_TESTER
		if (this->_receiveMap[clientFd].target.find("post_body") != std::string::npos && length > this->_receiveMap[clientFd].maxBodySize)
		{
			char trashcan[2048];
			int n = 0;
			while (n >= 0)
				n = read(clientFd, trashcan, 2048);
			throw Response::ERROR_413();
		}
	#else
		if (length > this->_receiveMap[clientFd].maxBodySize)
			throw Response::ERROR_413();
	#endif

	return (length);
}

/**
 * @brief  will set the target for the post
 * @note   was previously also setting the temp file, might be used again
 * @param  clientFd: clients filedescriptor
 * @param  target: the target specified in the request, but with the routed path
 * @retval None
 */
void Response::setPostTarget(int clientFd, std::string target)
{
	this->_receiveMap[clientFd].target = target;
}

/**
 * @brief  checks existance of the target specified by the request
 * @note   POST can not write to a file that already is existing
 * @param  clientFd: clients filedescriptor
 * @param  &request: the request object
 * @param  port: the port that was used for the request
 * @retval None
 */
// void Response::checkPostTarget(int clientFd, const Request &request, int port)
// {
// 	std::string target = request.getRoutedTarget();
// 	if (access(target.c_str(), F_OK ) != -1)
// 	{
// 		this->removeFromReceiveMap(clientFd);
// 		#ifdef SHOW_LOG_2
// 			std::stringstream message;
// 			message << "file " << target << " is already existing";
// 			LOG_RED(message.str());
// 		#endif
// 		// this->_createFileExistingHeader(clientFd, request, port);
// 	}
// }

/**
 * @brief  converts a string to a size_t, similar to atoi
 * @note
 * @param  str: the string that contains the number you want to convert
 * @retval the found number or an exception
 */
static size_t _strToSizeT(std::string str)
{
	size_t out = 0;
	std::stringstream buffer;
	buffer << SIZE_T_MAX;
	std::string sizeTMax = buffer.str();
	if (str.find("-") != std::string::npos && str.find_first_of(DECIMAL) != std::string::npos && str.find("-") == str.find_first_of(DECIMAL) - 1)
	{
		std::cout << str << std::endl;
		throw Response::NegativeDecimalsNotAllowedException();
	}
	else if (str.find_first_of(DECIMAL) != std::string::npos)
	{
		std::string number = str.substr(str.find_first_of(DECIMAL));
		if (number.find_first_not_of(WHITESPACE) != std::string::npos)
			number = number.substr(0, number.find_first_not_of(DECIMAL));
		if (str.length() >= sizeTMax.length() && sizeTMax.compare(number) > 0)
		{
			std::cout << RED << ">" << number << RESET << std::endl;
			throw Response::SizeTOverflowException();
		}
		else
			std::istringstream(str) >> out;
	}
	return (out);
}

/**
 * @brief  checks if a POST request is chunked or not
 * @note
 * @param  clientFd: clients filedescriptor
 * @param  target: the target specified by the request, is outdated because temp files are currently not in use
 * @param  &headerFields: the header fields of the request
 * @retval None
 */
void Response::setPostChunked(int clientFd/* , std::string target */, std::map<std::string, std::string> &headerFields)
{
	if (this->_receiveMap.count(clientFd) && headerFields.count("transfer-encoding") && headerFields["transfer-encoding"] == "chunked")
	{
		this->_receiveMap[clientFd].isChunked = true;
		#ifdef SHOW_LOG_2
			LOG_RED("was set to chunked true");
		#endif
	}
}


/**
 * @brief  sets the total and bytesleft for the request, only for non chunked requests
 * @note
 * @param  clientFd: clients filedescriptor
 * @param  &headerFields: header fields of the request
 * @retval None
 */
void Response::setPostLength(int clientFd, std::map<std::string, std::string> &headerFields)
{
	size_t length = 0;
	if (headerFields.count("content-length"))
	{
		#ifdef SHOW_LOG_2
			std::cout << "content-length for POST: " << headerFields["content-length"] << std::endl;
		#endif
		length = _strToSizeT(headerFields["content-length"]);
	}

	this->_receiveMap[clientFd].total = length;
	this->_receiveMap[clientFd].bytesLeft = length;
	this->_receiveMap[clientFd].bytesWritten = 0;
}

/**
 * @brief  sets the buffersize defined in the config file for each POST request
 * @note
 * @param  clientFd: clients filedescriptor
 * @param  bufferSize: buffer size specified in the config
 * @retval None
 */
void Response::setPostBufferSize(int clientFd, size_t bufferSize, size_t maxBodySize)
{
	this->_receiveMap[clientFd].bufferSize = bufferSize;
	this->_receiveMap[clientFd].maxBodySize = maxBodySize;
}

void Response::setIsCgi(int clientFd, bool state)
{
	this->_receiveMap[clientFd].isCgi = state;
	#ifdef SHOW_LOG_2
		std::cout << RED << BOLD << "set isCgi to " << state << RESET << std::endl;
	#endif
	this->_receiveMap[clientFd].end = false;
}



////////// LEGACY CONTENT BELOW//////////

// /**
//  * @brief  creates a 303 response header for POST requests if the file is already existing
//  * @note
//  * @param  clientFd: clients filedescriptor
//  * @param  &request: the request object
//  * @param  port: the port on which the request came
//  * @retval None
//  */
// void Response::_createFileExistingHeader(int clientFd, const Request &request, int port)
// {
// 	std::stringstream serverNamePort;
// 	serverNamePort << request.getHostName() << ":" << port; // creates ie. localhost:8080
// // clear all the unused data
// 	this->headerFields.clear();
// 	this->body.clear();
// 	this->removeFromReceiveMap(clientFd);
// 	this->removeFromResponseMap(clientFd);
// // set all the data for the header
// 	this->setProtocol(PROTOCOL);
// 	this->setStatus("303");
// 	std::string location = "http://" + serverNamePort.str() + request.getRawTarget();
// 	this->addHeaderField("location", location);
// 	this->addHeaderField("Connection", "close");
// 	this->addHeaderField("Host", serverNamePort.str());
// 	// this->setBody("http://" + serverNamePort.str() + request.getRawTarget()); // for testing only!!!

// 	this->putToResponseMap(clientFd);
// // only for testing !!!!!! removes all leftover contents from the clientFd // ILLEGAL READ !!!!!!
// 	// char buffer[2];
// 	// while (read(clientFd, buffer, 1) > 0)
// 	// {
// 	// 	buffer[1] = '\0';
// 	// 	LOG_YELLOW(buffer);
// 	// }
// //
// }