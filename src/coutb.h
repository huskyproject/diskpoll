#ifndef __COUTB_H
#define __COUTB_H

#include "carray.h"
#include "cnode.h"

enum { KILL_NOKILL=0, KILL_TRUNC=1, KILL_KILL=2 };
enum { FL_FIRST=0, FL_NORMAL=0, FL_HOLD=1, FL_DIRECT=2, FL_CRASH=3,
       FL_IMMEDIATE=4, FL_LAST=4 };


class CSendFile
{
private:
  CString  strFilename;
  int      flavour;
  int      kill;

public:
  CSendFile();
  CSendFile(const CString&);
  CSendFile(const CString&,int,int);
  CSendFile(const CSendFile&);

  ~CSendFile();

  const CString& Filename() const;
  int            Flavour()   const;
  int            Killflag()       const;
};

class COutbound
{
private:
  void     _stripSlash(void);

protected:
  CString strBasepath;
  
public:
  COutbound();
  COutbound(const CString& strBasepath);
  virtual ~COutbound();

  void                       configureBasepath(const CString& strBasepath);
  virtual CArray<CSendFile>* getFilesFor      (const CNode&,
                                               int flavour=-1) = 0;
  virtual unsigned long      sendFilesTo      (const CNode&,
                                               CArray<CSendFile>*) =0;

  virtual int                removeFilesFor   (const CNode&,
                                               CArray<CSendFile>*) =0;
  virtual int                lockNode   (const CNode&) = 0;
  virtual void               unlockNode (const CNode&) = 0;
};

#endif






  

        
