#include "findfile.h"
#include "envdeps.h"

#include <sys/types.h>
#include <dirent.h>             
#include <fnmatch.h>

CArray<CString>* findfile(const CString& mask)
{
  size_t index;
  CString strDir,strMask;
                                // find the corresponding directory name
  for (index=mask.Length();index>0;index--)
    if (mask[index-1]=='\\'||mask[index-1]=='/'||mask[index-1]==':')
      break;

  if (!index)                   // no directory information
    strDir=".";
  else
    {
      strDir=mask.substr(0,index-1);
      if (strDir[index-1]==':')
        strDir+=".";
      else
        if (index>=2) 
          strDir=mask.substr(0,index-2);
        else
          strDir="";
    }
  if (index<mask.Length())
    strMask=mask.substr(index,mask.Length()-1);
  else
    strMask="*";

  DIR *hDir=opendir(strDir);

  strDir+=DEFDIRSEP;

  if (hDir==NULL)
    return NULL;

  struct dirent* dirent;
  CArray<CString> *pArray=new CArray<CString>;

  while ((dirent=readdir(hDir))!=NULL)
    {
      if (!fnmatch(strMask,dirent->d_name,FNM_FLAGS))
        {
          CString s(strDir);
          s+=dirent->d_name;
          pArray->Add(s);
        }
    }

  return pArray;
}
