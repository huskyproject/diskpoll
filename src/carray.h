#ifndef __CARRAY_H
#define __CARRAY_H

#include "cerror.h"

template <class K>
class CArray
{
public:
  CArray();
  CArray(unsigned long startSize, unsigned long _step);
  CArray(const CArray<K> &);
  ~CArray();

  unsigned long     Add (const K &);
  unsigned long     Size(void) const;

  void              Sort(int (*pf)(const void*, const void*));

  CArray<K>&        operator=  (const CArray<K> &);
  K&                operator[] (const unsigned long index);
  const K&          operator[] (const unsigned long index) const;
  
private:
  void           _larger(void);
  unsigned long  ulStep,ulMaxSize,ulSize;
  K* pArray;
};

#include "carray.cc"

#endif
