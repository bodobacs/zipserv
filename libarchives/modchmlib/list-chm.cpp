/*
MODIFIED extract_chm.c
*/

#include "chm_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(X, Y) _mkdir(X)
#define snprintf _snprintf
#else

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

const unsigned int MAX_PATH =  CHM_MAX_PATHLEN;

struct s_context
{
    const char *ppath;
	unsigned int len;
};

using namespace std;

int callback_extract(struct chmFile *pfile, struct chmUnitInfo *punitinfo, void *pcontext)
{
	struct s_context *pc = (struct s_context *)pcontext;

	if(0 == strncmp(pc->ppath, (const char *)&punitinfo->path, pc->len))
	{

		cout << "Found " << pc->ppath << "  " << punitinfo->path << endl;
		
		unsigned int len;
		unsigned int offset = 0;
		unsigned int remain = punitinfo->length;
		char buffer[32768];
		
	    while (remain != 0)
		{
			len = chm_retrieve_object(pfile, punitinfo, (unsigned char *)buffer, offset, 32768);
			if (len > 0)
			{
				fwrite(buffer, 1, (size_t)len, stdout);
				offset += len;
				remain -= len;
			}
			else
			{
				fprintf(stderr, "incomplete file: %s\n", punitinfo->path);
				break;
			}
		}

	}

    return CHM_ENUMERATOR_CONTINUE;
}

int callback_list(struct chmFile *pfile, struct chmUnitInfo *punitinfo, void *pcontext)
{
//just list files
	cout << punitinfo->path << endl;
	 return CHM_ENUMERATOR_CONTINUE;
}

int main(int c, char **v)
{
/*
    struct chmFile *h;
    struct s_context context; context.ppath = "/Ch99.htm"; context.len = strlen(context.ppath);

    if (c < 3)
    {
        fprintf(stderr, "usage: %s <chmfile> <outdir>\n", v[0]);
        exit(1);
    }

    h = chm_open(v[1]);
    if (h == NULL)
    {
        fprintf(stderr, "failed to open %s\n", v[1]);
        exit(1);
    }




	cout << "********* Listing chm content:" << endl;


    if (! chm_enumerate(h, CHM_ENUMERATE_NORMAL, callback_list, NULL)) cout << "chm_enumerate error" << endl;

	
	
	cout << "********* Finding chm content:" << endl;
    
	
	if (! chm_enumerate(h, CHM_ENUMERATE_NORMAL, callback_extract, (void *)&context)) cout << "chm_enumerate error" << endl;



/ *    printf("%s:\n", v[1]);
    ec.base_path = v[2];
    if (! chm_enumerate(h, CHM_ENUMERATE_ALL, _extract_callback, (void *)&ec)) printf("   *** ERROR ***\n");
* /
*/


    struct chmFile *h;
	struct chm_list_rundata rundata;

    h = chm_open(v[1]);
    if (h == NULL)
    {
        fprintf(stderr, "failed to open %s\n", v[1]);
        exit(1);
    }

	if(chm_list_start(h, &rundata, CHM_ENUMERATE_NORMAL))
	{
		while(chm_list_next(h, &rundata))
		{
			cout << rundata.ui.path << endl;
		}
	}

    chm_close(h);

    return 0;
}

