#ifdef INCS_NEED_DOT_H
#include <stdio.h>
#include <fstream.h>
#include <ctype.h>
#else
#include <stdio>
#include <fstream>
#include <ctype>
#endif
#include "cinbound.h"
#include "copyfile.h"
#include "log.h"

void CInbound::makevalidpath()
{
  if (!(strPath.charAt(strPath.Length()-1)==':'||
        strPath.charAt(strPath.Length()-1)=='\\'||
        strPath.charAt(strPath.Length()-1)=='/'))
    strPath+='/';
}

                                // MakeNewFileName checks if the filename
                                // passed as argument already does exist. If
                                // not, the same name is returned. If it does
                                // exist, a new filename is created. This way,
                                // we assure that received files never
                                // overwrite existing ones.
                                // May return an empty name if no name could be
                                // found.

static CString makeNewFilename(const CString& strName)
{
  CString str=strName;
  char *lastch;
  static char altExts[]="0123456789abcdefghijklmnopqrstuvwxyz";
  size_t length;
  size_t replace_pos;

  lastch=altExts;
  length = str.Length();
  replace_pos = length - 1;

  /* Rename ?UT-Packets to .PKT */
  if (length > 4)
    {
      if (toupper(str.charAt(length-1)) == 'T' &&
          toupper(str.charAt(length-2)) == 'U' &&
          toupper(str.charAt(length-4)) == '.')
        {
          str.setCharAt(length-3,'p');
          str.setCharAt(length-2,'k');
          str.setCharAt(length-1,'t');
          replace_pos = length - 5;  // don't modify PKT extension - modify
                                     // base name instead!
        }
    }

  if (length)
    {
      while (1)
        {
          ifstream ifs(str,ios::bin|ios::in);
          if (!ifs)                  // file does not exist
            break;
          ifs.close();               // file did exist - find new name

          if (*lastch)
            str.setCharAt(replace_pos, *(lastch++));
          else
            {
              str="";       // could not find a new name
              break;
            }
        }
    }

  return str;
}


CInbound::CInbound():strPath(){ makevalidpath(); receivedAnything = 0;}
CInbound::CInbound(const CString&r):strPath(r)
{
    makevalidpath();
    receivedAnything = 0;
}

int CInbound::Receive(const CString& strFilename, int killflag)
{
  CString strName,strDestname;
  size_t index;
  int crc;
                                // find the corresponding directory name
  for (index=strFilename.Length();index>0;index--)
    if (strFilename.charAt(index-1)=='\\'||
        strFilename.charAt(index-1)=='/'||
        strFilename.charAt(index-1)==':')
      break;

  if (index<strFilename.Length()||strFilename.Length()==0)
    strName=strFilename.substr(index,strFilename.Length()-1);
  else
    {
      logmsg(LOGDBG, "CInbound::Receive: error interpreting file name for %s",
                      (const char *)strFilename);
      return 0;                   // file not found ...
    }

  strDestname=makeNewFilename(strPath+strName);
  if (!strDestname.Length())
    {
      logmsg(LOGDBG, "CInbound::Receive: error creating destination "
                     "file name for %s plus %s",
                     (const char *)strPath, (const char *)strName);
      return 0;           // name for destination file could not be found
    }

  logmsg(LOGDBG, "Receiving %s as %s", (const char *)strFilename,
                                       (const char *)strDestname);
  crc = copyfile(strDestname,strFilename);
  logmsg(LOGDBG, "CInbound::Receive: Return code of copyfile is %d",
         (int) crc);

  switch(crc)
    {
    case COPY_NOERR:
      if (killflag==KILL_KILL)
        remove(strFilename);
      if (killflag==KILL_TRUNC)
        {
          remove(strFilename);
          ofstream f(strFilename);
          f.close();
        }
      receivedAnything = 1;
      return RECERR_NOERROR;
    case COPY_NOTEXIST:
      return RECERR_NOTEXIST;
    }
  return RECERR_OTHER;
}

int CInbound::gotMail(void)
{
  return receivedAnything;
}

