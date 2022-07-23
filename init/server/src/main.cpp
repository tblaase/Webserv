#include "Server.hpp"
#include "Config.hpp"

std::string parseArgv(int argc, char **argv) // maybe change to static void function or include it into some object
{
	std::string defaultConfPath = "config/www.conf";
	if (argc == 1)
	{
		return (defaultConfPath);
	}
	else if (argc > 2)
	{
		std::cerr << RED << "Please only use webserv with config file as follows:" << std::endl << BLUE << "./webserv <config_filename.conf>" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string sArgv = argv[1];
	std::string ending = ".conf";
	if ((argv[1] + sArgv.find_last_of(".")) != ending)
	{
		std::cerr << RED << "Please only use webserv with config file as follows:" << std::endl << BLUE << "./webserv <config_filename.conf>" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	return (sArgv);
}

int main(int argc, char **argv)
{
	Config *config = new Config();
	try
	{
		config->start(parseArgv(argc, argv));
	}
	catch (std::exception &e)
	{
			std::cerr << RED << e.what() << RESET << std::endl;
			delete config;
			return (EXIT_FAILURE);
	}
	Server *test = new Server(config); // somehow pass the listen ports to the server ??
	// std::cout << BLUE << test->cluster.size() << " elements found inside map" << RESET << std::endl;
	// test if the data inside the cluster is accessable
	// std::string firstName = "weebserv";
	// std::string secondName = "anotherone";

	// std::cout << "### attempting to print contents of the configStructs" << std::endl;
	#ifdef SHOW_LOG
		config->printCluster();
	#endif
	// system("leaks webserv");
	test->run();
	delete config;
	delete test;
	config = NULL;
	test = NULL;
	// system("leaks webserv");
	return (0);
}
