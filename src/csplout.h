#ifndef __CSPLOUT_H
#define __CSPLOUT_H

#include "coutb.h"

                                /* CSpoolOutbound provides a means to cope with
                                   "spooler" outbound directories like the
                                   fidogate outbound. Programs like fidogate
                                   assume that they only have one uplink and
                                   put all their output in the same directory,
                                   without any control information. */

class CSpoolOutbound: public COutbound
{
public:
  CSpoolOutbound();
  CSpoolOutbound(const CString&);

  virtual CArray<CSendFile>* getFilesFor   (const CNode&, int flavour=-1);
  virtual unsigned long      sendFilesTo   (const CNode&, CArray<CSendFile>*);
  virtual int                removeFilesFor(const CNode&, CArray<CSendFile>*);
  virtual int                lockNode  (const CNode&) { return 1;}
  virtual void               unlockNode(const CNode&) {}
};

#endif
