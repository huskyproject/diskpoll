#include <stdarg.h>
#include <stdio.h>              // sprintf
#include "envdeps.h"
#include "log.h"
#include <iostream.h>
#include <time.h>

void logmsg(char type, char *cpformat, ...)
{
  static char buffer[2048];
  char prefix[3];
  time_t t=time(&t);
  struct tm *tm=localtime(&t);
  char cpTime[64];
  static char *cpMonths[]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                           "Aug", "Sep", "Oct", "Nov", "Dec", "???"};

  sprintf(cpTime,"%02i %3s %02i:%02i:%02i DISK",
          tm->tm_mday, cpMonths[tm->tm_mon], tm->tm_hour, tm->tm_min,
          tm->tm_sec);

  va_list argptr;
  va_start(argptr, cpformat);

  if (cpformat==NULL)
    buffer[0]=prefix[0]=cpTime[0]='\0';
  else
    {
      VSNPRINTF(buffer, 2047, cpformat, argptr);
      prefix[0]=type; prefix[1]=' '; prefix[2]='\0';
      buffer[2047]='\0';
    }

  if (type==LOGERR)
    cerr << prefix << cpTime << " " << buffer << "\n";
  else
    cout << prefix << cpTime << " " << buffer << "\n";

  va_end(argptr);
}



