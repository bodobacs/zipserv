
#include "zsrv.h"
#include <iostream>
#include <signal.h> //signal handler

czsrv server;

void signalhandler(int signum) // ????????????????????????
{
	std::clog << "[Signal caught]: " << signum << std::endl;

	server.stop();
}

int main(int argc, char **argv)
{

	if(1 < argc)
	{
//	signal(SIGINT, signalhandler); //that little shit makes an "undefined reference in main" error on NDK

	 std::cout << " Built on " __DATE__  " " __TIME__ << std::endl;

		server.init(argv[1], 19000);

		if(server.open_zipfile())
		{
			server.run_server();

			server.close_zipfile();
		}

	}else{
		std::cout << "Missing zip file in arguent! Cannot start server." << std::endl;
	}

	return 0;
}
