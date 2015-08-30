#include <iostream>
#include "archives.h"


int main(int argc, char **argv)
{
	carchive archive;

	if(argc > 1 && archive.open(argv[1]))
	{
		char filenamebuffer[512];
		if(archive.list_start())
		{
			while(archive.list_next_file(filenamebuffer, 512))
			{
				std::cout << filenamebuffer << std::endl;
			}
		}

		archive.find_file("/EasyCHM.html");
		filenamebuffer[512] = 0;
		while(archive.read(filenamebuffer, 511))
		{
			std::cout << filenamebuffer << std::flush;
		}

	}else{
        fprintf(stderr, "failed to open %s\n", argv[1]);
        exit(1);
	}

	return 0;
}

