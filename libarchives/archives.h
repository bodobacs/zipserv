#ifndef ARCHIVES_H
#define ARCHIVES_H

#include <string>

//with this I do not have to expose filetype specific code
class carchive_base
{
public:
	carchive_base(){};

	virtual bool open(const std::string &filename){return false;}
	virtual void close(void){}

	virtual bool find_file(const std::string &filename){return false;}

	virtual unsigned int read(char *buffer, const int &buffer_size){return 0;}

	virtual bool list_start(void){return false;}
	virtual bool list_next_file(char *buffer, const int &buffer_size){return false;}

};

//This interface class is for use outside
class carchive
{
	bool checkfiletype(const std::string &filename, const std::string &type);

	std::string archive_filename;
public:
	carchive();
	~carchive();

	bool is_open(void);
	bool open(const std::string &filename);
	void close(void);

	bool find_file(const std::string &filename);
	unsigned int read(char *buffer, const int &buffer_size);
	bool list_start(void);
	bool list_next_file(char *buffer, const int &buffer_size);

	std::string get_filename(){ return archive_filename; };
private:
	carchive_base *parchive;
};

#endif

