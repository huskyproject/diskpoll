#include "envdeps.h"
#ifdef SHORTINCLUDEFILENAMES
#include <strstrea.h>           // strstream
#else
#include <strstream.h>
#endif
#include "findfile.h"
#include "cbinkout.h"
#include <stdlib.h>             // abort
#include <fstream.h>
#include <stdio.h>              // unlink
#include <ctype.h>
#include "log.h"


CString CBinkleyOutbound::_packetExtension (int flavour)
{
  switch (flavour)
    {
    case FL_NORMAL:    return CString(".out");
    case FL_HOLD:      return CString(".hut");
    case FL_DIRECT:    return CString(".dut");
    case FL_CRASH:     return CString(".cut");
    case FL_IMMEDIATE: return CString(".iut");
    case FL_FILEMASK:  return CString(".?ut");
    default: abort();
    }
  return NULLSTRING;
}

CString CBinkleyOutbound::_flowfileExtension(int flavour)
{
  switch (flavour)
    {
    case FL_NORMAL:    return CString(".flo");
    case FL_HOLD:      return CString(".hlo");
    case FL_DIRECT:    return CString(".dlo");
    case FL_CRASH:     return CString(".clo");
    case FL_IMMEDIATE: return CString(".ilo");
    case FL_FILEMASK:  return CString(".?lo");
    default: abort();
    }
  return NULLSTRING;
}

CString CBinkleyOutbound::_nodebaseName(const CNode& n)
{
  strstream sext, sext2, sext3;
  char *cpext;

  sext.fill('0');
  sext << hex << strBasepath;
  if (n.Zone()!=baseZone)
    {
      sext << '.';
      sext.width(3);
      sext << n.Zone();
    }
  sext << '\0';
  if (!adaptcase(sext.str()))
    {
      mymkdir(sext.str());
    }
  sext2 << hex;
  sext2 << sext.str();
  sext2 << '/';
  sext2.fill('0');
  sext2.width(4);  sext2 << n.Net();
  sext2.width(4);  sext2 << n.Node();
  if (n.Point()!=0)
    {
      sext2 << ".pnt" << '\0'; 
      if (!adaptcase(sext2.str()))
        {
          mymkdir(sext2.str());
        }
      sext3 << hex;
      sext3 << sext2.str();
      sext3.fill('0');
      sext3 << "/";
      sext3.width(8);  sext3 << n.Point();
    }
  else
    {
      sext2 << '\0';
      sext3 << sext2.str();
    }
  sext3 << '\0';
  cpext=sext3.str();
  CString s(cpext);
  delete cpext;

  return s;
}

CString CBinkleyOutbound::_flowfileName(const CNode& n, int flavour)
{
  return _nodebaseName(n)+
         _flowfileExtension(flavour);
}

CString CBinkleyOutbound::_packetfileName(const CNode& n, int flavour)
{
  return _nodebaseName(n)+_packetExtension(flavour);
}


CArray<CSendFile>* CBinkleyOutbound::getFilesFor(const CNode&n, int flavour)
{
  CArray<CSendFile>* pRet=new CArray<CSendFile>;
  CArray<CString>*   pTemp;
  unsigned long      i;

  CheckPointer(pRet,"CBinkleyOutbound::getFilesFor()");

  pTemp=findfile(_flowfileName(n,FL_FILEMASK));
  if (pTemp!=NULL)
  {
    for (i=0;i<pTemp->Size();i++)
      {
        int fl;
        CString flf=(*pTemp)[i];

        for (fl=FL_FIRST;fl<=FL_LAST;fl++)
          {
            if ((flavour != -1) && (!((1 << fl) & flavour)))
              continue;
              
            CString ffe=_flowfileExtension(fl);

            if ( flf.Length() >= ffe.Length() )
              if (upcase(ffe) ==
                  upcase(flf.substr(flf.Length()-ffe.Length(),flf.Length()-1)))
                {
                  ifstream f(flf);
                  CString s;

                  while (f)
                    {
                      CString rawInput,fileName;
                      int killflag;

                      f >> rawInput;
                      if (rawInput.Length())
                        {
                          switch (rawInput.charAt(0))
                            {
                            case '~':
                              continue; // skip this one
                            case '#':
                              killflag=KILL_TRUNC;
                              fileName=rawInput.substr(1,rawInput.Length()-1);
                              break;
                            case '^':
                              killflag=KILL_KILL;
                              fileName=rawInput.substr(1,rawInput.Length()-1);
                              break;
                            default:
                              killflag=KILL_NOKILL;
                              fileName=rawInput;
                            }
                          fileName=importFilenameFromFlowfile(fileName);
                          CSendFile sf(fileName,fl,killflag);
                          pRet->Add(sf);
                        }
                    }
                  f.close();
                }
          }
      }
    delete pTemp;
  }

  pTemp=findfile(_packetfileName(n,FL_FILEMASK));
  if (pTemp!=NULL)
  {
    for (i=0;i<pTemp->Size();i++)
      {
        int fl;
        CString pf=(*pTemp)[i];

        for (fl=FL_FIRST;fl<=FL_LAST;fl++)
          {
            if ((flavour != -1) && (!((1 << fl) & flavour)))
              continue;

            CString pfe=_packetExtension(fl);

            if ( pf.Length() >= pfe.Length() )
              if (upcase(pfe) ==
                  upcase(pf.substr(pf.Length()-pfe.Length(),pf.Length()-1)))
                {
                  CSendFile sf(pf,fl,KILL_KILL);
                  pRet->Add(sf);
                }
          }
      }
    delete pTemp;
  }

  return pRet;
}

unsigned long CBinkleyOutbound::sendFilesTo (const CNode&n, CArray<CSendFile>*pFiles)
{
  unsigned long i;

  for (i=0;i<pFiles->Size();i++)
    {
      ofstream f(_flowfileName(n,(*pFiles)[i].Flavour()),ios::app);
      if (!f) break;
      if ((*pFiles)[i].Killflag()==KILL_TRUNC)
        f << "#";
      if ((*pFiles)[i].Killflag()==KILL_KILL)
        f << "^";
      f << exportFilenameToFlowfile((*pFiles)[i].Filename()) << '\n';
    }

  return i;
}


static int fncomparator(const void *a, const void *b)
{
  const CSendFile *p1=(const CSendFile*)a;
  const CSendFile *p2=(const CSendFile*)b;
  return strcmp(p1->Filename(),p2->Filename());
}

int CBinkleyOutbound::removeFilesFor(const CNode& n, CArray<CSendFile>* pFiles)
{
  CArray<CSendFile>* pOrg;
  CArray<CSendFile>* pNew=new CArray<CSendFile>();
  unsigned long i,j;
  int cr,flavour;

  logmsg(LOGDBG, "CBinkleyOutbound::removeFilesFor");

  pOrg=getFilesFor(n);

  logmsg(LOGDBG, "dumping list of files that are currently on hold");
  for (i = 0; i < pOrg->Size(); i++)
    {
      logmsg(LOGDBG," %s", (const char *)(*pOrg)[i].Filename());
    }

  logmsg(LOGDBG, "dumping list of files schedule for removal from flow files");
  for (i = 0; i < pFiles->Size(); i++)
    {
      logmsg(LOGDBG," %s", (const char *)(*pFiles)[i].Filename());
    }

  pOrg->Sort(fncomparator);
  pFiles->Sort(fncomparator);


  i=j=0; cr=0;

  while (i<pOrg->Size())
    {
      if (j<pFiles->Size())
        cr=strcmp((*pOrg)[i].Filename(),(*pFiles)[j].Filename());
      else
        cr=-1;
      if (cr<0) cr=-1;
      if (cr>0) cr=1;
      switch (cr)
        {
        case 1:                 // pOrg[i]>pFiles[j]
          j++;
          break;
        case -1:                // pOrg[i]<pFiles[j]
          pNew->Add((*pOrg)[i]);
          i++;
          break;
        case 0:                 // pOrg[i]==pFiles[j]
                                // pOrg[i] will not be in the output - remove
                                // the reference. kill or trunc is done by the
                                // inbound routines.
          i++;
          j++;
          break;
        }
    }

  // remove all flow files
 for (flavour=FL_FIRST;flavour<=FL_LAST;flavour++)
   {
     remove(_flowfileName(n,flavour));
   }


  logmsg(LOGDBG, "dumping new list of files on hold");
  for (i = 0; i < pNew->Size(); i++)
    {
      logmsg(LOGDBG," %s", (const char *)(*pNew)[i].Filename());
    }


  i=sendFilesTo(n,pNew);          // rebuild the flow files

  delete(pOrg);
  delete(pNew);

  return (i==pNew->Size());
}


CBinkleyOutbound::CBinkleyOutbound():COutbound()
{ baseZone=2; outDirSep=DEFDIRSEP; 
  pFloFilePrefixes=pLocFilePrefixes=(CArray<CString>*)NULL; 
}

CBinkleyOutbound::CBinkleyOutbound(const CString& strBasepath, long ownZone)
  :COutbound(strBasepath)
{ baseZone=ownZone; outDirSep=DEFDIRSEP; 
  pFloFilePrefixes=pLocFilePrefixes=(CArray<CString>*)NULL;
}

void CBinkleyOutbound::setLanReplaceCharacteristics
   (CArray<CString>* pFlo, CArray<CString>* pLoc, char cSep)
{
  if (pFloFilePrefixes!=NULL)
    delete pFloFilePrefixes;
  if (pLocFilePrefixes!=NULL)
    delete pLocFilePrefixes;
  pFloFilePrefixes=pFlo;
  pLocFilePrefixes=pLoc;
  outDirSep=cSep;
}

CBinkleyOutbound::~CBinkleyOutbound()
{
  setLanReplaceCharacteristics((CArray<CString>*)NULL,
                               (CArray<CString>*)NULL,
                               outDirSep);
}

static int fileExists(const CString& cpFilename)
{
  CString s = cpFilename;
  return adaptcase(s);
}

static int makeFile(const CString& cpFilename)
{
  ofstream f(cpFilename);
  if (f)
    {
      f.close();
      return 1;
    }
  else
    return 0;
}

int CBinkleyOutbound::lockNode(const CNode& node)
{
  CString cpBusyfile=_nodebaseName(node)+".bsy";
  if (fileExists(cpBusyfile))
    return 0;
  else
    return makeFile(cpBusyfile);
}

void CBinkleyOutbound::unlockNode(const CNode& node)
{
  remove(_nodebaseName(node)+".bsy");
}

CString CBinkleyOutbound::importFilenameFromFlowfile(const CString& flowName)
{
  CString retval=flowName;
  unsigned long index;

  if (pFloFilePrefixes!=NULL&&pLocFilePrefixes!=NULL)
    {
      unsigned long i;

      CheckCond(pFloFilePrefixes->Size()==pLocFilePrefixes->Size(),
                "CBinkleyOutbound::importFilenameFromFlowfile()");
      for (i=0;i<pFloFilePrefixes->Size();i++)
        {
          const char *cp,*cp2;

          if ((*pFloFilePrefixes)[i].Length()>flowName.Length())
            continue;

          for (cp=(*pFloFilePrefixes)[i],
               cp2=flowName; *cp; cp++, cp2++)
            {
              if (outDirSep=='\\')
                if (toupper(*cp)!=toupper(*cp2))
                  break;
                else;
              else
                if (*cp!=*cp2)
                  break;
                else;
            }

          if (!(*cp))
            {
              retval=(*pLocFilePrefixes)[i];
              retval+=cp2;
              break;
            }
        }
    }

  for (index=0;index<retval.Length();index++)
    if (retval.charAt(index)==outDirSep)
      retval.setCharAt(index,'/');

  adaptcase(retval);

  return retval;
}


CString CBinkleyOutbound::exportFilenameToFlowfile(const CString& locName)
{
  CString retval=locName;
  unsigned long index;

  if (pFloFilePrefixes!=NULL&&pLocFilePrefixes!=NULL)
    {
      unsigned long i;

      CheckCond(pFloFilePrefixes->Size()==pLocFilePrefixes->Size(),
                "CBinkleyOutbound::importFilenameFromFlowfile()");
      for (i=0;i<pLocFilePrefixes->Size();i++)
        {
          const char *cp,*cp2;

          if ((*pLocFilePrefixes)[i].Length()>locName.Length())
            continue;

          for (cp=(*pLocFilePrefixes)[i],
               cp2=locName; *cp; cp++, cp2++)
            {
              if (outDirSep=='\\')
                if (toupper(*cp)!=toupper(*cp2))
                  break;
                else;
              else
                if (*cp!=*cp2)
                  break;
                else;
            }

          if (!(*cp))
            {
              retval=(*pFloFilePrefixes)[i];
              retval+=cp2;
              break;
            }
        }
    }

  for (index=0;index<retval.Length();index++)
    if (retval.charAt(index)=='/')
      retval.setCharAt(index,outDirSep);

  return retval;
}












