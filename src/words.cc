#ifdef INCS_NEED_DOT_H
#include <ctype.h>
#include <stdlib.h> /* NULL */
#else
#include <ctype>
#include <stdlib> /* NULL */
#endif
#include "words.h"
#include "cerror.h"

TWords::TWords(char *cpWord)
{ char *cp;
  int i;

  if (cpWord==NULL)
    {
      nWords=0;
    }
  else
    {
      for (cp=cpWord;(*cp)&&(isspace(*cp));cp++);
      if (!(*cp))
        {
          nWords=0;
        }
      else
        {
          nWords=1;
          for (;*cp;)
            {
              if (*(cp=getNextWord(cp)))
                {
                  nWords++; cp++;
                }
            }
        }
    }

  if (nWords>0)
    {
      cpWords=new char*[nWords+1];
      CheckPointer(cpWords,"TWords::TWords()");
      for (cp=cpWord;(*cp)&&(isspace(*cp));cp++);
      for (i=0;*cp;i++,cp=getNextWord(cp+1))
      { char *cp2; int len;
        for (cp2=cp,len=0;(*cp2)&&(!isspace(*cp2));cp2++,len++);
        cpWords[i]=new char[len+1];
        CheckPointer(cpWords[i],"TWords::TWords()");
        for (cp2=cp,len=0;(*cp2)&&(!isspace(*cp2));cp2++,len++)
        { cpWords[i][len]=*cp2;
        }
        cpWords[i][len]='\0';
      }
      CheckCond(i==nWords,"TWords algorithm error");
   } else cpWords=0;
}

TWords::~TWords()
{ int i;
  for (i=0;i<nWords;i++) delete[] cpWords[i];
  if (cpWords) delete cpWords;
}


char *TWords::getWord(int nr)
{ if (nr<getNWords())
    return cpWords[nr];
  else return 0;
}

int TWords::getNWords(void)
{ return nWords;
}


char *TWords::getNextWord(char *cpWord)
{ char *cp=cpWord;
  for (;(*cp)&&(!isspace(*cp));cp++);
  for (;(*cp)&&(isspace(*cp));cp++);
  return cp;
}

TWords::TWords(const TWords&r)
{ int i;
  nWords=r.nWords;
  if (nWords)
    {
      cpWords=new char*[nWords+1];
      CheckPointer(cpWords,"TWords::TWords()");
      for (i=0;i<nWords;i++)
        {
          cpWords[i]=new char[strlen(r.cpWords[i])+1];
          CheckPointer(cpWords[i],"TWords::TWords()");
          strcpy(cpWords[i],r.cpWords[i]);
        }
    }
  else
    cpWords=0;
}

TWords& TWords::operator=(const TWords&r)
{ int i;
  for (i=0;i<nWords;i++) delete[] cpWords[i];
  if (cpWords) delete cpWords;
  nWords=r.nWords;
  if (nWords)
    {
      cpWords=new char*[nWords+1];
      CheckPointer(cpWords,"TWords::operator=()");
      for (i=0;i<nWords;i++)
        {
          cpWords[i]=new char[strlen(r.cpWords[i])+1];
          CheckPointer(cpWords[i],"TWords::operator=()");
          strcpy(cpWords[i],r.cpWords[i]);
        }
    }
  else
    cpWords=0;
  return (*this);
}



