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
	socket_type listen_socket; //ezen jonnek a keresek
	fd_set	open_sockets;// all open sockets
	fd_set read_fds; //ezt állitja be a select

protected:
	bool barchive_ok;
	bool barchive_used;

	static const int error_msg_buffer_size = 100;
	char error_msg_buffer[error_msg_buffer_size];
	std::string archive_name;
	int listen_port;

	std::string request_str;
	std::string URI_str;
	static const int buffer_size = 2048;//ennek elégnek kéne lennie, firefoxnak van 8kb

	socket_type client_socket;

	void send_NOT_FOUND(const std::string &what);
	void send_feedback_site(const std::string &title, const std::string &message);//title has to be a valid http response
	bool send_file(void);
	void send_file_list(void);
	void send_error(void);
	bool parse_request(void);
	void decode_request(std::string &req);
	bool webserv(void);
	void list_mimetypes(void);

	void shutdown_sockets(void);

	bool run;
public:


	czsrv();
	~czsrv();

	carchive archive;
	bool open_archive(const std::string fn);
	bool init(int p);
	bool run_server(void);

	void stop(void);
	void cleanup(void);

	int get_port(void){ return listen_port; }
private:
};

#endif
