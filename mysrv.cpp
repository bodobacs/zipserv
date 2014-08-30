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

#include "../zlib/contrib/minizip/unzip.h"

using namespace std;

const string C_err  ("\x1B[31m");
const string C_ok   ("\x1B[32m");
const string C_warn ("\x1B[33m");
const string C_reset("\033[0m" );

const string appname("zipserv-dev");

//atmeneti valtozok

char zipname[] = "../abs-guide.zip";

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

	zipfile = unzOpen(zipname);
	if(NULL != zipfile)
	{
		return true;	
	}else{
		cerr << C_err << "[unzOpen failed]" << C_reset << endl;
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
		if(UNZ_OK == unzOpenCurrentFile(zipfile))
		{
			clog << "[Uncompressed size] " << file_info.uncompressed_size << endl;

			string::size_type pos = URI_str.find_last_of('.');
			string filetype;

			if(0)//string::npos != pos && string::npos != URI_str[pos+1])
			{
				filetype = URI_str.substr(pos+1);
			}else{
				filetype.assign("text/html");
			}

			const int buffsize = 256;
			char buffer[buffsize];
			stringstream response;
			response << "HTTP/1.1 200 OK\nServer: " << appname << "\nContent-Type: " << filetype << "\nTransfer-Encoding: chunked\n\n" << setbase(16);

			while(0 <= (ret = unzReadCurrentFile(zipfile, buffer, buffsize)))
			{
				response << ret << "\n";
				response.write(buffer, ret);
				response << "\n";

//				clog << response.str() << endl;

				if(0 < send(client_socket, response.str().data(), response.str().length(), 0))
				{
					chunks++;
					success = true;
					if(ret == 0){ clog << C_ok << "[Last chunk sent]" << C_reset << endl;  break; }
				}else{ 
					cerr << C_err << "[Sending response to client failed] " << C_reset << endl; 
					perror("send"); 
					success = false;
					break;
				}
				response.str(string()); //empty response stream
			}//while else cerr << C_err  << "[unzRead failed: " << ret << " ]" << C_reset << endl;
			
			unzCloseCurrentFile(zipfile);
		}else cerr << C_err << "[unzOpen failed]" << C_reset << endl;
	}else cerr << C_err << "[Reading zip's file info failed]" << C_reset << endl;

	if(success)
	{
		clog << C_ok << "[" << chunks << " chunks]" << C_reset << endl;
	}else send_NOT_FOUND(URI_str);

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

	response << "HTTP/1.1 200 OK\nServer: " << appname << "\nContent-Length: " << content.str().length() << "\nConnection: keep-alive\nContent-Type: html\n\n" << content.str() << flush;

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
	if(!token.compare("GET") || !token.compare("HEADER"))
	{
		clog << C_ok << "[Found] " << token << C_reset << endl;
		ss >> URI_str;
		if(0 < URI_str.length())
		{
			cout << "[URL] " << URI_str << endl;
			return true;
		}
	}

	clog << C_warn << "[Not a GET request] " << token << C_reset << endl;
	return false;
}

bool get_request(void)
{
	char buffer[buffer_size];
	int ret;
	if(0 < (ret = recv(client_socket, buffer, buffer_size, 0)))
	{
		request_str.assign(buffer, ret);
		clog << C_warn << "[Request start] " << endl << request_str << endl << "[Request end]" << C_reset << endl;

		return true;

	}else if(ret < 0)
	{
		perror("recv");
	}
	
	return false;
}

bool find_file(void)
{
	if(NULL != zipfile)
	{
		clog << C_ok << "[Searching zip for] " << URI_str.c_str()+1 << C_reset << endl;
		if( UNZ_OK == unzLocateFile(zipfile, URI_str.c_str()+1, 2))
		{
			return true;
		}
	}

	clog << C_warn << "[Not found] " << URI_str << C_reset << endl;
	return false;
}

void webserv(void)
{
	clog << endl << C_ok << "[Socket ready, serving client]" << C_reset << endl;

//	read_all_request();
	open_zipfile();

	while(get_request() && parse_request())
	{
		clog << C_warn << "[RESPONSE START]" << C_reset << endl;
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
		clog << C_warn << "[RESPONSE END]" << C_reset << endl;
	}
	close_zipfile();

	sleep(1); //EZ NAGYON KELL, ki kell találni valami jobbat helyette
	close(client_socket);

	clog << C_warn << "[Socket closed]" << C_reset << endl;
}



int main(int argc, char **argv)
{
	clog << C_ok << "[zipserv started]" << C_reset << endl;
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
						cout << endl << "[New client connected] " << ntohl(client_address.sin_port) << endl;

						linger lin; lin.l_onoff = 1; lin.l_linger = 5;
					/*	if(setsockopt(client_socket, SOL_SOCKET, SO_LINGER, (void*)&lin, sizeof(lin)))
						{
							perror("setsockopt");
						}
*/
						webserv();
					}else{
						perror("[ accept() ] ");
					}
				}//while
			}else{
				perror("[ listen() ]");
			}
		}else{
			perror("[ bind() ]");
		}
	}

	close(listen_socket);

	clog << endl << "[Server stopped]" << endl;

return 0;
}
