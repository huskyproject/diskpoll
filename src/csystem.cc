#ifdef INCS_NEED_DOT_H
#include <stdlib.h>             // NULL
#include <fstream.h>            // ofstream
#else
#include <stdlib>             // NULL
#include <fstream>            // ofstream
#endif
#include "envdeps.h"
#include "cnode.h"
#include "csystem.h"
#include "log.h"
#include "prepcfg.h"
#include "cbinkout.h"
#include "csplout.h"
#include "findfile.h"

CSystem::CSystem()
{
  pAkas    =(CArray<CNode>*) NULL;
  pInbound =(CInbound*)      NULL;
  pOutbound=(COutbound*)     NULL;
  strName  =                 NULLSTRING;
  strMailFlag =              NULLSTRING;
}

CSystem::CSystem(COutbound* pout, CInbound* pin, CArray<CNode>* pad, const
                 CString& str, const CString& mf)
{
  pAkas=NULL; pInbound=NULL; pOutbound=NULL;
  setInbound(pin);
  setOutbound(pout);
  setAkas(pad);
  strName=str;
  strMailFlag=mf;
}

CSystem::CSystem(const CSystem& s)
{
  pAkas=NULL; pInbound=NULL; pOutbound=NULL; strName=NULLSTRING;
  strMailFlag = NULLSTRING;
  (*this)=s;
}

CSystem& CSystem::operator=(const CSystem&s)
{
  abort();                      // does not work yet!
  if (pAkas)     delete pAkas;
  if (pInbound)  delete pInbound;
  if (pOutbound) delete pOutbound;
  //  pOutbound=new COutbound(*(s.pOutbound)); this line is the problem
  pInbound =new CInbound (*(s.pInbound));
  //  pAkas    =new CArray<CNode>(*(s.pAkas));
  strName  =s.strName;
  strMailFlag = s.strMailFlag;

  return (*this);
}

CSystem::~CSystem()
{
  if (pAkas)     delete pAkas;
  if (pInbound)  delete pInbound;
  if (pOutbound) delete pOutbound;
}

void CSystem::setInbound (CInbound* pin)
{
  if (pInbound) delete pInbound;
  pInbound=pin;
}

void CSystem::setOutbound(COutbound* pout)
{
  if (pOutbound) delete pOutbound;
  pOutbound=pout;
}

void CSystem::setAkas(CArray<CNode>* pakas)
{
  if (pAkas) delete pAkas;
  pAkas=pakas;
}

void CSystem::setMailFlag(const CString& mf)
{
  strMailFlag = mf;
}

const CString& CSystem::getMailFlag() const
{
  return strMailFlag;
}

const CString& CSystem::getName() const
{
  return strName;
}

void CSystem::poll(CSystem& uplink, int sendFlavour, int receiveFlavour)
{
  unsigned long i,j;
  int recerr;

  logmsg(LOGMSG,"CONNECT: Uplink: %s, Downlink: %s",
         (const char *)uplink.getName(),(const char*)getName());

  for (i=0;i<uplink.pAkas->Size();i++)
    {

      /* Send my files to uplink */

      if (pOutbound->lockNode((*(uplink.pAkas))[i]))
        {
          CArray<CSendFile>
            *pFiles=pOutbound->getFilesFor((*(uplink.pAkas))[i], sendFlavour);
          CArray<CSendFile> sentFiles;

          logmsg(LOGMSG,"Selected %lu files to send to %s",
                        pFiles->Size(), LOGNODE((*(uplink.pAkas))[i]));

          for (j=0;j<pFiles->Size();j++)
            {
              recerr = uplink.pInbound->Receive((*pFiles)[j].Filename(),
                                                (*pFiles)[j].Killflag());
              if (recerr == RECERR_NOERROR)
                {
                  logmsg(LOGMSG,"Sent %s to %s",
                         (const char*)(*pFiles)[j].Filename(),
                         LOGNODE((*(uplink.pAkas))[i]));
                  sentFiles.Add((*pFiles)[j]);
                }
              else
                {
                  logmsg(LOGMSG,"Could not send %s to %s",
                         (const char*)(*pFiles)[j].Filename(),
                         LOGNODE((*(uplink.pAkas))[i]));
                  if (recerr == RECERR_NOTEXIST)
                    {
                       sentFiles.Add((*pFiles)[j]);
                    }
                }
            }

          pOutbound->removeFilesFor((*(uplink.pAkas))[i],&sentFiles);
          delete pFiles;

          pOutbound->unlockNode((*(uplink.pAkas))[i]);
        }
      else
        {
          logmsg(LOGERR,"Downlink reports: Other node sending to %s",
                 LOGNODE((*(uplink.pAkas))[i]));
        }
    }

  for (i=0;i<pAkas->Size();i++)
    {

      /* Receive files from uplink */

      if (uplink.pOutbound->lockNode((*pAkas)[i]))
        {
          CArray<CSendFile>*
              pFiles=uplink.pOutbound->getFilesFor((*pAkas)[i],
                                                   receiveFlavour);
          CArray<CSendFile> receivedFiles;

          logmsg(LOGMSG,"Selected %lu files to receive at %s",
                 pFiles->Size(), LOGNODE((*pAkas)[i]));

          for (j=0;j<pFiles->Size();j++)
            {

              recerr = pInbound->Receive((*pFiles)[j].Filename(),
                                         (*pFiles)[j].Killflag());
              if (recerr == RECERR_NOERROR)
                {
                  logmsg((LOGMSG),"Received %s at %s",
                         (const char*)(*pFiles)[j].Filename(),
                         LOGNODE((*pAkas)[i]));
                  receivedFiles.Add((*pFiles)[j]);
                }
              else
                {
                  logmsg((LOGMSG),"Could not receive %s at %s",
                                   (const char*)(*pFiles)[j].Filename(),
                                   LOGNODE((*pAkas)[i]));
                  if (recerr == RECERR_NOTEXIST)
                    {
                      logmsg(LOGDBG,
                         "Will remove non-existant file %s from flow file.",
                         (const char*)(*pFiles)[j].Filename());
                      receivedFiles.Add((*pFiles)[j]);
                    }
                }
            }

          uplink.pOutbound->removeFilesFor((*pAkas)[i],&receivedFiles);
          delete pFiles;
          uplink.pOutbound->unlockNode((*pAkas)[i]);
        }
      else
        logmsg(LOGERR,"Uplink reports: Other node sending to %s",
               LOGNODE((*pAkas)[i]));
    }
  if (uplink.pInbound->gotMail() && !(uplink.getMailFlag() == NULLSTRING))
    {
      ofstream f(uplink.getMailFlag());
      f.close();
      logmsg(LOGMSG,"Created flag file %s", (const char*)(uplink.getMailFlag()));
    }
  if (pInbound->gotMail() && !(getMailFlag() == NULLSTRING))
    {
      ofstream f(getMailFlag());
      f.close();
      logmsg(LOGMSG,"Created flag file %s", (const char *)(getMailFlag()));
    }
  logmsg(LOGMSG,"DISCONNECT");
}

                                // this function builds a system descriptor
                                // class from an input stream. the input stream
                                // is expected to be a text stream containing
                                // the configuration statements.

istream& operator >> (istream& ifs, CSystem& system)
{
  int error=0;
                                // re-initialize the system
  if (system.pAkas)     delete system.pAkas;
  if (system.pInbound)  delete system.pInbound;
  if (system.pOutbound) delete system.pOutbound;
  system.pAkas=new CArray<CNode>();
  CheckPointer(system.pAkas,
               "istream& operator >> (istream&, CSystem&)");
  system.pInbound=0;
  system.pOutbound=0;

  CString strOutboundType=NULLSTRING;
  CString strOutboundBase=NULLSTRING;
  CString strInbound=NULLSTRING;
  CString strMailFlag=NULLSTRING;

  CString strLine;

  TWords  *pOutboundOptions=NULL;

  while (ifs)                   // as long as eof is not reached and no error
                                // has occured
    {
      CString strKeyword;
      TWords  words("");
      int     nwords;

      if (ifs.peek()=='[')        // new section begins
        break;

      ifs >> strLine;
      words=prepareConfigString(strLine); nwords=words.getNWords();

      if (!nwords)              // empty line
        continue;

      strKeyword=upcase(words.getWord(0));
      if (strKeyword=="OUTBOUNDTYPE")
        {
          if (nwords<2)
            {
              error=1; break;
            }

          strOutboundType=words.getWord(1);

          if (pOutboundOptions!=NULL)
            delete pOutboundOptions;

          pOutboundOptions=new TWords(words);

        } else
      if (strKeyword=="OUTBOUNDBASE")
        {

          strOutboundBase=words.getWord(1);
        } else
      if (strKeyword=="MAILFLAG")
        {
          if (nwords!=2) { error=1; break; }
          strMailFlag=words.getWord(1);
          adaptcase(strMailFlag);
        } else
      if (strKeyword=="INBOUND")
        {
          if (nwords!=2) { error=1; break; }
          strInbound=words.getWord(1);
        } else
      if (strKeyword=="ADDRESS"||strKeyword=="AKA")
        {
          if (nwords!=2) { error=1; break; }
          CNode node(words.getWord(1));
          if (node.Zone()==0)   // error
            {
              error=1; break;
            }
          system.pAkas->Add(node);
        } else { error=1; break; }

    }

  if (error)
    logmsg(LOGERR,"System [%s]: Syntax error: \"%s\"",
           (const char*) system.getName(),
           (char*)strLine);


  if (!error)
    while (1)
      {
        if (strOutboundType.Length()==0||
            strOutboundBase.Length()==0||
            strInbound.Length()==0)
          {
            logmsg (LOGERR, "System [%s]: Definition is incomplete.",
                    (const char*) system.getName());
            error=1;
            break;
          }
        if (system.pAkas->Size()<1)
          {
            logmsg (LOGERR, "System [%s]: Definition is incomplete.",
                    (const char*) system.getName());
            error=1;
            break;
          }

        system.pInbound=new CInbound(strInbound);
        CheckPointer(system.pInbound,
                     "istream& operator >> (istream&, CSystem&)");

        if (upcase(strOutboundType)=="BINKLEY")
          {
            CBinkleyOutbound *pBinkleyOutbound=new
              CBinkleyOutbound(strOutboundBase, (*(system.pAkas))[0].Zone());
            CheckPointer(pBinkleyOutbound,
                         "istream& operator >> (istream&,CSystem&)");

            if (pOutboundOptions->getNWords()>2)
              {
                char dirsep=DEFDIRSEP;
                CArray<CString> *pflo=new CArray<CString>();
                CArray<CString> *ploc=new CArray<CString>();
                int i=2;

                if (strlen(pOutboundOptions->getWord(i))==1)
                  dirsep=pOutboundOptions->getWord(i++)[0];

                for (;i<pOutboundOptions->getNWords();i++)
                  {
                    char *opt=pOutboundOptions->getWord(i);
                    char *cp=strchr(opt,'=');

                    if (cp==opt||cp==NULL||cp==opt+strlen(opt)-1)
                      {
                        logmsg(LOGERR,
                               "System [%s]: Invalid Outbound Option %s",
                               (const char *)system.getName(),
                               opt);
                        error=1;
                        break;
                      }

                    CString strOpt(opt);
                    CString strFlo=strOpt.substr(0,cp-opt-1);
                    CString strLoc(cp+1);

                    pflo->Add(strFlo);
                    ploc->Add(strLoc);
                  }

                pBinkleyOutbound->setLanReplaceCharacteristics(pflo,ploc,
                                                               dirsep);
              }

            if (error)
              break;

            system.pOutbound=pBinkleyOutbound;
          }
        else if (upcase(strOutboundType)=="TRIVIAL")
          {
            CSpoolOutbound *pSpoolOutbound=new CSpoolOutbound(strOutboundBase);
            CheckPointer(pSpoolOutbound,"istream& operator >> (istream&, CSystem&)");
            system.pOutbound=pSpoolOutbound;
          }
        else
          {
            logmsg (LOGERR, "System [%s]: Unknown Outbound Type %s.",
                    (const char*)system.getName(), (char*)strOutboundType);
            error=1;
            break;
          }

        if (!(strMailFlag == NULLSTRING))
          {
            system.setMailFlag(strMailFlag);
          }

        break;
    }

  if (error)
    {
      ifs.setstate(ios::failbit);

      if (system.pAkas)     delete system.pAkas;
      if (system.pInbound)  delete system.pInbound;
      if (system.pOutbound) delete system.pOutbound;

      system.pAkas=0;
      system.pInbound=0;
      system.pOutbound=0;
    }
  else
    {
      int state=ifs.rdstate();
      ifs.clear();
      ifs.setstate(state&(~ios::failbit));
    }

  if (pOutboundOptions!=NULL)
    delete pOutboundOptions;

  return ifs;
}



