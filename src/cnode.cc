#ifdef INCS_NEED_DOT_H
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#else
#include <ctype>
#include <stdlib>
#include <string>
#endif
#ifdef SHORTINCLUDEFILENAMES
#ifdef INCS_NEED_DOT_H
#include <strstrea.h>
#else
#include <strstrea>
#endif
#else
#ifdef INCS_NEED_DOT_H
#include <strstream.h>
#else
#include <strstream>
#endif
#endif
#include "cnode.h"
#include "cstring.h"
#include "envdeps.h"

CNode::CNode(const long zone, const long net, const long node, const long
             point, const CString& strDomain):strDomain()
{
  Zone()=zone;
  Node()=node;
  Net()=net;
  Point()=point;
  Domain()=strDomain;
}

CNode::CNode()
{
  Zone()=0;
  Node()=0;
  Net()=0;
  Point()=0;        
  Domain()=NULLSTRING;
  cpHumanReadableNotation=0;
}

CNode::CNode(const CNode& n)
{
  Zone()=n.Zone();
  Node()=n.Node();
  Net()=n.Net();
  Point()=n.Point();
  Domain()=n.Domain();
  if (cpHumanReadableNotation)
    {
      delete[] cpHumanReadableNotation;
      cpHumanReadableNotation=0;
    }
}

CNode::~CNode()
{
  if (cpHumanReadableNotation)
    {
      delete[] cpHumanReadableNotation;
      cpHumanReadableNotation=0;
    }
}

CNode::CNode(const CString& str)
{
  makeFromString(str);
  cpHumanReadableNotation=0;
}

CNode& CNode::operator=(const CNode& n)
{
  Zone()=n.Zone();
  Node()=n.Node();
  Net()=n.Net();
  Point()=n.Point();
  Domain()=n.Domain();
  if (cpHumanReadableNotation)
    {
      delete[] cpHumanReadableNotation;
      cpHumanReadableNotation=0;
    }
  return (*this);
}


static int sid(const char *cp)  // str is digit or space
{
  const char *c;
  for (c=cp;*c;c++)
    if ((!isdigit(*c))&&(!isspace(*c)))
      return 0;
  return 1;
}

int CNode::makeFromString(const CString& str)
{

  CString dummy(str);
  char *cpDummy =dummy;
  char *cpZone  =(char*)NULL;
  char *cpColon =(char*)NULL;
  char *cpNet   =(char*)NULL;
  char *cpSlash =(char*)NULL;
  char *cpNode  =(char*)NULL;
  char *cpPoint =(char*)NULL;
  char *cpPointnumber=(char*)NULL;
  char *cpAt    =(char*)NULL;
  char *cpDomain=(char*)NULL;

  if (cpDummy==NULL) goto error;

  cpColon=strchr(cpDummy,':');
  cpSlash=strchr(cpDummy,'/');
  cpPoint=strchr(cpDummy,'.');
  cpAt=strchr(cpDummy,'@');

  if (cpColon==NULL||cpSlash==NULL)
    goto error;                 // 3D required at least

  if (!( cpColon<cpSlash && (cpSlash<cpAt||cpAt==NULL)
         && (cpSlash<cpPoint||cpPoint==NULL)))
    goto error;

  cpZone=cpDummy;  *cpColon=0; 
  cpNet=cpColon+1; *cpSlash=0;
  cpNode=cpSlash+1;

  if (cpPoint!=NULL&&(cpAt==NULL||cpPoint<cpAt))
    {
                   *cpPoint=0;
      cpPointnumber=cpPoint+1;
    }
  else
    cpPointnumber=(char*)NULL;

  if (cpAt!=NULL)
    {
      *cpAt=0;
      cpDomain=cpAt+1;
    }
  else
    cpDomain=(char*)NULL;

  if (!(sid(cpZone)&&sid(cpNet)&&sid(cpNode)))
    goto error;
  if (cpPointnumber)
    if (!sid(cpPointnumber))
      goto error;

  Zone()=atoi(cpZone);
  Net()=atoi(cpNet);
  Node()=atoi(cpNode);
  if (cpPointnumber)
    Point()=atoi(cpPointnumber);
  else
    Point()=0;
  if (cpDomain)
    Domain()=cpDomain;
  else
    Domain()=NULLSTRING;

  return 1;

error:
  Zone()=Net()=Node()=0;
  Domain()=NULLSTRING;
  return 0;
}
  
ostream& operator << (ostream& o, const CNode& n)
{
  o << n.Zone() << ":" << n.Net() << "/" << n.Node() << "." << n.Point();
  if (!(CString((char*)NULL)==n.Domain()))
    {
      o << "@" << n.Domain();
    }
  return o;
}

istream& operator >> (istream& i, CNode& n)
{
  CString str;
  char c;
  int eating=1;
    
  while (i)
    {
      c=i.get();
      if (isspace(c)||iscntrl(c))
        {
          if (!eating)
            {
              i.putback(c);
              break;
            }
        }
      else
        {
          eating=0;
          str+=c;
        }
    }


  if (!n.makeFromString(str))
    i.setstate(ios::failbit);
  
  return i;
}

const char *CNode::str()
{
  size_t length=1;
  if (!(Domain()==NULLSTRING))
    length+=Domain().Length();
  length+=4;                    // :/.@
  length+=4*32;                 // space for 4 64-bit integers
  length++;

  if (cpHumanReadableNotation)
    delete[] cpHumanReadableNotation;

  cpHumanReadableNotation=new char[length];
  memset(cpHumanReadableNotation,0,length);
  strstream stringstream(cpHumanReadableNotation,length-1);
  stringstream << (*this);

  return cpHumanReadableNotation;
}
