#ifndef __CSYSTEM_H
#define __CSYSTEM_H

#include "carray.h"
#include "cnode.h"
#include "cinbound.h"
#include "coutb.h"


                                /* the CSystem class contains everything that
                                   is necessary to define a logical Fidonet
                                   Node System for the purposes of a disk poll
                                   */

class CSystem
{
private:
  CArray<CNode>   *pAkas;
  CInbound        *pInbound;
  COutbound       *pOutbound;
  CString          strName;     /* just an identifier for logging purposes */
  CString          strMailFlag;

public:
  CSystem();
  CSystem(COutbound*, CInbound*, CArray<CNode>*, const CString& = NULLSTRING,
          const CString& = NULLSTRING);
  ~CSystem();
  CSystem(const CSystem&);
  CSystem& operator=(const CSystem&);

  friend istream& operator >> (istream&,CSystem&);

  void setInbound (CInbound*);
  void setOutbound(COutbound*);
  void setAkas(CArray<CNode>*);
  void setMailFlag (const CString&);

  const CString& getName() const;
  const CString& getMailFlag() const;

  void poll(CSystem&, int = -1, int = -1);
};

  
  
  
  
  


#endif
