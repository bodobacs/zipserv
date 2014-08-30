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

#include "../zlib/contrib/minizip/unzip.h"

using namespace std;

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

	stringstream answer_ss;
	answer_ss << "HTTP/1.1 404 Not Found\nContent-Length: " << content_ss.str().length() << "\nConnection: close\nContent-Type: text/html\n\n" << content_ss.str();

	if(0 > send(client_socket, answer_ss.str().c_str(), answer_ss.str().length(), 0)) perror("Send message content");

	cout << "[Response]" << endl << answer_ss.str() << endl << "[ResponseEnd]" << endl;
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
	bool success = false;
	clog << "[Sending file]" << endl;

	unz_file_info file_info;
	int filename_buffer_size = 1024;
	char filename_buffer[filename_buffer_size];

	int ret = unzGetCurrentFileInfo(zipfile, &file_info, filename_buffer, filename_buffer_size, NULL, 0, NULL, 0);
	if(UNZ_OK == ret)
	{
		char *pbuffer = new char[file_info.uncompressed_size+1];

		if(NULL != pbuffer)
		{
			if(UNZ_OK == unzOpenCurrentFile(zipfile))
			{
				clog << "[Uncompressed size] " << file_info.uncompressed_size << endl;
				if(0 <= (ret = unzReadCurrentFile(zipfile, pbuffer, file_info.uncompressed_size)))
				{
					stringstream response;
					string::size_type pos = URI_str.find_last_of('.');
					string filetype;

					if(string::npos != pos && string::npos != URI_str[pos+1])
					{
						filetype = URI_str.substr(pos+1);
					}else{
						filetype.assign("text/html");
					}

					response << "HTTP/1.1 200 OK\nServer: " << appname << "\nContent-Length: " << file_info.uncompressed_size << "\nConnection: close\nContent-Type: " << "text/html" << "\n\n";
					response.write(pbuffer, file_info.uncompressed_size);


					clog << response.str() << endl;

				//	*(pbuffer+file_info.uncompressed_size) = 0;
				//	cout << pbuffer << endl;

					if(0 < (ret = send(client_socket, response.str().c_str(), response.str().length(), 0)))
					{
						clog << "[Response sent]" << endl;
						success = true;
					}else{ cerr << "sending to client failed" << endl; perror("send"); }
					clog << "[" << ret << " bytes]" << endl;
				}else cerr << "[unzRead failed: " << ret << " ]" << endl;
				
				unzCloseCurrentFile(zipfile);
			}else cerr << "[unzOpen failed]" << endl;

			delete[] pbuffer;
		}else cerr << "[Memory allocation failed]" << endl;
	}else cerr << "[Reading zip's file info failed]" << endl;

	if(!success) send_NOT_FOUND(URI_str);

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
		cout << "[Found] " << token << endl;
		ss >> URI_str;
		if(0 < URI_str.length())
		{
			cout << "[URL] " << URI_str << endl;
			return true;
		}
	}

	cout << "[Not a GET request] " << token << endl;
	return false;
}

bool get_request(void)
{
	char buffer[buffer_size];
	int ret;
	if(0 < (ret = recv(client_socket, buffer, buffer_size, 0)))
	{
		request_str.assign(buffer, ret);
		clog << "[Request start] " << endl << request_str << endl << "[Request end]" << endl;

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
		clog << "[Searching zip for] " << URI_str.c_str()+1 << endl;
		if( UNZ_OK == unzLocateFile(zipfile, URI_str.c_str()+1, 2))
		{
			return true;
		}
	}

	clog << "[Not found] " << URI_str << endl;
	return false;
}

void webserv(void)
{
	clog << endl << "[Socket ready, serving client]" << endl;

//	read_all_request();
	open_zipfile();

	while(get_request() && parse_request())
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
		clog << "[RESPONSE END]" << endl;
	}
	close_zipfile();

	sleep(1); //EZ NAGYON KELL, ki kell találni valami jobbat helyette
	close(client_socket);

	clog << "[Socket closed]" << endl;
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
