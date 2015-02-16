/*
Mivel marha nehéz olyan értelmezőt írni ami minden http kérést hatékonyan kezel ezért meghatározom mi a fenére is van szükség.
- csak GET kérésekre vagyok kíváncsi, többit dobom
- persistent kapcsolatban akarok kommunikálni a clienssel, mert ez hatékonyabb mint minden fájlhoz nyitni egy kapcsolatot
- HEADER-t sajna kell elemezgetni pl: keep alive hasznos lenne, hogy befejezze a weboldal töltését a browser
*/

#include "zsrv.h"
//#include <unistd.h>
#include <sys/types.h> //socket, bind
#include <sys/socket.h>
#include <arpa/inet.h> //htons stb
#include <iostream>
#include <cstdio>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <unistd.h> //select()
#include <signal.h> //signal handler

using namespace std;

const std::string TAG("zsrv.cpp");
const std::string NOT_FOUND(" NOT_FOUND");
const std::string APPNAME_str("zipserv-dev");

// minizip/unzip return messages
const std::map<int, std::string> unzip_return_codes = {

{ UNZ_OK, 					"UNZ_OK" },
{ UNZ_END_OF_LIST_OF_FILE, 	"UNZ_END_OF_LIST_OF_FILE" },
{ UNZ_ERRNO, 				"UNZ_ERRNO" },
{ UNZ_EOF,					"UNZ_EOF" },
{ UNZ_PARAMERROR, 			"UNZ_PARAMERROR" },
{ UNZ_BADZIPFILE, 			"UNZ_BADZIPFILE" },
{ UNZ_INTERNALERROR, 		"UNZ_INTERNALERROR"},
{ UNZ_CRCERROR, 			"UNZ_CRCERROR"}

};

//translates unzip return values to std::string
const std::string &get_unzip_error(int code)
{
	const map<int, std::string>::const_iterator itr = unzip_return_codes.find(code);

	if(unzip_return_codes.end() != itr) return (itr->second);

	cerr << "Unzip: return code not found" << endl;
	return NOT_FOUND;
}

czsrv::czsrv(const std::string fn = "help.zip", int p = 19000) : zipname(fn), listen_port(p), request_str(buffer_size, '0')
{
//	zipname = fn;
//	listen_port = p;
	client_socket = 0;
	run = true; 
}

czsrv::czsrv() : zipname("help.zip"), listen_port(19000), request_str(buffer_size, '0')
{
//	zipname = fn;
//	listen_port = p;
	client_socket = 0;
	run = true; 
}

void czsrv::send_NOT_FOUND(const std::string &what)//ez jó
{
 std::stringstream content_ss;

	content_ss << "<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>" << what << " Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n";

 std::stringstream response;
	response << "HTTP/1.1 404 Not Found\nContent-Length: " << content_ss.str().length() << "\nConnection: close\nContent-Type: text/html\n\n" << content_ss.str();

	if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("Send message content");

	clog << "[NOT FOUND 404 sent] " << endl;
}

void czsrv::send_http_error(const std::string &title, const std::string &message)//title has to be a valid http response
{
 std::stringstream content_ss;

	content_ss << "<html><head>\n<title>" << title << "</title>\n</head><body>\n<h1>" << message << "\n</body></html>\n";

 std::stringstream response;
	response << "HTTP/1.1 " << title << "\nContent-Length: " << content_ss.str().length() << "\nConnection: close\nContent-Type: text/html\n\n" << content_ss.str();

	if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("Send message content");

	clog << "[error " << title << " sent] " << endl;
}

void czsrv::close_zipfile(void)
{
	if(NULL != zipfile)
	{
		unzClose(zipfile);
		zipfile = NULL;
	}
}

bool czsrv::open_zipfile(void)
{
	close_zipfile();

	zipfile = unzOpen(zipname.c_str());
	if(NULL != zipfile)
	{
		clog << "[" << zipname << " found and opened]" << endl;
//ANDROID sikerult betolteni
		return true;	
	}

//ANDROID nem sikerult betolteni
	cerr << "[unzOpen failed]" << endl;
	return false;
}

bool czsrv::send_file(void)
{
	clog << "[Sending file]" << endl;

	bool lastchunk = false;

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

		 std::string::size_type pos = URI_str.find_last_of('.');
		 std::string filetype = URI_str.substr(pos+1);

			map<string, std::string>::const_iterator itr = mimetypes.find(filetype);
//			if(string::npos != pos && std::string::npos != URI_str[pos+1] && 
			if(mimetypes.end() != itr)
			{
				filetype.assign(itr->second);
			}else{
				filetype.assign("text/html");
			}

			clog << "[mimetype] " << itr->second << endl;

			const int buffsize = 1024;
			char buffer[buffsize];
		 std::stringstream response;
		response << "HTTP/1.1 200 OK\r\nServer: " << APPNAME_str << "\r\nContent-Type: " << filetype << setbase(16) << "\r\nTransfer-Encoding: chunked\r\n\r\n" << flush;
			
			while(0 <= (ret = unzReadCurrentFile(zipfile, buffer, buffsize)))
			{
				if(0 == ret)
				{
					//last chunk
					response << ret << "\r\n\r\n" << flush;
					lastchunk = true;
				}else{
					response << ret << "\r\n";
					response.write(buffer, ret);
					response << "\r\n" << flush;
				}

//				clog << response.str() << flush;

				if(-1 != (ret = send(client_socket, response.str().data(), response.str().length(), 0)))
				{
					chunks++;
					if(lastchunk) break;
				}else{ 
					perror("[send] "); 
					lastchunk = false;
					break;
				}
				response.str(std::string()); //empty response stream
			}//while else cerr  << "[unzRead failed: " << ret << " ]" << endl;
			
			if(UNZ_OK != unzCloseCurrentFile(zipfile))
				cerr << __LINE__ << ": unzCloseCurrentFile" << get_unzip_error(ret) << endl;
		}else cerr << __LINE__ << ": unzOpenCurrentFile" << get_unzip_error(ret) << endl;
	}else cerr << __LINE__ << ": unzGetCurrentFileInfo" << get_unzip_error(ret) << endl;

	clog << "[" << chunks << " chunks sent]" << endl;

	if(lastchunk) clog << "[File sent]" << endl;

	if(0 > ret){ send_NOT_FOUND(URI_str); return false; }

	return lastchunk;
}

void czsrv::send_file_list(void)
{	
	clog << "[Sending file list]" << endl;

 std::stringstream response;
	response << "HTTP/1.1 200 OK\r\nServer: " << APPNAME_str << "\r\nContent-Type: text/html\r\n" << "Transfer-Encoding: chunked\r\n\r\n" << setbase(16) << flush;

 std::stringstream content; content << "<html><header><h1>Generated file list</h1></header><body><ol>" << endl;

	if(NULL != zipfile)
	{
		int ret = unzGoToFirstFile(zipfile);
		int f = 0;
		for(; UNZ_OK == ret; f++)
		{
			unz_file_info file_info;
			int filename_buffer_size = 1024;
			char filename_buffer[filename_buffer_size];

			ret = unzGetCurrentFileInfo(zipfile, &file_info, filename_buffer, filename_buffer_size, NULL, 0, NULL, 0);
			if(UNZ_OK == ret)
			{
				content << "<li><a href=\"" << filename_buffer << "\"/>" << filename_buffer << "</a></li>" << endl;
			}else{
				if(!f) content << "<h1>Cannot read file info</h1>" << endl;
				break;
			}

			if(400 < content.str().length()) //be a bit bigger than a row
			{
				response << content.str().length() << "\r\n" << content.str() << "\r\n";
				if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0))
					perror("send");

				response.str(std::string());
				content.str(std::string());
			}

			ret = unzGoToNextFile(zipfile);
		}//for

		if(!f)// first try is failure
		{
			content << "<h1>Cannot list zip!</h1>" << endl;
		}else{
			content << "<h3>" << f << " entries listed.</h3>" << endl;
		}
	}else{
		content << "<h1>Zip is not open!</h1>" << endl;
	}

	content << "</ol></body></html>" << endl;

	response << content.str().length() << "\r\n" << content.str() << "\r\n0\r\n\r\n";
	if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("send");

	return;
}

void czsrv::send_error(void)
{
	cerr << "send_error() called" << endl;
}

bool czsrv::parse_request(void)
{
 std::stringstream ss(request_str);

 std::string token;
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

//BAD REQUEST TODO
	return false;
}

bool czsrv::find_file(void)
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

bool czsrv::webserv(void)
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

			if(*(URI_str.crbegin()) == '/')
			{
				URI_str.append("index.html"); // root dir or DIR
				if(find_file())	send_file(); else send_file_list();

			}else if(find_file()) send_file(); else send_NOT_FOUND(URI_str);

			return true;
			clog << "[RESPONSE END]" << endl;
	//		sleep(1); //select használata előttig ez NAGYON KELLett
		}

	}else if(ret == -1)
	{
		perror("recv");
	}

	return false;
}

//mimetypes.cpp-be valo
void czsrv::list_mimetypes(void)
{
	for(map<std::string, std::string>::const_iterator  itr = mimetypes.begin(); itr != mimetypes.end(); itr++)
	{
		cout << itr->first << " " << itr->second << endl;
	}
}

void signalhandler(int signum) // ????????????????????????
{
	cerr << "[Signal caught]: " << signum << endl;

//	run = false;
}

void czsrv::run_server(void)
{
	bool ret = false;

	int listen_socket; //ezen jonnek a keresek
	struct sockaddr_in server_address;

	//szerver cimenek beallitasa
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(listen_port);
	server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//INADDR_ANY

	if(0 <= (listen_socket = socket(AF_INET, SOCK_STREAM, 0))) //domain, type, protocol(tipusonkent egy szokott lenni)
	{
		if(0 <= bind(listen_socket, (struct sockaddr *) &server_address, sizeof(server_address)))
		{
			//ANDROID itt lehetne egy sikeres inicializalas, listening uzenet
			if(0 == listen(listen_socket, 10))//a kapcsolodasokra figyel a 2. param a varolista hosszat adja meg
			{
				//add listen_socket
				fd_set	open_sockets;
				FD_ZERO(&open_sockets);
				fd_set read_fds;

				FD_SET(listen_socket, &open_sockets);
				int greatest_socket = listen_socket;
				
				clog << endl << "[zipserv started]" << endl << endl;

				while(run)
				{//loop

					FD_ZERO(&read_fds);
					read_fds = open_sockets;

//					struct timeval tv;
//					tv.tv_sec = 10; tv.tv_usec = 0;

//					int r = select(greatest_socket+1, &read_fds, NULL, NULL, &tv);
					int r = select(greatest_socket+1, &read_fds, NULL, NULL, NULL);
					if(r > 0)
					{//there are sockets to check
						int last_valid_socket = -1;
						for(int i=0; i <= greatest_socket; i++)
						{
							if(!(FD_ISSET(i, &read_fds))) continue; //this i is not in the set 
							last_valid_socket = i;

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
								if(!webserv()) //socket state changed, try to serve or close
								{
									clog << "[Closing socket] " << i << endl;
									close(i);
									FD_CLR(i, &open_sockets);

									if(i == greatest_socket) greatest_socket = last_valid_socket;
								}
							}//check socket
						}//for
					}else if(r == 0){//timeout
						clog << "[Waiting ...]" << endl;
					}else{
						perror("select()");
						run = false;
					}

				}//while

				//cleanup
				clog << "[Closing sockets]" << endl;
				FD_ZERO(&read_fds);
				for(int i=greatest_socket; 0 <= i; i--)
				{
					if(!(FD_ISSET(i, &open_sockets))) continue; //this i is not in the set 
					FD_CLR(i, &open_sockets);
					shutdown(i, 2);
					close(i);

					clog << "[Closing socket] " << i << endl;
				}
				FD_ZERO(&open_sockets);

			}else cerr << "listen(): " << strerror(errno) << endl;
		}else cerr << "bind(): " << strerror(errno) << endl;

		if(run){ shutdown(listen_socket, 2); close(listen_socket); }

	}else cerr << "socket(): " << strerror(errno) << endl;
	clog << "[Server closed]" << endl;
}

/****************************************** C specific functions *************************************************/

int main(int argc, char **argv)
{

	if(1 < argc)
	{
//	signal(SIGINT, signalhandler); //that little shit makes an "undefined reference in main" error on NDK

		cout << APPNAME_str << " Built on " __DATE__  " " __TIME__ << endl;

		czsrv server(argv[1], 19000);

		if(server.open_zipfile())
		{
			server.run_server();

			server.close_zipfile();
		}

		cout << endl << "[" << APPNAME_str << " stopped. Bye!]" << endl << endl;

	}

	return 0;
}
