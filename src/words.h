#ifndef __words_hpp
#define __words_hpp

class TWords
{
private:
  char **cpWords;
  int nWords;
  char *getNextWord(char *);

public:
  TWords(char *);
  TWords(const TWords&);
  ~TWords();

  TWords& operator =(const TWords&);

  char *getWord(int nr);
  int   getNWords(void);
};

#endif
