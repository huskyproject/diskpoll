#ifndef __COPYFILE_H
#define __COPYFILE_H

#define COPY_NOERR     0
#define COPY_NOTEXIST -1
#define COPY_OTHER    -2

int copyfile(const char *cpDest, const char *cpSource);
#define BUFLEN 32768

#endif

     
