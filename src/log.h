#ifndef __LOG_H
#define __LOG_H

#define LOGDBG '?'
#define LOGMSG '+'
#define LOGERR '!'

extern int debug_mode;

void logmsg(char type, char *cpformat, ...); /* sprintf syntax */

#endif
