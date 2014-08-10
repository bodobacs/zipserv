//#include <unistd.h>
#include <sys/types.h> //socket, bind
#include <sys/socket.h>
#include <arpa/inet.h> //htons stb
#include <iostream>
#include <cstdio>
#include <cctype>
#include <cstring>

#include "../zlib/contrib/minizip/unzip.h"

using namespace std;

//atmeneti valtozok

char zipname[] = "../abs-guide.zip";
char root_path[] = "";

//-----------------
const int buffer_size = 2048;
char buffer[buffer_size+1];//a záró nullának +1

bool url_found = false;
int url_ends_at = 0;
/*
class cServHttp
{
public:
	int client_socket;

	string zipname;
	unzFile zipfile;

	streambuf buffer;

	bool init(int cli_socket = 0, const string &zname)
	{
		//clearall
		if(0 != cli_socket 
		&& 0 < zname.length)
		{
			client_socket = cli_socket;
			zipname = zname;
		}

		return false;
	}
	
	void clean_all(void)
	{
	}
	
	void get_request(void)
	{
		
	}
};*/

unzFile zipfile = NULL;

bool get_request(int cli_socket)
{
	buffer[0] = buffer[4] = 0;
	int c = read(cli_socket, buffer, 4);
	if(c == 3)
	{
		for(int i = 0; i < 3; i++) buffer[i] = tolower(buffer[i]);
		if(0 == strcmp("get " , buffer))
		{
			char k = getc(cli_socket);
			int i = 0;
			for(i = 0; i < buffer_size && k != " "; i++)
			{
				buffer[i] = k;
				k = getc(cli_socket);
			}

			buffer[i] = 0;
//			if(i == 0) buffer = "index.html";//ha nincs megadva filenév akkor ezt keresi, ha nincs akkor listát fog generálni

			if(i == buffer_size) clog << "Small adress buffer" << endl;
			cout << "GET: " << buffer << endl;
			return true;
			 
		}else{
			cerr << "No GET!" << endl;
		}
	}else{
		cerr << "No request." << endl;
	}
	return false;
}
/*
bool send_page(int cli_socket)
{
	
	int k = write(cli_socket, );
	return false;
}

bool send_message(int cli_socket)
{

}
*/

void webserv(int cli_socket)
{
	clog << endl << "Serving client" << endl;

	if(get_request(cli_socket))
	{
		
		send_file_list();
	}

	close(cli_socket);

	clog << endl << "Serving client" << endl;
}


bool open_zipfile(void)
{
	close_zipfile();

	zipfile = unzOpen(zipname);
	if(NULL != zipfile)
	{
		return true;	
	}else{
		cerr << "unzOpen failed" << endl;
	}
	return false;
}

void close_zipfile(void)
{
	if(NULL != zipfile)
	{
		unzClose(zipfile);
		zipfile = NULL;
	}
}

bool send_file(void)
{
	return false;
}

bool send_file_list(int cli_socket)
{	
	if( NULL != zipfile)
	{
		int ret = unzGoToFirstFile(zipfile);
			for(int f = 1; UNZ_OK == ret && f < 10000; f++)//10000 csak egy korlát
			{
				unz_file_info file_info;
				int filename_buffer_size = 1024;
				char filename_buffer[filename_buffer_size];

				ret = unzGetCurrentFileInfo(zipfile, &file_info, filename_buffer, filename_buffer_size, NULL, 0, NULL, 0);
				if(UNZ_OK == ret)
				{
				}else{
					cerr << "unzGetCurrentFileInfo failed" << endl;
				} 

				ret = unzGoToNextFile(zipfile);
			}//for
		}else{
			cerr << "unzGetGlobalInfo failed" << endl;
		}
	
	return false;
}

bool find_file(const char *path = 0)
{
	if(NULL != zipfile && 0 != path)
	{
		if( UNZ_OK == unzLocateFile(zipfile, path, 2))
		{
			send_file();
			return true;
		}else{
			send_file_list();
			return true;
		}
	}

//	send_message(); //send error message
	return false;
}



int main(int argc, char **argv)
{

// demon()?
//szerver listener socket
	int listen_socket; //ezen jonnek a keresek
	const int listen_port = 19000; //server portja
	struct sockaddr_in server_address;

	//szerver cimenek beallitasa
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(listen_port);
	server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//INADDR_ANY

//amikor egy kapcsolat letrejon csinal egy childprocess-t es annak adja at a kliens socketet
//client socket
	int client_socket;
	struct sockaddr_in client_address;
	socklen_t length;

	if(0 <= (listen_socket = socket(AF_INET, SOCK_STREAM, 0))) //domain, type, protocol(tipusonkent egy szokott lenni)
	{

		if(0 <= bind(listen_socket, (struct sockaddr *) &server_address, sizeof(server_address)))
		{
			if(0 == listen(listen_socket, 10))//a kapcsolodasokra figyel a 2. param a varolista hosszat adja meg
			{
				while(1)
				{
					if(0 <= (client_socket = accept(listen_socket, (struct sockaddr *)&client_address, &length)))
					{
						cout << endl << "New client:" << ntohl(client_address.sin_port) << endl;
						webserv(client_socket);
					}else{
						perror("accept");
					}
				}//while
			}else{
				perror("listen");
			}
		}else{
			perror("bind");
		}
	}

	close(listen_socket);
	cout << endl << "Server stopped" << endl;

return 0;
}
