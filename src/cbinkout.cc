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
  strstream sext;
  char *cpext;

  sext.fill('0');
  sext << hex << strBasepath;
  if (n.Zone()!=baseZone)
    {
      sext << '.';
      sext.width(3);
      sext << n.Zone();
    }
  sext << '/';
  sext.fill('0');
  sext.width(4);  sext << n.Net();
  sext.width(4);  sext << n.Node();
  if (n.Point()!=0)
    {
      sext << ".pnt/"; sext.width(8);  sext << n.Point();
    }
  sext << '\0';
  cpext=sext.str();
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
                          switch (rawInput[0])
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

  pOrg=getFilesFor(n);

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

  i=sendFilesTo(n,pNew);          // rebuild the flow files

  delete(pOrg);
  delete(pNew);

  return (i==pNew->Size());
}


CBinkleyOutbound::CBinkleyOutbound():COutbound()
{ baseZone=2; outDirSep=DEFDIRSEP; pFloFilePrefixes=pLocFilePrefixes=NULL; }
CBinkleyOutbound::CBinkleyOutbound(const CString& strBasepath, long ownZone)
  :COutbound(strBasepath)
{ baseZone=ownZone;
  outDirSep=DEFDIRSEP; pFloFilePrefixes=pLocFilePrefixes=NULL;
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
  setLanReplaceCharacteristics(NULL,NULL,outDirSep);
}

static int fileExists(const CString& cpFilename)
{
  ifstream f(cpFilename);
  if (f)
    {
      f.close();
      return 1;
    }
  else
    return 0;
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
  CString cpBusyfile=_nodebaseName(node)+".BSY";
  if (fileExists(cpBusyfile))
    return 0;
  else
    return makeFile(cpBusyfile);
}

void CBinkleyOutbound::unlockNode(const CNode& node)
{
  remove(_nodebaseName(node)+".BSY");
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
    if (retval[index]==outDirSep)
      retval[index]='/';

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
    if (retval[index]=='/')
      retval[index]=outDirSep;

  return retval;
}











