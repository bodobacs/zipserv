/*
Mivel marha nehéz olyan értelmezőt írni ami minden http kérést hatékonyan kezel ezért meghatározom mi a fenére is van szükség.
- csak GET kérésekre vagyok kíváncsi, többit dobom
- persistent kapcsolatban akarok kommunikálni a clienssel, mert ez hatékonyabb mint minden fájlhoz nyitni egy kapcsolatot
- HEADER-t sajna kell elemezgetni pl: keep alive hasznos lenne, hogy befejezze a weboldal töltését a browser
*/

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

const string msg_SUCCESS("HTTP/1.1 200 OK\nServer: zipserv\nContent-Type: text/html\n\n<html><head><title>File list</title></head><body><ol>\n");

const string msg_NOT_FOUND("HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n");

//-----------------
const int buffer_size = 2048;//ennek elégnek kéne lennie, firefoxnak van 8kb
char buffer[buffer_size+1];//a záró nullának +1

bool url_found = false;
int url_ends_at = 0;

int client_socket = 0;


unzFile zipfile = NULL;

void read_all_request(void)
{
	int ret = read(client_socket, buffer, buffer_size);
	buffer[buffer_size] = 0;

	cout << "Full request: " << buffer << endl;
}

//CRLF = "\r\n"
int read_section(const char stop)
{
	int j, ret;

	for(j = 0; j < buffer_size && (ret = read(client_socket, buffer+j, 1)) && buffer[j] != stop; ('\n' == buffer[j] || '\r' == buffer[j] ? 0 : j++))
	{
/*		if('\r' == buffer[j] || '\n' == buffer[j])
		{
			j--;	
		}	
*/		buffer[j] = tolower(buffer[j]);
	}

	if(ret == 0 || ret == -1 || j == buffer_size)
	{
		buffer[buffer_size] = 0;
		cerr << "Socket read failed" << buffer << endl;
		return 0;
	}

	buffer[j] = 0;

	cout << "j=" << j << endl << "stop:\"" << stop << "\"" << endl << "section: " << buffer << endl << endl;

	return j;
}
/*
bool is_GET(void)
{
	int j, ret;
	char minibuf[4];
	bool swallow = false;

	for(j = 0; j < 4 && (ret = read(client_socket, minibuf+j, 1)); (swallow ? 0 : j++))
	{
		if( isspace(minibuf[j]) )
		{
			swallow = true;
		}else{
			minibuf[j] = tolower(minibuf[j]);
			swallow = false;
		}
	}

	minibuf[4] = 0;

	if(0 == strcmp("get", minibuf)) return true;
	return false;
}
*/

//a folyamatos kapcsolat miatt a kérések egymás után érkeznek, ezért keres egy "GET " mintát és az ez után következő szakaszt tekinti a kért URL-nek. Gondok lehetnek, ha POST-ban kap valami tartalmat amiben van ilyen minta, de ez elég esélytelen ha mégis gondok lennének akkor lehet nézni, hogy milyen request jott és ha nem get akkor dobni egy hibát
bool find_GET(void)
{
	const char req[] = {"get "};
	bool last_is_space = true;//if GET at the start

	int ret;
	while(ret = read(client_socket, buffer, 1))
	{
		if(isspace(buffer[0]))
		{
			last_is_space = true;
		}else{
			if(last_is_space)
			{
				last_is_space = false;
				buffer[0] = tolower(buffer[0]);
				int i = 0; 
				for(; ret && i < sizeof(req)-1 && buffer[0] == req[i] ; i++ )
				{
					ret = read(client_socket, buffer, 1);
					buffer[0] = tolower(buffer[0]);
				}

//				cout << "i=\"" << i << "\" buffer[0]=\"" << buffer[0] << "\" sizeof(req)=" << sizeof(req) << endl;
				if(ret && i == sizeof(req)-1) return true;
			}
		}
	}
	cerr << "find_GET failed" << endl;
	return false;
}

bool get_URL(void)
{
	int j, ret;
	bool swallow = false;

	for(j = 0; j < buffer_size && (ret = read(client_socket, buffer+j, 1)); (swallow ? 0 : j++))
	{	
		if( isspace(buffer[j]) )
		{
			swallow = true;
			if(j) break;//már nem az elejéről enné a whitespace karikat tehát a végén vagyunk akkor ennyi
		}else{
			buffer[j] = tolower(buffer[j]);
			swallow = false;
		}
	}

	buffer[j] = 0;

	if(ret == 0 || ret == -1 || j == buffer_size)
	{
		cerr << "Socket read failed" << buffer << endl;
		return false;
	}

	cout << "URL:\"" << buffer << "\"" << endl;
	return true;

}

void close_zipfile(void)
{
	if(NULL != zipfile)
	{
		unzClose(zipfile);
		zipfile = NULL;
	}
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

bool send_file(void)
{
	return false;
}

void send_file_list(void)
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
					sprintf(buffer, "<li><a href=\"%s\"/>%s</li>", filename_buffer, filename_buffer);
					write(client_socket, buffer, strlen(buffer));
					//cout << buffer << endl;
				}else{
					cerr << "unzGetCurrentFileInfo failed" << endl;
				} 

				ret = unzGoToNextFile(zipfile);
			}//for
		}else{
			cerr << "unzGetGlobalInfo failed" << endl;
		}
	
	return;
}

void send_error(void)
{
	cerr << "send_error called" << endl;
}

bool get_request(void)
{
	if(open_zipfile() && find_GET() && get_URL())
	{
		clog << "GET and URL found" << endl;
		
		
		sprintf(buffer, "HTTP/1.1 200 OK\nServer: zipserv\nContent-Type: text/html\n\n<html><head><title>File list</title></head><body><ol>\n");
		write(client_socket, buffer, strlen(buffer));

		send_file_list();		

		sprintf(buffer, "</ol></body></html>\n\n");
		write(client_socket, buffer, strlen(buffer));

		close_zipfile();
		return true;
	}else{
		send_error();
	}
/*
	if(read_section(' ') && && read_section(' '))
	{
	}else{
		cerr << "Bad request, not GET" << endl;
	}
*/
	return false;
}
bool find_file(void)
{
	if(NULL != zipfile)
	{
		if( UNZ_OK == unzLocateFile(zipfile, buffer, 2))
		{
			send_file();
			
			clog << "File found." << endl;
			return true;
		}else{
			
			write(client_socket, msg_NOT_FOUND.c_str(), msg_NOT_FOUND.length());

		//	send_file_list();

			cerr << "File not found" << endl;
			return true;
		}
	}

//	send_message(); //send error message
	return false;
}

void webserv(void)
{
	clog << endl << "Serving client" << endl;

//	read_all_request();
	while(get_request())
	{
	}
	
	sleep(1); //EZ NAGYON KELL, ki kell találni valami jobbat helyette
	close(client_socket);

	clog << endl << "Socket closed." << endl;
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
//	int client_socket;
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

						linger lin; lin.l_onoff = 1; lin.l_linger = 5;
					/*	if(setsockopt(client_socket, SOL_SOCKET, SO_LINGER, (void*)&lin, sizeof(lin)))
						{
							perror("setsockopt");
						}
*/
						webserv();
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
