#include "envdeps.h"

#ifdef INCS_NEED_DOT_H
#include <iostream.h>
#include <fstream.h>
#include <ctype.h>
#ifdef NEED_GETOPTH
#include <getopt.h>
#endif
#ifdef NEED_UNISTDH
#include <unistd.h>
#endif
#else
#include <iostream>
#include <fstream>
#include <ctype>
#ifdef NEED_GETOPTH
#include <getopt>
#endif
#ifdef NEED_UNISTDH
#include <unistd>
#endif
#endif
#include "csystem.h"
#include "log.h"
#include "words.h"
#include "prepcfg.h"
#include "carray.h"
#include "copyfile.h"

                        // returns: 0: OK, 1: empty line, 2: not a section

static int getSectionName(const CString& strLine, CString& strSectionname)
{
  TWords words(""); unsigned long nwords;
  CString strRaw;

  words=prepareConfigString(strLine); nwords=words.getNWords();

  if (nwords>1)
    return 2;
  if (nwords==0)
    return 1;

  strRaw=CString(words.getWord(0));

  if (strRaw.Length()<3)
    return 2;
  if (strRaw.charAt(0)!='[' || strRaw.charAt(strRaw.Length()-1)!=']')
    return 2;

  strSectionname=strRaw.substr(1,strRaw.Length()-2);
  return 0;
}

static void doCall(const CString& strConfig, const CString& strUplink, const
                   CString& strDownlink, int sendFlavour, int receiveFlavour)
{
  ifstream ifs(strConfig,ios::in);
  CArray<CSystem*> vSystems;
  int fail=0;
  unsigned long i;

  if (!ifs)
    {
      logmsg(LOGERR,"Could not find configuration file %s",
             (const char*)strConfig);
      fail=1;
    }
                                // read the config file
  while ((!ifs.eof())&&(!fail))
    {
      CString strLine;
      CString strSectionname;

      ifs >> strLine;
      switch(getSectionName(strLine,strSectionname))
        {
        case 2:                 // syntax error
          logmsg(LOGERR,"Syntax Error: %s.",(char *)strLine);
          fail=1;
          continue;

        case 1:                 // empty line
          continue;

        default:                // new section
          {
            CSystem *paSystem=new CSystem((COutbound*)NULL,
                                          (CInbound*)NULL,
                                          (CArray<CNode>*)NULL,strSectionname);
            CheckPointer(paSystem,"main.cc: doCall()");
            ifs >> *paSystem;
            if (ifs.fail())
              {
                fail=1;
                continue;
              }
            else
              vSystems.Add(paSystem);
          }
       }
    }

  if (!fail)
  {
  CSystem *pUplink=0;
  CSystem *pDownlink=0;

  for (i=0;i<vSystems.Size();i++)
    {
      if ((vSystems[i]->getName())==strDownlink)
        pDownlink=vSystems[i];
      if ((vSystems[i]->getName())==strUplink)
        pUplink=vSystems[i];
    }

  if (pUplink==0)
    logmsg(LOGERR,"No definition found for uplink system named \"%s\"",
           (const char *)strUplink);
  if (pDownlink==0)
    logmsg(LOGERR,"No definition found for downlink system named \"%s\"",
           (const char *)strDownlink);

  if (pUplink!=0&&pDownlink!=0)
    {
      pDownlink->poll(*pUplink, sendFlavour, receiveFlavour);
    }
  }

  for (i=0;i<vSystems.Size();i++)
    delete vSystems[i];

  ifs.close();
}

int main(int argc, char **argv)
{
  int i,help=0;
  const char *cpConfigfile=DEFCONFIGFILE;
  const char *cpUplink  ="Uplink";
  const char *cpDownlink="Downlink";
  int sendFlavour = -1, receiveFlavour = -1;

  optind=1; opterr=0;
  while ((i=getopt(argc,argv,"s:r:c:hdw"))!=-1)
    {
      switch (i)
        {
        case '?':
          cerr << "Invalid option " << (char)optopt << "\n";
        case 'h':
          help=1;
          break;
        case 'd':
          debug_mode = 1;
          break;
        case 'w':
          copyfile_no_api = 1;
          break;
        case 's':
        case 'r':
          int flav; char *cp;

          for (cp = optarg, flav = 0; *cp != '\0' && *cp != '?'; cp++)
            {
              switch(toupper(*cp))
                {
                case 'N':
                case 'F': flav |= (1 << FL_NORMAL);    continue;
                case 'H': flav |= (1 << FL_HOLD);      continue;
                case 'D': flav |= (1 << FL_DIRECT);    continue;
                case 'I': flav |= (1 << FL_IMMEDIATE); continue;
                case 'C': flav |= (1 << FL_CRASH);     continue;
                }
              if (*cp == '?')
                {
                  flav = -1; break;
                }
              cerr << "Invalid flavour argument string " <<
                            optarg << "\n";
              help = 1;
              break;
            }

          if (i == 's')
            sendFlavour = flav;
          else
            receiveFlavour = flav;
          break;

        case 'c':
          cpConfigfile=optarg;
          break;
        }
    }
  while (optind<argc)
    {
      static int n=0;

      switch (n)
        {
        case 0:                 // first parameter
          cpUplink=argv[optind];
          break;
        case 1:
          cpDownlink=argv[optind];
          break;
        default:
          cerr << "Invalid Argument: " << argv[optind] << "\n";
          help=1;
        }
      optind++; n++;
    }

  if (help)
    {
      cerr << PROGRAMID << "\n" <<
        "Usage: diskpoll [-c config] [-s flav] [-r flav] [-d] " <<
#if defined(__OS2__) || defined(__NT__)
        "[-w] " <<
#endif
        "[uplink] [downlink]\n" <<
        "          config:     config file name, default: " << DEFCONFIGFILE << "\n" <<
        "          flav:       defaults to \"?\" (all flavours)\n"<<
        "          uplink:     defaults to \"Uplink\"\n" <<
        "          downlink:   defaults to \"Downlink\"\n" <<
        "          -d          means \"enable debug mode\"\n"
#if defined(__OS2__) || defined(__NT__)
        << "          -w          enable workaround for broken operating system copy API\n"
#endif
        ;

    }
  else
    {
      logmsg(LOGMSG,(char *)NULL);
      logmsg(LOGMSG,"Begin: %s",PROGRAMID);
      doCall(cpConfigfile,cpUplink,cpDownlink, sendFlavour, receiveFlavour);
      logmsg(LOGMSG,"End: %s",PROGRAMID);
    }
}
