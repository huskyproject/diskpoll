#include "csplout.h"
#include "cerror.h"
#include "cinbound.h"
#include "findfile.h"
#include "envdeps.h"
#include "coutb.h"
#include <stdio.h>              // remove
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define unused(x) ((void)(x));

CSpoolOutbound::CSpoolOutbound():COutbound(){}
CSpoolOutbound::CSpoolOutbound(const CString& str):COutbound(str){}
CArray<CSendFile>* CSpoolOutbound::getFilesFor   (const CNode& node,
                                                  int flavour)
{
  CArray<CSendFile>* pRet=new CArray<CSendFile>;
  CArray<CString>*   pTemp;
  unsigned long      i;

  unused(node); unused(flavour);

  CheckPointer(pRet,"CSpoolOutbound::getFilesFor()");

  pTemp=findfile(strBasepath+"/"ALLFILES);

  if (pTemp!=NULL)
    {
      struct stat sb;

      for (i=0;i<pTemp->Size();i++)
        if (!stat((*pTemp)[i],&sb))
          if (S_ISREG(sb.st_mode))        // only use regular files
            pRet->Add(CSendFile((*pTemp)[i],FL_NORMAL,KILL_KILL));

      delete pTemp;
    }
  return pRet;
}

unsigned long CSpoolOutbound::sendFilesTo(const CNode& n, CArray<CSendFile>
                                          *pFiles)
{
  unsigned long i;
  CInbound directory(strBasepath);

  unused(n);

  for (i=0;i<pFiles->Size();i++)
    {
                                // We can view a spooler outbound just like a
                                // Inbound in this case. We have to copy all
                                // files from pFiles into the outbound
                                // directory, assuring that they are renamed if
                                // files of the same name are already present.


      if (!directory.Receive((*pFiles)[i].Filename(),(*pFiles)[i].Killflag()))
        break;
    }
  return i;
}

int CSpoolOutbound::removeFilesFor(const CNode& n, CArray<CSendFile>* pFiles)
{
  unsigned long i;
  int rc = 1;
  unused(n);
  for (i=0;i<pFiles->Size();i++)
    if (remove((*pFiles)[i].Filename()))
      rc = 0;
  return rc;
}
