#ifndef __CBINKOUT_H
#define __CBINKOUT_H

#include "cstring.h"
#include "coutb.h"
#include "cstring.h"

#define FL_FILEMASK FL_LAST+1

class CBinkleyOutbound: public COutbound
{
protected:
  long baseZone;
  char outDirSep;
  CArray<CString> *pFloFilePrefixes;
  CArray<CString> *pLocFilePrefixes;

  virtual CString _packetExtension  (int flavour);
  virtual CString _flowfileExtension(int flavour);
  virtual CString _nodebaseName     (const CNode&);
public:
  virtual CString importFilenameFromFlowfile(const CString&);
  virtual CString exportFilenameToFlowfile  (const CString&);
public:
  virtual CString _flowfileName     (const CNode&, int flavour=FL_FILEMASK);
  virtual CString _packetfileName   (const CNode&, int flavour=FL_FILEMASK);
public:
  CBinkleyOutbound();
  CBinkleyOutbound(const CString& strBasepath, long ownZone=2);
  virtual ~CBinkleyOutbound();

  virtual void setLanReplaceCharacteristics (CArray<CString>*,
                                             CArray<CString>*,
                                             char);

  virtual CArray<CSendFile>* getFilesFor    (const CNode&, int flavour=-1);
  virtual unsigned long      sendFilesTo    (const CNode&,
                                             CArray<CSendFile>*);
  virtual int                removeFilesFor (const CNode&,
                                             CArray<CSendFile>*);
  
  virtual int                lockNode(const CNode&);
  virtual void               unlockNode(const CNode&);
};

#endif
