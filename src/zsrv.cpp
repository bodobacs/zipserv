/*
Mivel marha nehéz olyan értelmezőt írni ami minden http kérést hatékonyan kezel ezért meghatározom mi a fenére is van szükség.
- csak GET kérésekre vagyok kíváncsi, többit dobom
- persistent kapcsolatban akarok kommunikálni a clienssel, mert ez hatékonyabb mint minden fájlhoz nyitni egy kapcsolatot
- HEADER-t sajna kell elemezgetni pl: keep alive hasznos lenne, hogy befejezze a weboldal töltését a browser
*/

//#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <cstdlib>

#include "zsrv.h"
//Visual Studio _WIN32
#ifdef _WIN32


errno_t strerror_r(int errnum, char *buffer, size_t buffer_size) //strerror() is not threadsafe, threadsafe version on windows is strerror_s 
{
	return strerror_s(buffer, buffer_size, errnum);
}

inline int close(_In_ SOCKET s) { return closesocket(s); }

WSADATA wsaData;
bool b_init_not_ready = true; //windows socket init

boolean init_winsock2(void)
{
	if(b_init_not_ready)
	{
		int r;

		r = WSAStartup(MAKEWORD(2,2), &wsaData);
		if(r != 0)
		{
			std::cerr << "Windows Socket DLL initializaiton failed. WSA error code: " << r << std::endl;
			return false;
		}
	}

	b_init_not_ready = false;
	std::clog << "wsaStartup ok" << std::endl;
	return true;
}

#else

#define INVALID_SOCKET -1

#endif

const std::string TAG("czsrv");
const std::string NOT_FOUND(" NOT_FOUND");
const std::string APPNAME_str("zserv-dev");

czsrv::czsrv() : request_str(buffer_size, '0')
{
	client_socket = 0;
	listen_port = 19000;

	barchive_ok = false; //successful open
}

czsrv::~czsrv()
{
	cleanup();
	archive.close();
}

void czsrv::send_NOT_FOUND(const std::string &what)
{
	std::stringstream content_ss;

	content_ss << "<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>" << what << " Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n";

 std::stringstream response;
	response << "HTTP/1.1 404 Not Found\nContent-Length: " << content_ss.str().length() << "\nConnection: close\nContent-Type: text/html\n\n" << content_ss.str();

	if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("Send message content");

	std::clog << "[NOT FOUND 404 sent] " << std::endl;
}

void czsrv::send_feedback_site(const std::string &title, const std::string &message)//title has to be a valid http response
{
 std::stringstream content_ss;

	content_ss << "<html><head>\n<title>" << title << "</title>\n</head><body>\n<h1>" << message << "\n</body></html>\n";

	std::stringstream response;
	response << "HTTP/1.1 " << title << "\nContent-Length: " << content_ss.str().length() << "\nConnection: close\nContent-Type: text/html\n\n" << content_ss.str();

	if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("Send message content");

	std::clog << "[error " << title << " sent] " << std::endl;
}

bool czsrv::send_file(void)
{
//	std::clog << "[Sending file]" << std::endl;

	bool lastchunk = false;
	int chunks = 0;
	
	std::string::size_type pos = URI_str.find_last_of('.');
	std::string filetype = URI_str.substr(pos+1);

	std::map<std::string, std::string>::const_iterator itr = mimetypes.find(filetype);

	if(mimetypes.end() != itr)
	{
		filetype.assign(itr->second);
	}else{
		filetype.assign("text/html");
	}

//	std::clog << "[mimetype] " << itr->second << std::endl;

	const int buffsize = 1024;
	char buffer[buffsize];
	std::stringstream response;

	response << "HTTP/1.1 200 OK\r\nServer: " << APPNAME_str << "\r\nContent-Type: " << filetype << std::setbase(16) << "\r\nTransfer-Encoding: chunked\r\n\r\n" << std::flush;
			
	unsigned int ret;
	while(0 <= (ret = archive.read(buffer, buffsize)))
	{
		if(buffsize > ret)
		{
			//last chunk
			response << ret << "\r\n";
			response.write(buffer, ret);
			response << "\r\n0\r\n\r\n" << std::flush;
			lastchunk = true;
		}else{
			response << ret << "\r\n";
			response.write(buffer, ret);
			response << "\r\n" << std::flush;
		}

//			std::clog<< response.str() << std::flush;

		if(-1 != (ret = send(client_socket, response.str().data(), response.str().length(), 0)))
		{
			chunks++;
			if(lastchunk) break;
		}else{ 
			std::cerr << "send(): " << strerror_r(errno, error_msg_buffer, error_msg_buffer_size) << std::endl; //perror("[send] "); 

			lastchunk = false;
			break;
		}
		response.str(std::string()); //empty response stream
	}//while else std::clog  << "[unzRead failed: " << ret << " ]" << std::endl;
	
//	std::clog << "[" << chunks << " chunks sent]" << std::endl;

	if(lastchunk) std::clog << "File sent: \"" << URI_str << "\"" << std::endl;

	if(0 > ret){ send_NOT_FOUND(URI_str); return false; }

	return lastchunk;
}

#include <algorithm>

void czsrv::send_file_list(void)
{	
	std::clog << "[Sending file list]" << std::endl;

	std::stringstream response;
	response << "HTTP/1.1 200 OK\r\nServer: " << APPNAME_str << "\r\nContent-Type: text/html\r\n" << "Transfer-Encoding: chunked\r\n\r\n" << std::setbase(16) << std::flush;

	std::stringstream content; content << "<html><header><h1>" << archive.get_filename() << "</h1></header><body><h2>Generated file list</h2><ol>" << std::endl;

	if(archive.list_start())
	{
		const int filename_buffer_size = 1024;
		char filename_buffer[filename_buffer_size];

		int f = 0;
		for(; archive.list_next_file(filename_buffer, filename_buffer_size);) //null terminated string in a std::string?!?!?!
		{
			//filter
			std::string fn(filename_buffer); //assume filename_buffer is always null terminated
			std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);

			if(fn.length() -5 == fn.rfind(".html")
			|| fn.length() -4 == fn.rfind(".htm"))
			{

				content << "<li><a href=\"" << filename_buffer << "\"/>" << filename_buffer << "</a></li>" << std::endl;

				if(400 < content.str().length()) //be a bit bigger than a row
				{
					response << content.str().length() << "\r\n" << content.str() << "\r\n";
					if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("send");

					response.str(std::string());
					content.str(std::string());

					f++;
				}
			}

		}//for

		if(f)// first try is failure
		{
			content << "<h3>" << f << " entries found.</h3>" << std::endl;
		}else{
			content << "<h1>Cannot list files!</h1>" << std::endl;
		}
	}else{
		content << "<h1>Archive is not open!</h1>" << std::endl;
	}

	content << "</ol></body></html>" << std::endl;

	response << content.str().length() << "\r\n" << content.str() << "\r\n0\r\n\r\n";
	if(0 > send(client_socket, response.str().c_str(), response.str().length(), 0)) perror("send");

	return;
}

void czsrv::send_error(void)
{
	std::cerr << "send_error() called" << std::endl;
}

//decode encoded URL
//http://www.w3schools.com/tags/ref_urlencode.asp
//URLs can only be sent over the Internet using the ASCII character-set.
//Since URLs often contain characters outside the ASCII set, the URL has to be converted into a valid ASCII format.
//URL encoding replaces unsafe ASCII characters with a "%" followed by two hexadecimal digits.
//URLs cannot contain spaces. URL encoding normally replaces a space with a plus (+) sign or with %20.
void czsrv::decode_request(std::string &req)
{
	if(2 < req.length())
	{
		std::stringstream ss_out;

		int i = 0;
		int end = req.length();

		while(i < end)
		{
			if(req[i] == '+') ss_out << ' '; //+ -> space
			else if(req[i] == '%' && 1 < end - i)
			{
				//ellenorizni, hogy hex code-e
				if(
				('0' <= req[i+1] && '9' >= req[i+1] ||
					'A' <= req[i+1] && 'F' >= req[i+1] ||
					'a' <= req[i+1] && 'f' >= req[i+1]
				) && (
				'0' <= req[i+2] && '9' >= req[i+2] ||
					'A' <= req[i+2] && 'F' >= req[i+2] ||
					'a' <= req[i+2] && 'f' >= req[i+2]
				))
				{
					char anyad[3] = {req[i+1], req[i+2], '\0'};
//good too, not implemented in android	ss_out << char(std::stoi(anyad, 0, 16));
					ss_out << char(strtoul(anyad, NULL, 16));

					i += 2;
				}
			}else{
				ss_out << req[i];
			}

			i++;
		}
//		std::cout << "req before: " << req << std::endl;
		req = ss_out.str();
//		std::cout << "req after: " << req << std::endl;
	}//if
}

bool czsrv::parse_request(void)
{
 std::stringstream ss(request_str);

 std::string token;
	ss >> token;
	if(!token.compare("GET"))// || !token.compare("HEADER"))
	{
//	std::clog<< "[Found] " << token << std::endl;
		ss >> URI_str;

		if(0 < URI_str.length())
		{
//			std::cout << "[URL] " << URI_str << std::endl;
			decode_request(URI_str);
			return true;
		}
	}

	std::clog << "[Not a GET request] " << token << std::endl;

//BAD REQUEST TODO
	return false;
}

bool czsrv::webserv(void)
{
//	std::clog << std::endl << "[Socket ready, serving client]" << std::endl;

//	std::clog << "[Getting request]" << std::endl;
	char buffer[buffer_size];
	int ret;

	if(0 < (ret = recv(client_socket, buffer, buffer_size, 0)))
	{
		request_str.assign(buffer, ret);
//		std::clog << std::endl << "[Request start] " << std::endl << request_str << std::endl << "[Request end]" << std::endl;

		if(parse_request())
		{
//		std::clog<< "[RESPONSE START]" << std::endl;
			if(barchive_ok){
				if(*(URI_str.crbegin()) == '/')
				{
	//				URI_str.append("index.html"); // root dir or DIR
					if(archive.find_file(URI_str + "index.html")) send_file(); else send_file_list();

				}else if(archive.find_file(URI_str)) send_file(); else send_NOT_FOUND(URI_str);
			}else send_feedback_site("404 Not Found", "Zserv reader's server is working, but no proper file selected. Choose a zip or a chm file in the application.");
//			sleep(1); //select használata előttig ez NAGYON KELLett
		}

	}else{
//		std::cerr << "[NO REQUEST]" << std::endl;

		if(ret == -1)
		{
			perror("recv");
		}
	}

	return false;
}

//mimetypes.cpp-be valo
void czsrv::list_mimetypes(void)
{
	for(std::map<std::string, std::string>::const_iterator  itr = mimetypes.begin(); itr != mimetypes.end(); itr++)
	{
		std::cout << itr->first << " " << itr->second << std::endl;
	}
}

//must use between two run_server
bool czsrv::open_archive(const std::string fn)
{
	return barchive_ok = archive.open(fn);
}

bool &czsrv::is_open_ok(void)
{
	return barchive_ok;
}

bool czsrv::init(int p)
{
//	std::cout << "cout ok" << std::endl;
//	std::clog << "clog ok" << std::endl;
//	std::cerr << "cerr ok" << std::endl;

	run = true;

//	if(barchive_ok) //opening the requested archive
	{

	#ifdef _WIN32
		init_winsock2();
	#endif

		request_str.reserve(buffer_size);
		listen_port = p;
		client_socket = 0;

		struct sockaddr_in server_address;

		//szerver cimenek beallitasa
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(listen_port);
		server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//INADDR_ANY

		if(INVALID_SOCKET != (listen_socket = socket(AF_INET, SOCK_STREAM, 0))) //domain, type, protocol(tipusonkent egy szokott lenni)
		{
#ifdef WIN32
			int optval = TRUE;
			if(setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof(optval))) perror("setsockopt");
#else
			int optval = 1;
			if(setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) perror("setsockopt"); //can reuse port right after close/crash
#endif

			if(0 == bind(listen_socket, (struct sockaddr *) &server_address, sizeof(server_address)))
			{
				//ANDROID itt lehetne egy sikeres inicializalas, listening uzenet
				if(0 == listen(listen_socket, 10))//a kapcsolodasokra figyel a 2. param a varolista hosszat adja meg
				{
					//add listen_socket
					FD_ZERO(&open_sockets);
					FD_SET(listen_socket, &open_sockets);
					
					std::clog << std::endl << "zserv is listening on port " << listen_port << std::endl << std::endl;
					return true;

				}else std::cerr << "listen(): " << strerror_r(errno, error_msg_buffer, error_msg_buffer_size) << std::endl;
			}else std::cerr << "bind(): " << strerror_r(errno, error_msg_buffer, error_msg_buffer_size) << std::endl;

			//shutdown(listen_socket, 2);
			close(listen_socket);

		}else std::cerr << "socket(): " << strerror_r(errno, error_msg_buffer, error_msg_buffer_size) << std::endl;
	}

	std::cerr << std::endl << "zserv initialization failed" << std::endl << std::endl;
	return false;	
}

//shutdown all sockets, select can end pending transfers
void czsrv::shutdown_sockets(void)
{
	std::clog << "[Shutting down connections ..]" << std::endl;

	for(int i = FD_SETSIZE; 0 <= i; i--)
	{
		if(!(FD_ISSET(i, &open_sockets))) continue; //this i is not in the set 
		shutdown(i, 1);
	}

	std::clog << "[Shutdown finished]" << std::endl;
}

//destroying sockets
void czsrv::cleanup(void)
{
	std::clog << "[Closing connections ..]" << std::endl;

	for(int i = FD_SETSIZE; 0 <= i; i--)
	{
		if(!(FD_ISSET(i, &open_sockets))) continue; //this i is not in the set 

		close(i); //listen_socket is also in this list, the first added
		FD_CLR(i, &open_sockets);

		std::clog << "[Closing socket] " << i << std::endl;
	}

	FD_ZERO(&open_sockets);

	std::clog << "[Closing finished]" << std::endl;

#ifdef WIN32
	WSACleanup();
	b_init_not_ready = true;
#endif
}

void czsrv::stop(void)
{
	shutdown(listen_socket, 2);
	run = false;	
}

bool czsrv::run_server(void)
{
	FD_ZERO(&read_fds);
	read_fds = open_sockets;

#ifdef _WIN32
//closing sockets on linux causes select() to return but on windows it does not, so on windows if you close all the connections the program hangs in select() (proof: when you try to connect in the browser the program returns!) Do not know about other systems, or versions.
	struct timeval tv; //timer
	tv.tv_sec = 5; tv.tv_usec = 0;

	int r = select(FD_SETSIZE, &read_fds, NULL, NULL, &tv);
#else
	int r = select(FD_SETSIZE, &read_fds, NULL, NULL, NULL);
#endif

//	std::clog << "Select returned " << std::endl;
	if(r > 0)
	{//there are sockets to check

#ifdef _WIN32
			for(unsigned int j = 0; j < read_fds.fd_count; j++)
			{
				socket_type i = read_fds.fd_array[j]; //win32 uses pointers as file descriptors, fd_set is a struct with a counter and a pointer array (winsock2.h)

#else //LINUX

			for(socket_type i=0; i <= FD_SETSIZE; i++) //FD_SETSIZE default = 64, not for big servers, in windows too
			{

				if(!(FD_ISSET(i, &read_fds))) continue; //this i is not in the set 
#endif

				if(i == listen_socket)
				{//new connection
//								struct sockaddr_in client_address;
//								socklen_t length;
//								int new_socket = accept(listen_socket, (struct sockaddr *)&client_address, &length);

					int new_socket = accept(listen_socket, NULL, NULL);
					if(INVALID_SOCKET != new_socket)
					{
//						std::cout << "[New connection created] " <<  new_socket << std::endl;
						FD_SET(new_socket, &open_sockets);

						//linger lin; lin.l_onoff = 1; lin.l_linger = 5; if(setsockopt(client_socket, SOL_SOCKET, SO_LINGER, (void*)&lin, sizeof(lin))) perror("setsockopt");

					}else{
						//listening socket not return a new socket, so it gone wrong
						//std::cerr << "accept(): " << strerror_r(errno, error_msg_buffer, error_msg_buffer_size) << std::endl;// perror("[ accept() ] "); 

						//std::clog << "[Closing listening socket] " << i << std::endl;
						//close_sockets();

						std::cerr << "Maybe error! Listening socket returned with error!" << i << std::endl;
						run = false;
					}
				}else{
					client_socket = i;
					if(!webserv()) //socket state changed, try to serve or close
					{
						std::clog << "[Closing socket] " << i << std::endl;
						//shutdown(i, 2);
						//no more readable data
						close(i);
						FD_CLR(i, &open_sockets);
					}
				}//check socket
			}//for
		}else if(r == 0)
		{//timeout
			std::clog << "[Waiting ...]" << std::endl;
		}else{//r < 0 error
			std::cerr << "select(): " << strerror_r(errno, error_msg_buffer, error_msg_buffer_size) << std::endl; //perror("select()");
			run = false;
		}

	return run;
}

