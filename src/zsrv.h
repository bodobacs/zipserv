#ifndef ZIPSRV_H
#define ZIPSRV_H

#include <cstring>
#include "mimetypes.h"
#include "../libarchives/archives.h"

//Visual Studio _WIN32
#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET socket_type;
#else

//Berkeley
#include <arpa/inet.h> //htons stb
#include <sys/types.h> //socket, bind
#include <sys/socket.h>
#include <unistd.h> //select()

typedef int socket_type;

#endif


class czsrv
{

protected:
	carchive archive;

	static const int error_msg_buffer_size = 100;
	char error_msg_buffer[error_msg_buffer_size];
	std::string archive_name;
	int listen_port;

	std::string request_str;
	std::string URI_str;
	static const int buffer_size = 2048;//ennek elégnek kéne lennie, firefoxnak van 8kb

	socket_type client_socket;
	bool run; 

	void send_NOT_FOUND(const std::string &what);
	void send_http_error(const std::string &title, const std::string &message);//title has to be a valid http response
	bool send_file(void);
	void send_file_list(void);
	void send_error(void);
	bool parse_request(void);
	bool find_file(void);
	bool webserv(void);
	void list_mimetypes(void);

	static bool mstaticStopAll; //ugly but no better yet, if true, all instances stop running
public:

	void init(const std::string fn, int p);

	czsrv(const std::string fn, int p);
	czsrv();
	~czsrv();

	void run_server(void);
	void stop(void);

	bool open(void);

	static void stopALL(void){ mstaticStopAll = true; }
private:
};

#endif
