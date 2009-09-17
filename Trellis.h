#ifndef _TRELLIS_H_
#define _TRELLIS_H_

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "TagNode.h"
using namespace std;


typedef vector<vector<TAGNODE> > vvTAGNODE;
extern const string SENTENCE_START         ;
extern const string SENTENCE_END           ;
extern string p_dataDir;

extern string WORDTAGTUPLEFILE     ;
extern string TAGWORDTUPLEFILE    ;
extern string CONTEXTTAGTUPLEFILE  ;
extern  string BIGRAMTUPLEFILE      ;
extern  string TRIGRAMTUPLEFILE     ;
extern  string FOURGRAMTUPLEFILE     ;
extern  string PREFIXBIGRAMTUPLEFILE  ;
extern  string PREFIXTRIGRAMTUPLEFILE;
extern  string WORDDICTFILE           ;
extern  string TAGDICTFILE           ;
extern  string CONTEXTDICTFILE        ;
extern  string USEDCONTEXTFILE        ;
extern  string WORDTAGLEXICONDICTFILE ;

void tagger_loadfadd(string wordTagTupleFile,
		     string tagWordTupleFile,
		     string contextTagTupleFile,
		     string bigramTupleFile,
		     string trigramTupleFile,
		     string fourgramTupleFile,
		     string prefixBigramTupleFile,
		     string prefixTrigramTupleFile,
		     string wordDictFile,
		     string tagDictFile,
		     string contextDictFile,
		     string wordTagLexiconDictFile);
void tagger_close();
double transitionProbability(vector<string> tagSequence, 
			     vector<string> contextSequence,
			     vector<string> words,
                             int transitionType,int index = 0);

void tagger_loadContextLabels(string usedContextFileName);

class TRELLIS 
{
 public:
  
  TRELLIS(vector<string> words,vvTAGNODE trellis,string startTag,string endTag);

  ~TRELLIS();

  int getTagCount()  
    {return tagCount;}

  int getWordCount() 
    {return wordCount;}
  
  void computeForwardProbabilities(); 
  void computeBackwardProbabilities();
  void computeForwardBackwardValues();
  void computeBaselineValues();
  void rankTags();  
  void keepNBestTags(int n);
  void keepPercentageOfTags(int p);
  void keepGoodTags(double margin,map<int,int>& gold,int& right,int& sum); 
  void StoreTags(double margin,map<int,int> & gold,int& right,int& sum); 
  int  collectGoodTags(int *remainingTags,
                       int *remainingTagScores,
                       int debug);
  int  compareWithCorrect(vector<pair<string,pair<int,int> > > corrTags, int debug);
  void countWordsAndTags(int onlyNonDeleted);
  void print();
  void expandTags();
  void reduceTags();
  void printTagging(fstream & fresult);
	void Combine();
	void reduceTags_NO_Context();

 private:
  
  vector<string> 
    words;
  
  vvTAGNODE 
    trellis;

  int 
    tagCount,
    wordCount;
};
#endif