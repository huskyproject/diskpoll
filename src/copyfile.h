#ifndef __COPYFILE_H
#define __COPYFILE_H

#define COPY_NOERR     0
#define COPY_NOTEXIST -1
#define COPY_OTHER    -2

extern int copyfile_no_api; /* prevent usage of OS copy API */
int copyfile(const char *cpDest, const char *cpSource);
#define BUFLEN 65536

#endif

     
