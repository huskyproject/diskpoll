#ifdef INCS_NEED_DOT_H
#include <assert.h>
#include <stdlib.h>
#else
#include <assert>
#include <stdlib>
#endif

                                // included by carray.h - anders gehts wohl
                                // nicht 

template <class K>
CArray<K>::CArray()
{
  ulMaxSize=0; ulSize=0; ulStep=1;
  pArray=0;
}

template <class K>
CArray<K>::CArray(unsigned long startSize, unsigned long _step)
{
  ulMaxSize=0; ulSize=0; pArray=0; ulStep=startSize;
  _larger();
  ulStep=_step;
}

template <class K>
CArray<K>::~CArray()
{
  if (pArray)
    {
      delete[] pArray;
    }
}

template <class K>
void CArray<K>::_larger(void)
{
  K *pNewArray;
  unsigned long i;
  
  pNewArray = new K[ulMaxSize+ulStep];
  assert(pNewArray!=0);
  for (i=0;i<ulMaxSize;i++)
    {
      pNewArray[i]=pArray[i];
    }
  if (pArray)
    delete[] pArray;
  pArray=pNewArray;
  ulMaxSize+=ulStep;
}

template <class K>
unsigned long CArray<K>::Add(const K &k)
{
  assert (ulSize<=ulMaxSize);
  if (ulSize==ulMaxSize)
    _larger();
  pArray[ulSize++]=k;
  return ulSize-1;
}

template <class K>
unsigned long CArray<K>::Size() const
{
  return ulSize;    
}

template <class K>
CArray<K>& CArray<K>::operator=(const CArray<K> &k)
{
  unsigned long ul;
  
  if (pArray) delete[] pArray;

  ulMaxSize=ulSize=0; pArray=0;
  ulStep=k.Size();
  _larger();

  assert(ulMaxSize==k.Size());
  ulSize=ulMaxSize;
  ulStep=k.ulStep;

  for (ul=0;ul<ulSize;ul++)
    pArray[ul]=k[ul];

  return *this;
}

template <class K>
K& CArray<K>::operator[] (const unsigned long index)
{
  CheckUpperBound(index+1,ulSize,"CArray::operator[]");
  return pArray[index];
}

template <class K>
const K& CArray<K>::operator[] (const unsigned long index) const
{
  CheckUpperBound(index+1,ulSize,"CArray::operator[]");
  return pArray[index];
}


template <class K>
void CArray<K>::Sort(int (*pf)(const void *, const void*))
{
  qsort(pArray,Size(),sizeof(K),pf);
}
  


