#ifndef __ENVDEPS_H
#define __ENVDEPS_H

#define PROGRAMID "Diskpoll 0.1.4 (dev)"

// ------- OS/2 or NT with any other compiler except EMX -------
#if ((defined(__OS2__) || defined(__NT__) || defined(OS2) || defined(NT) || defined(WINNT)) && (!defined(__EMX__)))
#error "EMX GCC required under OS/2, RSX required unter NT"


// ------- EMX settings for OS/2 or NT -------
#elif (defined(__EMX__))
#if (!defined(__OS2__)) && (!defined(__NT__)) && (!defined(OS2)) && (!defined(NT)) && (!defined(WINNT))
#error "Only OS/2 or NT platforms supported for EMX gcc"
#else
#define VSNPRINTF(a,b,c,d) vsnprintf(a,b,c,d)    // emx as vsnprintf!

// #define NEED_GETOPTH
// pgcc getopt.h has getopt prototype with empty args list - this won't
// work for C++, so we need our own prototypes
extern "C" {
int getopt (int argc, char *const *argv, const char *shortopts);
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;
}

//#define SHORTINCLUDEFILENAMES  // use strstrea.h, not strstream.h
#define DEFDIRSEP '\\'
#ifndef CFGDIR
#define CFGDIR "."
#endif
#define ALLFILES "*"
#include <stdlib.h> /* mkdir */
#define mymkdir(a) mkdir((a), 0)
#endif


// ------- These settings are for DOS/DPMI with DJGPP v2  -------
#elif defined __DJGPP__
#define VSNPRINTF(a,b,c,d) vsprintf(a,c,d)
#define NEED_UNISTDH
#define DEFDIRSEP '\\'
#ifndef CFGDIR
#define CFGDIR "."
#endif
#define ALLFILES "*"
#include <sys/stat.h> /* mkdir */
#define mymkdir(a) mkdir((a), 0)

// ------- DOS with any other compiler -------
#elif defined(__DOS__) || defined(DOS)
#error "The DOS version requires DJGPP as compiler!"

// ------- These settings are for Unix (FreeBSD, AIX, Linux, ...) -------
#else
#define UNIXLIKE
#ifdef  HAVE_VSNPRINTF
#define VSNPRINTF(a,b,c,d) vsnprintf(a,b,c,d)
#else
#define VSNPRINTF(a,b,c,d) vsprintf(a,c,d)
#endif
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__)
#define NEED_GETOPTH
#endif
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#define NEED_UNISTDH
#endif
//#define SHORTINCLUDEFILENAMES
#define DEFDIRSEP '/'
#ifndef CFGDIR
#define CFGDIR "/usr/local/etc"
#endif
#define ALLFILES "*"
#include <sys/stat.h> /* for mkdir */
#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(_AIX) || defined(__osf__) || defined(__GLIBC__)
#define mymkdir(a) mkdir((a), 0777)
#else
#define mymkdir(a) __mkdir((a), 0777)
#endif
#endif

#define DEFCONFIGFILE CFGDIR"/diskpoll.cfg"

#endif

