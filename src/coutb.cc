#include "coutb.h"

CSendFile::CSendFile():strFilename()
{
  flavour=FL_NORMAL; kill=KILL_NOKILL;
}

CSendFile::CSendFile(const CString& str)
  :strFilename(str)
{
  flavour=FL_NORMAL; kill=KILL_NOKILL;
}

CSendFile::CSendFile(const CString& str, int f, int k)
  :strFilename(str),flavour(f),kill(k)
{}

CSendFile::CSendFile(const CSendFile& csf)
{
  strFilename=csf.Filename();
  flavour=csf.Flavour();
  kill=csf.Killflag();
}


CSendFile::~CSendFile()
{}
  
  
const CString& CSendFile::Filename() const
{
  return strFilename;
}

int CSendFile::Flavour() const
{
  return flavour;
}

int CSendFile::Killflag() const
{
  return kill;
}

COutbound::COutbound():strBasepath()
{
  _stripSlash();
}

COutbound::COutbound(const CString& str):strBasepath(str)
{
  _stripSlash();
}

COutbound::~COutbound()
{
 _stripSlash();
}

void COutbound::_stripSlash()
{
  CString str;
  
  if (strBasepath.Length())
    {
      char& c=strBasepath.charAt(strBasepath.Length()-1);
      if (c=='\\'||c=='/')
        c='\0';
      if (c==':')
        strBasepath+=CString(".");
      str=((const char *)(strBasepath));
      strBasepath=str;
    }
}


  

