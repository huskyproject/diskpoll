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
    if (mask.charAt(index-1)=='\\'||mask.charAt(index-1)=='/'||
        mask.charAt(index-1)==':')
      break;

  if (!index)                   // no directory information
    strDir=".";
  else
    {
      strDir=mask.substr(0,index-1);
      if (strDir.charAt(index-1)==':')
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

  struct dirent* dirent;
  CArray<CString> *dirs;

  if (strDir.Length()>2 ||
      (strDir.Length() == 2 && strDir.charAt(1) != ':') ||
      (strDir.Length() == 1 && strDir.charAt(0) != '.'))
    {
      // if we have a non-trivial directory name - SEARCH IT!
      // this is slow, but the only way to get around case sensitivity
      // problems on unix boxes that mount dos ressources ...

      dirs = findfile(strDir);
    }
  else
    {
      dirs = new CArray<CString>;
      dirs->Add(strDir);
    }

  if (dirs == NULL)
    return NULL;

  CArray<CString> *pArray=new CArray<CString>;

  for (unsigned long ndir = 0; ndir < dirs->Size(); ndir++)
    {
      DIR *hDir;
      if ((*dirs)[ndir].Length())
        hDir=opendir((*dirs)[ndir]);
      else
        {
          CString s;
          s+=DEFDIRSEP;
          hDir=opendir(s);
        }

      if (hDir==NULL)
        continue;

      while ((dirent=readdir(hDir))!=NULL)
        {
          if (!fnmatch(strMask,dirent->d_name,FNM_FLAGS))
            {
              CString s((*dirs)[ndir]);
              s+=DEFDIRSEP;
              s+=dirent->d_name;
              pArray->Add(s);
            }
        }
      closedir(hDir);
    }
  delete dirs;
  return pArray;
}

int adaptcase(char *fn)
{
  CArray<CString> *files = findfile(fn);
  unsigned long l = 0;

  if (files == NULL || files->Size() == 0)
    {
      if (files) delete files;
      return 0;
    }

  if (files->Size()>1)
    {
      for (l = 0; l<files->Size(); l++)
        {
          if (!strcmp((*files)[l],fn))
            {
              break;
            }
        }
      if (l==files->Size())
        {
          l = 0;
        }
    }
  //  cerr << (*files)[l] << " " << fn << "\n";
  CheckCond((strlen((*files)[l]) == strlen(fn)),
            "adaptcase used with wildcards");
  strcpy(fn, (*files)[l]);
  return 1;
}




