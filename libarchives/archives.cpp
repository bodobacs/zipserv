#include <string>
#include <iostream>
#include <map>
#include <cstring>
#include <cctype>
#include "./minizip/unzip.h"
#include "./modchmlib/chm_lib.h"
#include "archives.h"

class carchive_zip : public carchive_base
{

	unzFile zipfile;

public:

// minizip/unzip return messages 
#if defined(_WIN32)
	const std::map<int, std::string> unzip_return_codes = std::map<int, std::string>{ //error C2797: list initialization inside member initializer list or non-static data member initializer is not implemented
#else
// minizip/unzip return messages (this is the standard)
	const std::map<int, std::string> unzip_return_codes{
#endif

		{ UNZ_OK, 			"UNZ_OK" },
		{ UNZ_END_OF_LIST_OF_FILE, 	"UNZ_END_OF_LIST_OF_FILE" },
		{ UNZ_ERRNO, 			"UNZ_ERRNO" },
		{ UNZ_EOF,			"UNZ_EOF" },
		{ UNZ_PARAMERROR, 		"UNZ_PARAMERROR" },
		{ UNZ_BADZIPFILE, 		"UNZ_BADZIPFILE" },
		{ UNZ_INTERNALERROR, 		"UNZ_INTERNALERROR"},
		{ UNZ_CRCERROR, 		"UNZ_CRCERROR"}
	};

	const std::string NOT_FOUND = "NOT_FOUND";
	const std::string TAG = "archive_zip: ";

	carchive_zip()
	{
		zipfile = NULL;
	}

	~carchive_zip()
	{
		close();
	}

	const std::string &get_unzip_error(int code)
	{
		const std::map<int, std::string>::const_iterator itr = unzip_return_codes.find(code);

		if(unzip_return_codes.end() != itr) return (itr->second);

		std::cerr << "Unzip: return code not found" << std::endl;
		return NOT_FOUND;
	}

	bool file_is_open = false;

	bool open_file_in_zip(void)
	{
//		std::clog << __LINE__ << ": file_is_open" << std::endl;

		unz_file_info file_info;
		const int filename_buffer_size = 1024;
		char filename_buffer[filename_buffer_size];

		int ret = unzGetCurrentFileInfo(zipfile, &file_info, filename_buffer, filename_buffer_size, NULL, 0, NULL, 0);
		if(UNZ_OK == ret)
		{
			if(UNZ_OK == (ret = unzOpenCurrentFile(zipfile)))
			{
				file_is_open = true;
				return true;
			}else std::cerr << __LINE__ << ": unzOpenCurrentFile" << get_unzip_error(ret) << std::endl;
		}else std::cerr << __LINE__ << ": unzGetCurrentFileInfo" << get_unzip_error(ret) << std::endl;
		return false;
	}

	unsigned int read(char *buffer, const int &buffer_size)
	{
		if(file_is_open || open_file_in_zip())
		{
			int ret = unzReadCurrentFile(zipfile, buffer, buffer_size);
			if(buffer_size > ret)
			{
				if(0 > ret)
				{
					std::cerr << __LINE__ << ": unzReadCurrentFile" << get_unzip_error(ret) << std::endl;
					ret = 0;
				}

				if(UNZ_OK != unzCloseCurrentFile(zipfile))
					std::cerr << __LINE__ << ": unzCloseCurrentFile" << get_unzip_error(ret) << std::endl;

				file_is_open = false;
			}
			return ret;
		}
		return 0;
	}

	bool find_file(const std::string &fn)
	{
		if(NULL != zipfile && 1 < fn.length())
		{
//			std::clog << TAG << " find_file(\"" << fn << "\") ";

			unzGoToFirstFile(zipfile);
			if( UNZ_OK == unzLocateFile(zipfile, fn.c_str()+1, 2))
			{
//				std::clog << "found" << std::endl;
				return true;
			}
		}

//		std::clog << "not found" << std::endl;
		return false;
	}


	bool list_start(void)
	{
		if(NULL != zipfile && UNZ_OK == unzGoToFirstFile(zipfile)) return true;

		std::cerr << TAG << "list_start() failed" << std::endl;
		return false;
	}

	bool list_next_file(char *buffer, const int &buffer_size)
	{
		if(NULL != buffer)
		{
			unz_file_info file_info;
			
			if(UNZ_OK == unzGetCurrentFileInfo(zipfile, &file_info, buffer, buffer_size, NULL, 0, NULL, 0) 
			&& UNZ_OK == unzGoToNextFile(zipfile)) return true;
		}
		std::cerr << TAG << "no next file" << std::endl;
		return false;
	}

	bool open(const std::string &filename)
	{
//		std::cout << "carchive_zip tries to open: " << filename << std::endl;

		close();

		zipfile = unzOpen(filename.c_str());
		if(NULL != zipfile)
		{
//			std::clog << "[" << filename << " found and opened]" << std::endl;
			return true;	
		}

		std::cerr << "Failed to open: " << filename << std::endl;
		return false;
	}

	void close(void)
	{
		if(NULL != zipfile)
		{
			unzClose(zipfile);
			zipfile = NULL;
		}
	}
};//class carchive_zip : public carchive_base


class carchive_chm : public carchive_base
{
	struct chmFile *chmfile;
	struct chm_list_rundata rundata;

public:
	const std::string TAG = "archive_chm: ";

	carchive_chm()
	{
		chmfile = NULL;
	}

	~carchive_chm()
	{
		close();
//		std::cout << "chm destructor" << std::endl;
	}
	
	unsigned int offset = 0;
	unsigned int read(char *buffer, const int &buffer_size)
	{
		unsigned int len = unsigned int(chm_retrieve_object(chmfile, &rundata.ui, (unsigned char *)buffer, offset, buffer_size));
		offset += len;
		return len;
	}

	bool find_file(const std::string &fn)
	{
//		std::cout << TAG << " find_file(\"" << fn << "\") ";

		offset = 0; //kereseés után már megint a file elejétől kezdünk olvasni

		if(CHM_RESOLVE_SUCCESS == chm_resolve_object(chmfile, fn.c_str(), &rundata.ui))
		{
//			std::clog << "found" << std::endl;
			return true;
		}
		
//		std::cout << "not found" << std::endl;
		return false;
	}

	bool open(const std::string &filename)
	{
//		std::cout << "chm open " << filename << std::endl;
		chm_close(chmfile);

		chmfile = chm_open(filename.c_str());

		if(NULL != chmfile) return true;
		std::cerr << "Failed to open chm:" << filename << std::endl;
		return false;
	}

	void close(void)
	{
		if(NULL != chmfile)
		{
			chm_close(chmfile);
			chmfile = NULL;
		}
	}

	bool list_start(void)
	{
		return chm_list_start(chmfile, &rundata, CHM_ENUMERATE_NORMAL);
	}

	bool list_next_file(char *buffer, const int &buffer_size)
	{
		if(chm_list_next(chmfile, &rundata))
		{
			strncpy(buffer, rundata.ui.path, buffer_size); //rundata->ui->ui_path_len
			return true;	
		}

		return false;
	}

};// class carchive_chm : public carchive_base

carchive::carchive()
{
	parchive = NULL;
}

carchive::~carchive()
{
//	std::cout << "carchive destructor" << std::endl;
	if(NULL != parchive)
	{
//		delete parchive;
		parchive = NULL;
	}
}

// ************************************************* carchive *******************************************************
#include <algorithm>

bool icompare_pred(unsigned char a, unsigned char b)
{
    return std::tolower(a) == std::tolower(b);
}

bool carchive::checkfiletype(const std::string &filename, const std::string &type)
{
	if(3 < filename.length() && std::equal(filename.end()-3, filename.end(), type.begin(), icompare_pred))
		return true;
	return false;
}

carchive_chm a_chm; 
carchive_zip a_zip;

bool carchive::open(const std::string &filename)
{
	std::cout << filename << " looks like ";
	
	parchive = NULL;
	if(checkfiletype(filename, "chm"))
	{
//		parchive = new carchive_chm();
		parchive = &a_chm;
		std::cout << "a chm. " << std::endl;

	}else{
		if(checkfiletype(filename, "zip"))
		{
//			parchive = new carchive_zip();
			parchive = &a_zip;
			std::cout << "a zip " << std::endl;
		}else{
			std::cout << "an unknown filetype. " << std::endl;
		}
	}

	if(NULL != parchive)
	{
		return parchive->open(filename);
	}

//	std::cout << "Failed to create carchive instance" << std::endl;
	return false;
}

void carchive::close(void)
{
	if(NULL != parchive)
	{
		parchive->close();
		parchive = NULL;
	}
}

bool carchive::find_file(const std::string &filename)
{
//	if(NULL != parchive) return parchive->find_file(filename);
	return parchive->find_file(filename);
//	return false;
}

unsigned int carchive::read(char *buffer, const int &buffer_size)
{
//	if(NULL != parchive) return parchive->read(buffer, buffer_size);
	return parchive->read(buffer, buffer_size);
//	return 0;
}

bool carchive::list_start(void)
{
//	if(NULL != parchive) return parchive->list_start();
	return parchive->list_start();
//	return false;
}

bool carchive::list_next_file(char *buffer, const int &buffer_size)
{
//	if(NULL != parchive) return parchive->list_next_file(buffer, buffer_size);
	return parchive->list_next_file(buffer, buffer_size);
//	return false;
}

/*
bool carchive::isok(void)
{
	if(NULL != parchive) return true;
	return false;
}
*/
