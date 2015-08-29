#include "chm_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * callback function for enumerate API
 */
int _print_ui(struct chmFile *h, struct chmUnitInfo *ui, void *context)
{
	static char szBuf[128];
	memset(szBuf, 0, 128);
	if(ui->flags & CHM_ENUMERATE_NORMAL) strcpy(szBuf, "normal ");
	else if(ui->flags & CHM_ENUMERATE_SPECIAL) strcpy(szBuf, "special ");
	else if(ui->flags & CHM_ENUMERATE_META) strcpy(szBuf, "meta ");
	
	if(ui->flags & CHM_ENUMERATE_DIRS) strcat(szBuf, "dir");
	else if(ui->flags & CHM_ENUMERATE_FILES) strcat(szBuf, "file");

    printf("   %1d %8d %8d   %s\t\t%s\n", (int)ui->space, (int)ui->start, (int)ui->length, szBuf, ui->path);
    return CHM_ENUMERATOR_CONTINUE;
}

int main(int c, char **v)
{
    struct chmFile *h;
    int i;

	h = chm_open(v[i]);
	if (h =! NULL)
	{

		if (! chm_enumerate(h, CHM_ENUMERATE_ALL, _print_ui, NULL)) printf("   *** ERROR ***\n");

		chm_close(h);

	}else{
		fprintf(stderr, "failed to open %s\n", v[i]);
	}


    return 0;
}
