#include "copyfile.h"
#include "envdeps.h"
#include "log.h"
#if defined(__OS2__)
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>
#elif defined(__NT__)
#include <windows.h>
#else
#include <stdio.h>              // remove
#include <fstream.h>
#include "findfile.h"
#endif

/* copyfile is expected to ...
   - return COPY_NOTEXIST if the source path exists, but the specified
     file cannot be found in this path
   - return COPY_OTHER if any other error occurs, including
     non-existance of source or destination path, or others
   - return COPY_NOERR otherwise.
*/

int copyfile(const char *cpDest, const char *cpSource)
{
#if defined(__OS2__)
  APIRET rc;
  HDIR FindHandle;
  FILEFINDBUF3 FindBuffer;
  ULONG FindCount;
  
  logmsg(LOGDBG, "copyfile: using OS/2 API routines");
      
  FindHandle = HDIR_CREATE; FindCount = 1;
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
    case ERROR_NO_MORE_FILES: return COPY_NOTEXIST;
    case NO_ERROR: break;
    default: return COPY_OTHER;
    }
    
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
  
  if (CopyFile((LPCTSTR)cpSource,(LPCTSTR)cpDest,FALSE)==TRUE)
    {
      return COPY_NOERR;
    }
  else
    {
      rv = GetLastError();
      logmsg(LOGDBG, "copyfile: CopyFile last error is %d", (int) rv);
      if (rv == ERROR_FILE_NOT_FOUND)
        return COPY_NOTEXIST;
      else
        return COPY_OTHER;
    }
      
#else
  static unsigned char *cpBuf=new unsigned char[BUFLEN];
  int retval=COPY_NOERR;

  logmsg(LOGDBG, "copyfile: using built-in routines");

  ofstream fo(cpDest,ios::bin|ios::out);
  if (!fo)
    {
      logmsg(LOGDBG, "copyfile: failed to create output stream");
      return COPY_OTHER;
    }

  ifstream fi(cpSource,ios::bin|ios::in);
  if (!fi) 
    {
      logmsg(LOGDBG, "copyfile: failed to open input stream");
      
      fo.close();
      remove(cpDest);

      // determine if the path exists or not
      CArray<CString> *pTemp = findfile(cpSource);
      
      if (pTemp!=NULL)
        {
          if (pTemp->Size() == 0)
            {
              delete pTemp;
              logmsg(LOGDBG, "copyfile: source file had zero length");
              return COPY_NOTEXIST;
            }
          delete pTemp;
        }
      return COPY_OTHER;
    }

  while (fi)
    {
      int gc;

      fi.read(cpBuf,BUFLEN);
      if ((gc=fi.gcount())!=0)
        fo.write(cpBuf,gc);
      if (!fo)
        {
          logmsg(LOGDBG, "copyfile: problem when writing output");
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
#endif
}



