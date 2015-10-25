#!/bin/sh

LIBMODCHM=../libarchives/modchmlib/

7z a -tzip ./website/modchmlib.zip $LIBMODCHM/chm_lib.c $LIBMODCHM/chm_lib.h $LIBMODCHM/lzx.c $LIBMODCHM/lzx.h 

