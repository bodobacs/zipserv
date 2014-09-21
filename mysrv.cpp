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
#include <sstream>
#include <iomanip>
#include <unistd.h> //select()
#include <signal.h> //signal handler

#include "../zlib/contrib/minizip/unzip.h"
#include "mimetypes.h"

using namespace std;

const string APPNAME_str("zipserv-dev");

const string COLOR_ERROR	("\x1B[31m");
const string COLOR_OK		("\x1B[32m");
const string COLOR_WARNING	("\x1B[33m");
const string COLOR_RESET	("\033[0m" );

const string NOT_FOUND("NOT_FOUND");

const std::map<int, std::string> unzip_return_codes = {

{ UNZ_OK, "UNZ_OK" },
{ UNZ_END_OF_LIST_OF_FILE, "UNZ_END_OF_LIST_OF_FILE" },
{ UNZ_ERRNO, "UNZ_ERRNO" },
{ UNZ_EOF,"UNZ_EOF" },
{ UNZ_PARAMERROR, "UNZ_PARAMERROR" },
{ UNZ_BADZIPFILE, "UNZ_BADZIPFILE" },
{ UNZ_INTERNALERROR, "UNZ_INTERNALERROR"},
{ UNZ_CRCERROR, "UNZ_CRCERROR"}

};

//translates unzip return values to string
const string &get_unzip_error(int code)
{
	const map<int, string>::const_iterator itr = unzip_return_codes.find(code);

	if(unzip_return_codes.end() != itr) return (itr->second);

	cerr << "unzip code not found" << endl;
	return NOT_FOUND;
}


//atmeneti valtozok

//string zipname("../abs-guide.zip");
string zipname("irwin.zip");

//-----------------
const int buffer_size = 2048;//ennek elégnek kéne lennie, firefoxnak van 8kb
string request_str(buffer_size, '0');
string URI_str;

int client_socket = 0;

unzFile zipfile = NULL;

void send_NOT_FOUND(const string &what)//ez jó
{
	stringstream content_ss;

	content_ss << "<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>" << what << " Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n";

	stringstream response;
	response << "HTTP/1.1 404 Not Found\nContent-Length: " << content_ss.str().length() << "\nConnection: close\nContent-Type: text/html\n\n" << content_ss.str();

	if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("Send message content");

	clog << "[NOT FOUND 404 sent] " << endl;
//	cout << response.str() << endl;
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

	zipfile = unzOpen(zipname.c_str());
	if(NULL != zipfile)
	{
		return true;	
	}else{
		cerr << "[unzOpen failed]" << endl;
	}
	return false;
}

bool send_file(void)
{
	bool success = false;
	clog << "[Sending file]" << endl;

	unz_file_info file_info;
	int filename_buffer_size = 1024;
	char filename_buffer[filename_buffer_size];
	int chunks = 0;
	
	int ret = unzGetCurrentFileInfo(zipfile, &file_info, filename_buffer, filename_buffer_size, NULL, 0, NULL, 0);
	if(UNZ_OK == ret)
	{
		if(UNZ_OK == (ret = unzOpenCurrentFile(zipfile)))
		{
//			clog << "[Uncompressed size] " << file_info.uncompressed_size << endl;

			string::size_type pos = URI_str.find_last_of('.');
			string filetype = URI_str.substr(pos+1);

			map<string, string>::const_iterator itr = mimetypes.find(filetype);
//			if(string::npos != pos && string::npos != URI_str[pos+1] && 
			if(mimetypes.end() != itr)
			{
				filetype.assign(itr->second);
			}else{
				filetype.assign("text/html");
			}

			clog << "[mimetype] " << itr->second << endl;

			const int buffsize = 1024;
			char buffer[buffsize];
			stringstream response;
		response << "HTTP/1.1 200 OK\r\nServer: " << APPNAME_str << "\r\nContent-Type: " << filetype << setbase(16) << "\r\nTransfer-Encoding: chunked\r\n\r\n" << flush;
			
			while(0 <= (ret = unzReadCurrentFile(zipfile, buffer, buffsize)))
			{
				if(0 == ret)
				{
					//last chunk
					response << ret << "\r\n\r\n" << flush;
					success = true;
				}else{
					response << ret << "\r\n";
					response.write(buffer, ret);
					response << "\r\n" << flush;
				}

//				clog << response.str() << flush;

				if(-1 != (ret = send(client_socket, response.str().data(), response.str().length(), 0)))
				{
					chunks++;
					if(success) break;
				}else{ 
					perror("[send] "); 
					success = false;
					break;
				}
				response.str(string()); //empty response stream
			}//while else cerr  << "[unzRead failed: " << ret << " ]" << endl;
			
			if(UNZ_CRCERROR == unzCloseCurrentFile(zipfile)) success = false;

		}else cerr << "[unzOpen failed]" << endl;
	}else cerr << "[Reading zip's file info failed]" << endl;

	if(success)
	{
		clog << "[" << chunks << " chunks]" << endl;
	}else{
		cerr << "[unzip error] " << get_unzip_error(ret) << endl;
		send_NOT_FOUND(URI_str);
	}

	return success;
}

void send_file_list(void)
{	
	stringstream content;
	content << "<html><header>File list</header><body>" << endl;

	if( NULL != zipfile)
	{
		int ret = unzGoToFirstFile(zipfile);
		for(int f = 0; UNZ_OK == ret && f < 10000; f++)//10000 csak egy korlát
		{
			unz_file_info file_info;
			int filename_buffer_size = 1024;
			char filename_buffer[filename_buffer_size];

			ret = unzGetCurrentFileInfo(zipfile, &file_info, filename_buffer, filename_buffer_size, NULL, 0, NULL, 0);
			if(UNZ_OK == ret)
			{
				content << "<li><a href=\"" << filename_buffer << "\"/>" << filename_buffer << "</li>" << endl;
			}else{
				if(!f) content << "<h1>Files not found in zip!</h1>" << endl;
				break;
			} 

			ret = unzGoToNextFile(zipfile);
		}//for
	}else{
		content << "<h1>Cannot list zip!</h1>" << endl;
		
//		cerr << "unzGetGlobalInfo failed" << endl;
	}

	content << "</body></html>" << endl;
	stringstream response;

	response << "HTTP/1.1 200 OK\nServer: " << APPNAME_str << "\nContent-Length: " << content.str().length() << "\nConnection: keep-alive\nContent-Type: html\n\n" << content.str() << flush;

	if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("send");
	return;
}

void send_error(void)
{
	cerr << "send_error() called" << endl;
}

bool parse_request(void)
{
	stringstream ss(request_str);

	string token;
	ss >> token;
	if(!token.compare("GET"))// || !token.compare("HEADER"))
	{
//		clog << "[Found] " << token << endl;
		ss >> URI_str;
		if(0 < URI_str.length())
		{
			cout << "[URL] " << URI_str << endl;
			return true;
		}
	}

	clog << "[Not a GET request] " << token << endl;

//BAD REQUEST
	return false;
}

bool get_request(void)
{
}

bool find_file(void)
{
	if(NULL != zipfile)
	{
		clog << "[Searching zip for] " << URI_str.c_str()+1 << endl;
		unzGoToFirstFile(zipfile);
		if( UNZ_OK == unzLocateFile(zipfile, URI_str.c_str()+1, 2))
		{
			clog << "[Found] " << URI_str.c_str()+1 << endl;
			return true;
		}
	}

	clog << "[Not found] " << URI_str.c_str()+1 << endl;
	return false;
}

bool webserv(void)
{
	clog << endl << "[Socket ready, serving client]" << endl;

	clog << "[Getting request]" << endl;
	char buffer[buffer_size];
	int ret;

	if(0 < (ret = recv(client_socket, buffer, buffer_size, 0)))
	{
		request_str.assign(buffer, ret);
//		clog << endl << "[Request start] " << endl << request_str << endl << "[Request end]" << endl;

		if(parse_request())
		{
			clog << "[RESPONSE START]" << endl;
			if(find_file())
			{
				send_file();
			}else{
				if(!URI_str.compare("/"))
				{
					URI_str.assign("/index.html");
					if(find_file())
					{
						send_file();
					}else{
						send_file_list();
					}
				}else{
					send_NOT_FOUND(URI_str);
				}
			}

			return true;
			clog << "[RESPONSE END]" << endl;
	//		sleep(1); //EZ NAGYON KELL, ki kell találni valami jobbat helyette
		}

	}else if(ret == -1)
	{
		perror("recv");
	}

	return false;
}

//mimetypes.cpp-be valo
void list_mimetypes(void)
{
	for(map<string, string>::const_iterator  itr = mimetypes.begin(); itr != mimetypes.end(); itr++)
	{
		cout << itr->first << " " << itr->second << endl;
	}
}

bool run = true; 

void signalhandler(int signum)
{
	cerr << COLOR_ERROR << "[Signal caught]: " << signum << endl;

	run = false;
}

int main(int argc, char **argv)
{
	clog << endl << "[zipserv started]" << endl << endl;

	signal(SIGINT, signalhandler);

	open_zipfile();

// demon()?
//szerver listener socket
	int listen_socket; //ezen jonnek a keresek
	const int listen_port = 19000; //server portja
	struct sockaddr_in server_address;

	//szerver cimenek beallitasa
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(listen_port);
	server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//INADDR_ANY

	if(0 <= (listen_socket = socket(AF_INET, SOCK_STREAM, 0))) //domain, type, protocol(tipusonkent egy szokott lenni)
	{

		if(0 <= bind(listen_socket, (struct sockaddr *) &server_address, sizeof(server_address)))
		{
			if(0 == listen(listen_socket, 10))//a kapcsolodasokra figyel a 2. param a varolista hosszat adja meg
			{
				//add listen_socket
				fd_set	open_sockets;
				FD_ZERO(&open_sockets);

				FD_SET(listen_socket, &open_sockets);
				int greatest_socket = listen_socket;
				
				while(run)
				{
					fd_set read_fds;
					FD_ZERO(&read_fds);
					read_fds = open_sockets;

//					struct timeval tv;
//					tv.tv_sec = 10; tv.tv_usec = 0;

//					int r = select(greatest_socket+1, &read_fds, NULL, NULL, &tv);
					int r = select(greatest_socket+1, &read_fds, NULL, NULL, NULL);
					if(r > 0)
					{//there sockets to check
						for(int i=0; i <= greatest_socket; i++)
						{
							if(!(FD_ISSET(i, &read_fds))) continue; //this i is not in the set 
							if(i == listen_socket)
							{//new connection

//								struct sockaddr_in client_address;
//								socklen_t length;
//								int new_socket = accept(listen_socket, (struct sockaddr *)&client_address, &length);

								int new_socket = accept(listen_socket, NULL, NULL);
								if(-1 != new_socket)
								{
									cout << "[New connection created]" << endl;
									FD_SET(new_socket, &open_sockets);

									greatest_socket = new_socket;

									//linger lin; lin.l_onoff = 1; lin.l_linger = 5; if(setsockopt(client_socket, SOL_SOCKET, SO_LINGER, (void*)&lin, sizeof(lin))) perror("setsockopt");

								}else perror("[ accept() ] "); 

							}else{
								client_socket = i;
								if(!webserv()) //socket state changed
								{
									clog << "[Closing socket] " << i << endl;
									close(i);
									FD_CLR(i, &open_sockets);
								}
							}//check socket
						}//for
					}else if(r == 0){//timeout
						clog << "[Waiting ...]" << endl;
					}else{
						perror("select: ");
						run = false;
					}

				}//while
			}else perror("[ listen() ] ");
		}else perror("[ bind() ] "); 

		close(listen_socket);
	}

	close_zipfile();

	cerr << COLOR_RESET << flush;
	clog << COLOR_RESET << flush;

	cout << endl << "[" << APPNAME_str << " stopped. Bye!]" << endl << endl;
	return 0;
}


