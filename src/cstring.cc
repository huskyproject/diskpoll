#include <ctype.h>
#include "cerror.h"
#include "cstring.h"

CString::CString()
{ buffer=0; length=alloc_len=0;
}

CString::CString(const CString &s)
{ if ((const char *)s!=0)
  {
    CheckPointer(buffer=new char[(length=alloc_len=s.Length())+1],
                 "CString::copy constructor::buffer allocation");
    strcpy (buffer,(const char *)s);
  }
  else
  {
    buffer=0; length=alloc_len=0;
  }
}

CString::CString(const char *cp)
{
  if (cp!=0)
    {
      CheckPointer(buffer=new char[(length=alloc_len=strlen(cp))+1],
                   "CString::const char * constructor::buffer allocation");
      strcpy(buffer,cp);
    }
  else
    {
      buffer=0;
      length=alloc_len=0;
    }
}

CString::~CString()
{ if (buffer) delete [] buffer;
  buffer=0; length=alloc_len=0;
}

CString::operator char*()
{ return buffer;
}

CString::operator const char*() const
{ return buffer;
}

CString CString::substr(size_t i,size_t j) const
{
  char *cp,c;

  CheckUpperBound(i+1,length,"CString::substr()");
  CheckUpperBound(j+1,length,"CString::substr()");
  if (i>j)
    {
      int k;
      k=j; j=i; i=k;
    }
  cp=buffer+i; c=buffer[j+1]; buffer[j+1]='\0';

  CString retval(cp);

  buffer[j+1]=c;

  return retval;
}


CString& CString::operator +=(const CString &s)
{ char *cp=new char[(length=alloc_len=length+s.Length())+1];
  CheckPointer(cp,"CString::operator +=::memory allocation");
  *cp=0; if (buffer) strcpy(cp,buffer);
  if ((const char *)s) strcat(cp,(const char *)s);
  if (buffer) delete [] buffer;
  buffer = cp;
  return (*this);
}

CString& CString::operator +=(const char &c)
{
  char *cp;

  if (length>=alloc_len)
    {
      cp=new char[(alloc_len=length+64)];
      *cp=0;
      if (buffer)
        {
          strcpy(cp,buffer);
          delete[] buffer;
        }
    }
  else
    {
      cp=buffer;
    }
  cp[length]=c; cp[++length]='\0';
  buffer=cp;

  return (*this);
}


CString& CString::operator =(const CString &s)
{ if (buffer) delete[] buffer;
  if ((const char *)s!=0)
    {
      buffer=new char[(length=alloc_len=s.Length())+1];
      CheckPointer(buffer,"CString::operator =::memory allocation");
      strcpy(buffer,(const char*)s);
    }
  else
    {
      buffer=0;
      length=alloc_len=0;
    }
  return (*this);
}




/* Due to bad class design in my code, the following does not work
   any more with GCC 2.8 

char& CString::operator[](const size_t index)
{ CheckUpperBound(index,length,"CString::operator[]");
  return *(buffer+index);
}

char& CString::operator[](const size_t index) const
{ CheckUpperBound(index,length,"CString::operator[]");
  return *(buffer+index);
}

It is replaced with the following:
*/

char& CString::charAt(const size_t index) const
{ CheckUpperBound(index,length,"CString::operator[]");
  return *(buffer+index);
}

char& CString::setCharAt(const size_t index, const char& c)
{ CheckUpperBound(index,length,"CString::operator[]");
  return *(buffer+index) = c;
}

int CString::operator== (const CString &s) const
{ return ((*this)==(const char*)s);
}

int CString::operator== (const char *s) const
{ if (s==0||buffer==0)
    return s==buffer;
  if (strlen(s)==length)
    return !strcmp((const char*)s,buffer);
  else return 0;
}

int CString::operator== (char *s) const
{ if (s==0||buffer==0)
    return s==buffer;
  if (strlen(s)==length)
    return !strcmp((const char*)s,buffer);
  else return 0;
}



CString operator + (const CString &s1, const CString &s2)
{ CString tmp;

  tmp=s1;
  tmp+=s2;
  return tmp;
}

ostream& operator << (ostream& o, const CString&s)
{ o << (const char *)s;
  return o;
}

istream& operator >> (istream& i, CString&s)
{
  char c;
  s="";

  while (i)
    {
      i.get(c);
      if (!i)
        break;
      if (c=='\n'||c=='\r')
        break;
      else
        s+=c;
    }

  return i;
}

size_t CString::Length() const
{ return length;
}


CString upcase(const CString& src)
{
  CString ret=src;
  size_t i;

  for (i=0;i<src.Length();i++)
    ret.setCharAt(i,toupper(ret.charAt(i)));

  return ret;
}

