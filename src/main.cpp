
#include "zsrv.h"
#include <iostream>

int main(int argc, char **argv)
{

	if(1 < argc)
	{
//	signal(SIGINT, signalhandler); //that little shit makes an "undefined reference in main" error on NDK

	 std::cout << " Built on " __DATE__  " " __TIME__ << std::endl;

		czsrv server(argv[1], 19000);

		if(server.open_zipfile())
		{
			server.run_server();

			server.close_zipfile();
		}

	 std::cout << std::endl << "[" << " stopped. Bye!]" << std::endl << std::endl;

	}

	return 0;
}
