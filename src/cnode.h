#ifndef __NODE_H
#define __NODE_H

#ifdef INCS_NEED_DOT_H
#include <iostream.h>
#else
#include <iostream>
#endif
#include "cstring.h"

class CNode
{

private:
  long    zone,net,node,point;
  CString strDomain;
  char *cpHumanReadableNotation;
  int makeFromString(const CString&);
  
public:
  CNode(const long zone, const long net, const long node, const long point,
        const CString& strDomain);
  CNode(const CString&);
  CNode();
  CNode(const CNode&);
  ~CNode();

  CNode& operator=(const CNode&);

  long            Zone  (void) const { return zone;      }
  long            Net   (void) const { return net;       }
  long            Node  (void) const { return node;      }
  long            Point (void) const { return point;     }
  const CString&  Domain(void) const { return strDomain; }

  long&     Zone  (void) { return zone;      }
  long&     Net   (void) { return net;       }
  long&     Node  (void) { return node;      } 
  long&     Point (void) { return point;     }
  CString&  Domain(void) { return strDomain; }

  const char *str();
  
  friend ostream& operator << (ostream&, const CNode&);
  friend istream& operator >> (istream&, CNode&);

};

#define LOGNODE(x) ((x).str())

#endif
