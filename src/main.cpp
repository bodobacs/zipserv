#include "zsrv.h"
#include <iostream>
#include <signal.h> //signal handler

czsrv server;

void signalhandler(int signum) // ???????????????????????? Do not use in android!
{
	std::clog << "[Signal caught]: " << signum << std::endl;

	server.stop();
}

int main(int argc, char **argv)
{
	std::cout << "zipserv\nBuilt on " __DATE__  " " __TIME__ << std::endl;

	if(1 < argc)
	{
//	signal(SIGINT, signalhandler); //that little shit makes an "undefined reference in main" error on NDK

		int port = 19000;
		
		if(2 < argc) //there is a port number
		{
			std::string str_port(argv[2]);
			int t = std::stoi(str_port);

			if(0 < t) port = t;
			else std::cout << "Second parameter not a port number! Using default port " << port << std::endl;
		}

		std::string filename(argv[1]);
		std::cout << "Starting to serve " << filename << " on port " << port << " ..." << std::endl;

		server.init(filename, port);

		if(server.open())
		{
			server.run_server();
		}
	}else{

		std::cout << "Missing operands\nUsage: zipserv ZIP_FILE [PORT_NUMBER]\n" << std::endl;
	}

	return 0;
}

