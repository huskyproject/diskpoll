#ifndef __CERROR_HPP
#define __CERROR_HPP


/* Error Class to be thrown on Classlib errors */

class CError
{ public: char *cpType;
          char *cpMessage;
          char *cpFile;
          long nLine;

  CError(char *type, char *message, char *file, long line)
  { cpType=type; cpMessage=message; cpFile=file; nLine=line; }

  char *Type() { return cpType; }
  char *Message() { return cpMessage; }
  char *File() { return cpFile; }
  long Line() { return nLine; }
};

#ifdef NOCHECK
#define NOCHECKPOINTERS
#define NOCHECKBOUNDS
#define NOCHECKHANDLES
#define NOCHECKCONDITIONS
#endif

#ifdef NOEXCEPTIONS
#include <stdlib.h>
#ifdef NOCHECKPOINTERS
  #define CheckPointer(ptr,msg) (ptr)
#else
  #define  CheckPointer(ptr,msg) \
           if( (ptr) == 0 )      \
             abort();            \
           else;
#endif
#ifdef NOCHECKBOUNDS
  #define CheckBounds(i,min,max,msg) (i)
  #define CheckUpperBound(i,max,msg) (i)
#else
  #define  CheckBounds(i,min,max,msg)   \
           if( (i)<(min) || (i)>(max))          \
              abort(); \
              else;
  #define  CheckUpperBound(i,max,msg)   \
           if((i)>(max))          \
              abort(); \
              else;
#endif

#ifdef NOCHECKHANDLES
  #define CheckHandle(h,msg) (h)
#else
  #define CheckHandle(h,msg)                                       \
          if ((h)==0) abort(); \
                      else;
#endif

#ifdef NOCHECKCONDITIONS
  #define CheckCond(h,msg)      (h)
#else
  #define CheckCond(h,msg)  \
          if ((h)==0)       \
	    abort(); \
	    else;
#endif

#else
#ifdef NOCHECKPOINTERS
  #define CheckPointer(ptr,msg) (ptr)
#else
  #define  CheckPointer(ptr,msg)   \
           if( (ptr) == 0 )          \
              throw CError("Pointer is NULL",msg,__FILE__,__LINE__); \
              else;
#endif

#ifdef NOCHECKBOUNDS
  #define CheckBounds(i,min,max,msg) (i)
  #define CheckUpperBound(i,max,msg) (i)
#else
  #define  CheckBounds(i,min,max,msg)   \
           if( (i)<(min) || (i)>(max))          \
              throw CError("Bound check error",(msg),__FILE__,__LINE__); \
              else;
  #define  CheckUpperBound(i,max,msg)   \
           if((i)>(max))          \
              throw CError("Bound check error",(msg),__FILE__,__LINE__); \
              else;
#endif

#ifdef NOCHECKHANDLES
  #define CheckHandle(h,msg) (h)
#else
  #define CheckHandle(h,msg)                                       \
          if ((h)==0) throw CError("Handle is zero",(msg),__FILE__,__LINE__);\
                      else;
#endif

#ifdef NOCHECKCONDITIONS
  #define CheckCond(h,msg)      (h)
#else
  #define CheckCond(h,msg)  \
          if ((h)==0)       \
	    throw CError("Assertion failed",(msg),__FILE__,__LINE__); \
	    else;
#endif
#endif

#endif
