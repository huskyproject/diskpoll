/* This is a test program that can be used to test how the findfile
   function behaves / if it behaves correctly when porting to new
   platforms. Suggested calls to the findfile program:

   findfile "*"
   findfile "*.*"

   If the first call displays all files, including those with file
   name extensions, you should set #define ALLFILES "*" in envdeps.h.
   If only the latter call returns all files, you should set
   #define ALLFILES "*.*" instaed

   findfile "./*"
   findfile ".\*"

   Should both be equivalent.
*/

#include <iostream.h>
#include "envdeps.h"
#include "findfile.h"

int main(int argc, char **argv)
{
  if (argc != 2)
    {
      cerr << "Usage: findtest <filemask>\n";
      return 8;
    }

  CArray<CString>* found = findfile(CString(argv[1]));

  for (int i = 0; i < found->Size(); i++)
    {
      cout << (*found)[i] << '\n';
    }

  delete found;
}

  

  
    
    
    

