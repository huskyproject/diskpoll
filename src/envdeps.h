#ifndef __ENVDEPS_H
#define __ENVDEPS_H

#define PROGRAMID "Diskpoll 0.1.3 (dev)"

// ------- OS/2 or NT with any other compiler except EMX -------
#if ((defined(__OS2__) || defined(__NT__)) && (!defined(__EMX__)))
#error "EMX GCC required under OS/2"


// ------- EMX settings for OS/2 or NT -------
#elif (defined(__EMX__))
#if (!defined(__OS2__)) && (!defined(__NT__))
#error "Only OS/2 or NT platforms supported for EMX gcc"
#else
#define FNM_FLAGS (_FNM_OS2 | _FNM_IGNORECASE)   // flags for fnmatch
#define VSNPRINTF(a,b,c,d) vsnprintf(a,b,c,d)    // emx as vsnprintf!
#define NEED_GETOPTH           // include getopt.h when using getopt
#define SHORTINCLUDEFILENAMES  // use strstrea.h, not strstream.h
#define DEFDIRSEP '\\'
#define DEFCONFIGFILE "./diskpoll.cfg"
#define ALLFILES "*"
#include <stdlib.h> /* mkdir */
#define mymkdir(a) mkdir((a), 0)
#endif


// ------- These settings are for DOS/DPMI with DJGPP v2  -------
#elif defined __DJGPP__
#define FNM_FLAGS 0
#define VSNPRINTF(a,b,c,d) vsprintf(a,c,d)
#define NEED_UNISTDH
#define DEFDIRSEP '\\'
#define DEFCONFIGFILE "./diskpoll.cfg"
#define ALLFILES "*"
#include <sys/stat.h> /* mkdir */
#define mymkdir(a) mkdir((a), 0)

// ------- DOS with any other compiler ------- 
#elif defined __DOS__
#error "The DOS version requires DJGPP as compiler!"

// ------- These settings are for Unix (FreeBSD, AIX, Linux, ...) -------
#else 
#define FNM_FLAGS FNM_CASEFOLD
#ifdef  HAVE_VSNPRINTF
#define VSNPRINTF(a,b,c,d) vsnprintf(a,b,c,d)
#else
#define VSNPRINTF(a,b,c,d) vsprintf(a,c,d)
#endif
//#define NEED_GETOPTH
//#define SHORTINCLUDEFILENAMES
#define DEFDIRSEP '/'
#define DEFCONFIGFILE "/usr/local/etc/diskpoll.cfg"
#define ALLFILES "*"
#endif
#include <sys/stat.h> /* for mkdir */
#if defined(__FreeBSD__) || defined(_AIX)
#define mymkdir(a) mkdir((a), 0)
#else
#define mymkdir(a) __mkdir((a), 0)
#endif



#endif

