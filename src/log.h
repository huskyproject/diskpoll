#ifndef __LOG_H
#define __LOG_H

#define LOGDEB '?'
#define LOGMSG '+'
#define LOGERR '!'

void logmsg(char type, char *cpformat, ...); /* sprintf syntax */

#endif
