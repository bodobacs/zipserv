//#include <unistd.h>
#include <sys/types.h> //socket, bind
#include <sys/socket.h>
#include <arpa/inet.h> //htons stb
#include <iostream>
#include <cstdio>
#include <cctype>
#include <cstring>

using namespace std;

const int buffer_size = 2048;
char buffer[buffer_size];

bool url_found = false;
int url_ends_at = 0;

bool get_request(int cli_socket)
{
	int c = read(cli_socket, buffer, buffer_size-1);
	if(c != 0)
	{
		if(c <= buffer_size)
		{
			clog << "Request read. Buffer is enough." << endl;
			buffer[c] = 0;
		}else{
			clog << "Request read. Buffer is not enough." << endl;
			buffer[buffer_size-1] = 0;
		}

		for(int i=0; i < c; i++) buffer[i] = tolower(buffer[i]);

		clog << buffer << endl;

		if(0 == strncmp(buffer, "get ", 4))
		{
			clog << "Found 'GET ' in first 3 character" << endl;
			int j = 0;
			for(j=4; j<c && buffer[j] != ' '; j++);
			if(j<c)
			{
				clog << "Found space after URI." << endl;
				url_ends_at = j;
				
				if(0 == strncmp(buffer+j, " http", 5))
				{
					clog << "Found end of URL" << endl;
					return true;
				}
			}else{
				cerr << "No end of URL" << endl;
			}
		}else{
			cerr << "Not found GET request" << endl;
		}
	}else{
		cerr << "No client message." << endl;
	}
	return false;
}

bool send_page(int cli_socket)
{
	
	int k = write(cli_socket, );
	return false;
}

bool send_message(int cli_socket)
{

}

void webserv(int cli_socket)
{
	clog << endl << "Serving client" << endl;

	while(get_request(cli_socket))
	{
		
	}

	close(cli_socket);

	clog << endl << "Serving client" << endl;
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
