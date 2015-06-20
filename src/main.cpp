
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

	int port = 19000;

	if(1 < argc)
	{
//	signal(SIGINT, signalhandler); //that little shit makes an "undefined reference in main" error on NDK

		
		if(2 < argc) //there is a port number
		{
			int t = 0;

			if(1 == sscanf(argv[2], "%i", &t))
			{
				port = t;
			}else{
				std::cout << "Second parameter not a port number! Using default port " << port << std::endl;
			}
		}

		std::cout << "Starting to serve " << argv[1] << " on port " << port << " ..." << std::endl;
		server.init(argv[1], port);

		if(server.open_zipfile())
		{
			server.run_server();

			server.close_zipfile();
		}

	}else{

		std::cout << "Missing operands\nUsage: zipserv ZIP_FILE [PORT_NUMBER]\n" << std::endl;
	}

	return 0;
}

