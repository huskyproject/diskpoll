#include "words.h"
#include "cstring.h"
#include <stdlib.h>             // getenv
#include <ctype.h>              // isspace

static CString env_expand(const CString& rawString)
{
  CString strRetVal=NULLSTRING;
  CString strVar=NULLSTRING;
  
  int isVar=0;

  const char *cpsrc=rawString;
  char *cpvalue;

  if (cpsrc==NULL)
    return NULLSTRING;

  while (*cpsrc)
    {
      if (*cpsrc=='%')
        if (isVar)
          { 
            if (strVar.Length()==0)   // %% -> %
              strRetVal+='%';
            else
              if ((cpvalue=getenv(strVar))!=NULL)
                for (;*cpvalue;cpvalue++)
                  strRetVal+=*cpvalue;
            isVar=0;
          }
        else
          {
            isVar=1;
            strVar=NULLSTRING;
          }
      else
        if (isVar)
          strVar+=*cpsrc;
        else
          strRetVal+=*cpsrc;
      cpsrc++;
    }

  if (isVar)
    {
      isVar=0;
      strRetVal+='%';
      strRetVal+=strVar;
    }

  return strRetVal;
}

static CString strip_comments (const CString& rawString)
{
  unsigned long termPos;

  for (termPos=0;termPos<rawString.Length();termPos++)
    if (rawString[termPos]==';')
      {
        if (termPos==0)
          break;
        if (termPos>0)
          if (isspace(rawString[termPos-1]))
            break;
      }

  if (termPos>0)
    return rawString.substr(0,termPos-1);
  else
    return NULLSTRING;
}

TWords prepareConfigString(const CString& rawString)
{
  return TWords(env_expand(strip_comments(rawString)));
}

  

  
  



