#include "findfile.h"
#include "envdeps.h"

#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>


// ==================================================================== 
// patmat: a routine for pattern matching
// author: Screenath Chary, Nov 29 1988
// ==================================================================== 

static int patmat(char *raw, char *pat)
{

  size_t i;

  if (*pat == '\0' && *raw == '\0')
    {
      // if it is the end of both strings, then match
      return 1;
    }
  if (*pat == '\0')
    {
      // if it is the end of only pat then mismatch 
      return 0;
    }
  
  // if pattern is `*'
  
  if (*pat == '*')
    {
      if (*(pat + 1) == '\0')
        {
	  // if pat is just `*' then match
	  return 1;
        }
      else
        {
	  // else hunt for match or wild card
	  for (i = 0; i <= strlen(raw); i++)
            {
	      if (tolower(*(raw + i)) == tolower(*(pat + 1)) ||
		  *(pat + 1) == '?')
                {
		  // if found, match rest of pattern 
		  if (patmat(raw + i + 1, pat + 2) == 1)
                    {
		      return 1;
                    }
                }
            }
        }
    }
  else
    {
      if (*raw == '\0')
        {
          // we've reached the end of raw; return a mismatch
	  return 0;
        }
      
      if (*pat == '?' || tolower(*pat) == tolower(*raw))
        {
          // if chars match then try and match rest of it
	  if (patmat(raw + 1, pat + 1) == 1)
            {
	      return 1;
            }
        }
    }
  
  // fell through; no match found
  return 0;
}

#ifdef UNIXLIKE


// ==================================================================== 
// private_adaptcase: a routine for woring with case-insensitive file 
//            lists (like binkley flow files) on case sensitive file 
//     	      systems
// author:    Tobias Ernst, Jan 1999
// ==================================================================== 
  
/* The routine behaves as follows: It assumes that pathname
   is a path name which may contain multiple dashes, and it assumes
   that you run on a case sensitive file system but want / must to
   match the path name insensitively. adaptcase takes every path
   element out of pathname and uses findfirst to check if it
   exists. If it exists, the path element is replaced by the exact
   spelling as used by the file system. If it does not exist, it is
   converted to lowercase.  This allows you to make you program deal
   with things like mounts of DOS file systems under unix

   Return value is 1 if the file exists and 0 if not.

   Attention: Do not ever try to understand this code. I had to do
   heavy caching and other optimizations in this routine in order to
   reduce that startup time of msged to a reasonable value. (The
   problem is that opendir / readdir is very slow ...). If you ever
   have to fix something in this routine, you'd better rewrite it from
.   scratch.
*/

/* the cache will take  about 60 * 4192 + 30 * 512 * 4 bytes in this
   configuration, i.e. 360K */

#define adaptcase_cachesize   60
#define rawcache_stepsize   4192
#define cacheindex_stepsize  512

struct adaptcase_cache_entry
{
    char *query;
    char *result;
    char *raw_cache;
    size_t *cache_index;
    size_t n;
};
static int adaptcase_cache_position = -1;
static struct adaptcase_cache_entry adaptcase_cache[adaptcase_cachesize];

static char *current_cache;
static int cache_sort_cmp(const void *a, const void *b)
{
    return strcasecmp(current_cache+(*((const size_t *)a)),
                      current_cache+(*((const size_t *)b)));
}

static int cache_find_cmp(const void *a, const void *b)
{
    return strcasecmp((const char *)a, current_cache+(*((const size_t *)b)));
}
  
/* #define TRACECACHE */

#ifdef BSD
#define DIRENTLEN(x) ((x)->d_namlen)
#else
#define DIRENTLEN(x) (strlen((x)->d_name))
#endif

static void private_adaptcase(char *pathname)
{
    int i,j,k,l,n,nmax=0, found=1, addresult=0;
    size_t *m; size_t raw_high, rawmax=0;
    char buf[4096];
    DIR *dirp = NULL;
    struct dirent *dp;
    char c;

#ifdef TRACECACHE
    FILE *ftrc;
#endif
    
    if (!*pathname)
        return;
#ifdef TRACECACHE
    ftrc = fopen ("trace.log", "a");
    fprintf(ftrc, "--Query: %s\n", pathname);
#endif    
    
    if (adaptcase_cache_position == -1)
    {
        /* initialise the cache */
        memset(adaptcase_cache, 0, adaptcase_cachesize *
               sizeof(struct adaptcase_cache_entry));
        adaptcase_cache_position = 0;
    }
    
    k = strlen(pathname);
    if (k > 2)
    {
        for (k = k - 2; k>0 && pathname[k] != '/'; k--);
    }
    else
    {
        k = 0;
    }
    
    j = 0; i = 0;


start_over:

    if (k != 0)
    {
        l = adaptcase_cache_position;
        do
        {
            if (adaptcase_cache[l].query != NULL)
            {
                if ((!memcmp(adaptcase_cache[l].query,pathname,k)) &&
                    (adaptcase_cache[l].query[k] == '\0'))                                                  
                {
                    /* cache hit for the directory */
#ifdef TRACECACHE
                    fprintf (ftrc, "Cache hit for Dir: %s\n",
                             adaptcase_cache[l].result);
#endif                   
                    memcpy(buf, adaptcase_cache[l].result, k);
                    buf[k] = '/';
                    current_cache=adaptcase_cache[l].raw_cache;
                    m = (size_t *) bsearch(pathname + k + 1,
					   adaptcase_cache[l].cache_index,
					   adaptcase_cache[l].n,
					   sizeof(size_t),
					   cache_find_cmp);
                    if (m == 0)
                    {
#ifdef TRACECACHE
                        fprintf (ftrc, "Cache miss for file.\n");
#endif                   

                        /* file does not exist - convert to lower c. */
                        for (n = k + 1; pathname[n-1]; n++)
                        {
                            buf[n] = tolower(pathname[n]);
                        }
                        memcpy(pathname, buf, n-1);
#ifdef TRACECACHE
                        fprintf(ftrc, "Return: %s\n", pathname);
                        fclose(ftrc);
#endif    
                        return;
                    }
                    else
                    {
#ifdef TRACECACHE
                        fprintf (ftrc, "Cache hit for file: %s\n",
                                 adaptcase_cache[l].raw_cache+(*m));
#endif                   

                        /* file does exist = cache hit for the file */
                        for (n = k + 1; pathname[n-1]; n++)
                        {
                            buf[n] =
                                adaptcase_cache[l].raw_cache[(*m) + n - k - 1];
                        }
                        assert(buf[n-1] == '\0');
                        memcpy(pathname, buf, n-1);
#ifdef TRACECACHE
                        fprintf(ftrc, "Return: %s\n", pathname);
                        fclose(ftrc);
#endif
                        return;
                    }
                }
            }
            l = (l == 0) ? adaptcase_cachesize - 1 : l - 1;
        } while (l != adaptcase_cache_position);

#ifdef TRACECACHE
        fprintf (ftrc, "Cache miss for directory.\n");
#endif                   


        /* no hit for the directory */
        addresult = 1;
    }

               
    while (pathname[i])
    {
        if (pathname[i] == '/')
        {
            buf[i] = pathname[i]; 
            if (addresult && i == k)
            {
                goto add_to_cache;
            }
cache_failure:
            i++;
            buf[i]='\0';
            dirp = opendir(buf);
#ifdef TRACECACHE
            if (dirp == NULL)
            {
                fprintf (ftrc, "Error opening directory %s\n", buf);
            }
#endif            
        }
        else
        {
            assert(i==0);
            dirp = opendir("./");
#ifdef TRACECACHE
            if (dirp == NULL)
            {
                fprintf (ftrc, "Error opening directory ./\n");
            }
#endif            
        }

        j = i;
        for (; pathname[i] && pathname[i]!='/'; i++)
            buf[i] = pathname[i];
        buf[i] = '\0';
        found = 0;

        if (dirp != NULL)
        {
            while ((dp = readdir(dirp)) != NULL)
            {
                if (!strcasecmp(dp->d_name, buf + j))
                {
                    /* file exists, take over it's name */
            
                    assert((size_t)(i - j) == DIRENTLEN(dp));
                    memcpy(buf + j, dp->d_name, DIRENTLEN(dp) + 1);
                    closedir(dirp);
                    dirp = NULL;
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
        {
            /* file does not exist - so the rest is brand new and
               should be converted to lower case */
            
            for (i = j; pathname[i]; i++)
                buf[i] = tolower(pathname[i]);
            buf[i] = '\0';
            if (dirp != NULL)
            {
                closedir(dirp);
            }
            dirp = NULL;
            break;
        }
    }
    assert(strlen(pathname) == strlen(buf));

add_to_cache:
    while (addresult)
    {
        l = adaptcase_cache_position;
        l = (l == adaptcase_cachesize - 1) ? 0 : l + 1;

        if (adaptcase_cache[l].query != NULL)
        {
            free(adaptcase_cache[l].query);
            adaptcase_cache[l].query = NULL;
        }
        if (adaptcase_cache[l].result != NULL)
        {
            free(adaptcase_cache[l].result);
            adaptcase_cache[l].result = NULL;
        }
        if (adaptcase_cache[l].raw_cache != NULL)
        {
            free(adaptcase_cache[l].raw_cache);
            adaptcase_cache[l].raw_cache = NULL;
        }
        if ( (adaptcase_cache[l].query     = (char *) malloc(k + 1)) == NULL ||
             (adaptcase_cache[l].result    = (char *) malloc(k + 1)) == NULL ||
             (adaptcase_cache[l].raw_cache = (char *) malloc(rawmax = rawcache_stepsize)) == NULL ||
             (adaptcase_cache[l].cache_index = (size_t *) malloc((nmax = cacheindex_stepsize) * sizeof(size_t))) == NULL )
        {
            goto cache_error;
        }

        adaptcase_cache[l].n = 0;
        raw_high = 0;
        
        c = buf[k]; buf[k] = '\0';
        if ((dirp = opendir(buf)) == NULL)
        {
            buf[k] = c;
            goto cache_error;
        }
        buf[k] = c;

        while ((dp = readdir(dirp)) != NULL)
        {
            if (raw_high + DIRENTLEN(dp) + 1 > rawmax)
            {
                if ((adaptcase_cache[l].raw_cache =
                     (char *) realloc(adaptcase_cache[l].raw_cache,
                                      rawmax+=rawcache_stepsize)) == NULL)
                {
                    goto cache_error;
                }
            }
           
            if (adaptcase_cache[l].n == (size_t) nmax - 1)
            {
                if ((adaptcase_cache[l].cache_index = (size_t *)
                     realloc(adaptcase_cache[l].cache_index,
                             (nmax+=cacheindex_stepsize) *
                             sizeof(size_t))) == NULL)
                {
                    goto cache_error;
                }
            }

            memcpy (adaptcase_cache[l].raw_cache + raw_high,
                    dp->d_name, DIRENTLEN(dp) + 1);
            adaptcase_cache[l].cache_index[adaptcase_cache[l].n++] = raw_high;
            raw_high += DIRENTLEN(dp) + 1;
        }
        closedir(dirp);
        current_cache=adaptcase_cache[l].raw_cache;
        qsort(adaptcase_cache[l].cache_index, adaptcase_cache[l].n,
              sizeof(size_t), cache_sort_cmp);

        memcpy(adaptcase_cache[l].query, pathname, k);
        adaptcase_cache[l].query[k] = '\0';
        memcpy(adaptcase_cache[l].result, buf, k);
        adaptcase_cache[l].result[k] = '\0';
        
        adaptcase_cache_position = l;

#ifdef TRACECACHE
        fprintf  (ftrc, "Sucessfully added cache entry.\n");
#endif        
        goto start_over;

    cache_error:
        if (adaptcase_cache[l].query != NULL)
        {
            free(adaptcase_cache[l].query);
            adaptcase_cache[l].query = NULL;
        }
        if (adaptcase_cache[l].result != NULL)
        {
            free(adaptcase_cache[l].result);
            adaptcase_cache[l].result = NULL;
        }
        if (adaptcase_cache[l].raw_cache != NULL)
        {
            free(adaptcase_cache[l].raw_cache);
            adaptcase_cache[l].raw_cache = NULL;
        }
        if (adaptcase_cache[l].cache_index != NULL)
        {
            free(adaptcase_cache[l].cache_index);
            adaptcase_cache[l].cache_index = NULL;
        }
        if (dirp != NULL)
        {
            closedir(dirp);
        }
#ifdef TRACECACHE
        fprintf  (ftrc, "Error in building cache entry.\n");
#endif        
        addresult = 0;
        goto cache_failure;
    }
        
#ifdef TRACECACHE
    fprintf(ftrc, "Return: %s\n", pathname);
    fclose(ftrc);
#endif
    strcpy(pathname, buf);
    return;
}
#endif  


// ==================================================================== 
// This is the main routine of this module ... findfile
// It finds files that match patterns. By using private_adaptcase,
// it works fully case insensitive even on Unix.
// ==================================================================== 

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

  /*  if (strDir.Length()>2 ||
      (strDir.Length() == 2 && strDir.charAt(1) != ':') ||
      (strDir.Length() == 1 && strDir.charAt(0) != '.'))
    {
      // if we have a non-trivial directory name - SEARCH IT!
      // this is slow, but the only way to get around case sensitivity
      // problems on unix boxes that mount dos ressources ...

      dirs = findfile(strDir);
    }
  else */
    {
      dirs = new CArray<CString>;
#ifdef UNIXLIKE
      private_adaptcase((char*)strDir);
#endif
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
	  if (patmat(dirent->d_name, strMask))
	    //          if (!fnmatch(strMask,dirent->d_name,FNM_FLAGS))
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



// ==================================================================== 
// adaptcase: Returns 1 if the file exists, and 0 if not. On Unix, 
//            the string that the fn parameter points to will be ad-
//            justed so that it exactly matches the spelling of the
//            file that has been found.
// ==================================================================== 


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

  CheckCond((strlen((*files)[l]) == strlen(fn)),
            "adaptcase used with wildcards");
  strcpy(fn, (*files)[l]);
  return 1;
}




