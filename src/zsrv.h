#ifndef ZIPSRV_H
#define ZIPSRV_H

#include <cstring>
#include "mimetypes.h"
#include "minizip/unzip.h"

class czsrv
{

//protected:
public:
	std::string zipname;
	int listen_port;

	std::string request_str;
	std::string URI_str;
	const int buffer_size = 2048;//ennek elégnek kéne lennie, firefoxnak van 8kb

	int client_socket;
	unzFile zipfile = NULL;
	bool run; 

	void send_NOT_FOUND(const std::string &what);
	void send_http_error(const std::string &title, const std::string &message);//title has to be a valid http response
	void close_zipfile(void);
	bool open_zipfile(void);
	bool send_file(void);
	void send_file_list(void);
	void send_error(void);
	bool parse_request(void);
	bool find_file(void);
	bool webserv(void);
	void list_mimetypes(void);

public:
	czsrv(const std::string fn, int p);
	czsrv();

	void run_server(void);
	void stop(void);


private:
};

#endif
