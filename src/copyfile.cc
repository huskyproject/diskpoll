#include "copyfile.h"
#include "envdeps.h"
#include "log.h"

#if defined(__OS2__)
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>
#elif defined(__NT__)
#include <windows.h>
#endif

#include <errno.h>
#include <stdio.h>              // remove
#include <fstream.h>
#include "findfile.h"

/* copyfile is expected to ...
   - return COPY_NOTEXIST if the source path exists, but the specified
     file cannot be found in this path
   - return COPY_OTHER if any other error occurs, including
     non-existance of source or destination path, or others
   - return COPY_NOERR otherwise.
*/

static int api_copyfile(const char*, const char*);
int copyfile_no_api = 0;

int copyfile(const char *cpDest, const char *cpSource)
{
#if defined(__OS2__) || defined(__NT__)
  if (!copyfile_no_api)
  {
      return api_copyfile(cpDest, cpSource);
  }
#endif  

  static unsigned char *cpBuf=new unsigned char[BUFLEN];
  int retval=COPY_NOERR;

  logmsg(LOGDBG, "copyfile: using built-in copy routine");
  logmsg(LOGDBG, "copyfile: copy %s -> %s", cpSource, cpDest);

  ofstream fo(cpDest,ios::bin|ios::out);
  if (!fo)
    {
      logmsg(LOGDBG, "copyfile: failed to create output stream: %s",
             strerror(errno));
      return COPY_OTHER;
    }

  ifstream fi(cpSource,ios::bin|ios::in);
  if (!fi)
    {
      logmsg(LOGDBG, "copyfile: failed to open input stream: %s",
             strerror(errno));

      fo.close();
      remove(cpDest);

      // determine if the path exists or not
      CArray<CString> *pTemp = findfile(cpSource);

      if (pTemp!=NULL)
        {
          if (pTemp->Size() == 0)
            {
              delete pTemp;
              logmsg(LOGDBG, "copyfile: source file did not exist or had zero length");
              return COPY_NOTEXIST;
            }
          delete pTemp;
        }
      return COPY_OTHER;
    }

  logmsg(LOGDBG, "copyfile: successfully opened input and output stream.");

  while (fi)
    {
      int gc;

      fi.read(cpBuf,BUFLEN);
      if ((gc=fi.gcount())!=0)
        fo.write(cpBuf,gc);
      if (!fo)
        {
          logmsg(LOGDBG, "copyfile: problem when writing output: %s",
                 strerror(errno));
          retval=COPY_OTHER;
          break;
        }
    }

  fi.close();
  fo.close();
  if (retval != COPY_NOERR)                  // failure
    {
      logmsg(LOGDBG, "copyfile: some sort of problem, removing destination");
      remove(cpDest);
    }
  return retval;
}


/* Routine for copying via OS2 or Win32 API. This is a lot faster, so
   copyfile will call it if possible.
*/   

static int api_copyfile(const char *cpDest, const char *cpSource)
{
#if defined(__OS2__)
  APIRET rc;
  HDIR FindHandle;
  FILEFINDBUF3 FindBuffer;
  ULONG FindCount;

  logmsg(LOGDBG, "copyfile: using OS/2 API routines");

  FindHandle = HDIR_CREATE; FindCount = 1;
  logmsg(LOGDBG, "copyfile: DosFindFirst(%s, ...)", cpSource);
  rc = DosFindFirst((PSZ)cpSource, &FindHandle,
                    FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN |
                    FILE_READONLY,
                    (PVOID)&FindBuffer,
                    sizeof(FindBuffer),
                    &FindCount,
                    FIL_STANDARD);
  DosFindClose(FindHandle);

  logmsg(LOGDBG, "copyfile: DosFindFirst return code is %d", (int)rc);

  switch(rc)
    {
    case ERROR_NO_MORE_FILES:
        logmsg(LOGDBG, "copyfile: source file does not exist.");
        return COPY_NOTEXIST;
    case NO_ERROR: break;
    default: return COPY_OTHER;
    }

  logmsg(LOGDBG, "copyfile: DosCopy(%s,%s,%lx)",
                          (PSZ)cpSource,(PSZ)cpDest,DCPY_EXISTING);

  rc = DosCopy((PSZ)cpSource,(PSZ)cpDest,DCPY_EXISTING);

  logmsg(LOGDBG, "copyfile: DosCopy return code is %d", (int)rc);

  switch (rc)
    {
      case NO_ERROR:             return COPY_NOERR;
      case ERROR_FILE_NOT_FOUND: return COPY_NOTEXIST;
      default:                   return COPY_OTHER;
    }

#elif defined(__NT__)

  DWORD rv;

  logmsg(LOGDBG, "copyfile: using Win32 API routines");
  logmsg(LOGDBG, "copyfile: CopyFile(%s, %s, %d)", (char *) cpSource,
         (char *)cpDest, (int)FALSE);

  if (CopyFile((LPCTSTR)cpSource,(LPCTSTR)cpDest,FALSE)==TRUE)
    {
      logmsg(LOGDBG, "copyfile: CopyFile succeeded");
      return COPY_NOERR;
    }
  else
    {
      rv = GetLastError();
      logmsg(LOGDBG, "copyfile: CopyFile last error is %d", (int) rv);
      if (rv == ERROR_FILE_NOT_FOUND)
        {
          logmsg(LOGDBG, "copyfile: source file does not exist");
          return COPY_NOTEXIST;
        }
      else
        return COPY_OTHER;
    }

#else
 
  /* suppress warnings about unused parameters and functions*/
  ((void)(cpDest)); ((void)(cpSource));  ((void)(api_copyfile));
  
  logmsg(LOGERR, "copyfile: FATAL: don't have a copy API on this platform");
  return COPY_OTHER;

#endif
}

