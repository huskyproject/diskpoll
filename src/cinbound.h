#ifndef __CINBOUND_H
#define __CINBOUND_H

#include "coutb.h"              /* killflag */

#define RECERR_NOERROR   0
#define RECERR_NOTEXIST -1  /* file doesn't exist in uplinks directory  */
#define RECERR_OTHER    -2  /* other error, disk full or anything */

class CInbound
{
private:
  CString strPath;
  void makevalidpath();

public:
  CInbound();
  CInbound(const CString&);

  int Receive(const CString& strFilename, int killflag=KILL_NOKILL);
};
#endif
