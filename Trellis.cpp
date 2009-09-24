#include "Trellis.h"

#include <list>
#include <fstream>
#include <math.h>

#include "global.h"
#include "mwutool.h"
#include "StrHashTrait.h"
#include "fadd.h"

#ifdef linux
#include <ext/hash_map>
#else 
#include <hash_map>
#endif 

#ifdef linux
using namespace __gnu_cxx;
#endif

using namespace std;


const string SENTENCE_START         = "xxx_sentence_start";
const string SENTENCE_END           = "xxx_sentence_end";
//const string UNKNOWN_WORD           = "<UNKNOWN_WORD>";

const int    DEFAULT_TAG_ID         = -1;
const int    CONTEXT_STATESIZE      = 1;

vector<string>
  g_usedContextLabels;

int g_0_right=0 ;
int g_0_sum=0;

int g_1_right=0;
int g_1_sum=0;

int g_2_right=0;
int g_2_sum=0;

int g_3_right=0;
int g_3_sum=0;


vector< vector<double> > g_nor_grid;
vector< vector<double> > g_nor_ME_grid;
int g_maxSenLen = 1000;
int g_maxPosList = 30;
void InitGrid(void )
{
	
	g_nor_grid.resize(g_maxSenLen);
	for(int i = 0;i<g_maxSenLen;i++)
	{
		g_nor_grid[i].resize(g_maxPosList);
	}
	for(int i = 0;i<g_maxSenLen;i++)
	{
		for(int j = 0 ;j<g_maxPosList;j++)
		{
			g_nor_grid[i][j] = 0.0;
		}
	}

	g_nor_ME_grid.resize(g_maxSenLen);
	for(int i = 0;i<g_maxSenLen;i++)
	{
		g_nor_ME_grid[i].resize(g_maxPosList);
	}
	for(int i = 0;i<g_maxSenLen;i++)
	{
		for(int j = 0 ;j<g_maxPosList;j++)
		{
			g_nor_ME_grid[i][j] = 0.0;
		}
	}
}

bool g_hmm = 0;



hash_map<string,double,StrHashTrait<string> > g_hashProb;
hash_map<string,double,StrHashTrait<string> > g_hashContextProb;

///////////////////////////////////////////////////////////////////////////////


const int    INTERPOLATION_LIMIT    = 2; // smallest n-gram in interpolation
double g_diversity = 3;
void destroyWordList(list_of_words *list);
list_of_words *createWordList(vector<string> words);
double ngramProb(list_of_words *tags, double totalLambda, int n, int interpolationLimit, int useContextFadd) ;
void destroyNumberList(list_of_numbers *list);

const long   MAX_NGRAM_VALUE        = 1350;
const long   MAX_WORD_TAG_VALUE     = 1350;
const long   MAX_TAG_WORD_VALUE     = 1350;

string WORDTAGTUPLEFILE       = "wordTag.tpl";
string TAGWORDTUPLEFILE       = "tagWord.tpl";
string CONTEXTTAGTUPLEFILE    = "context3.tpl";
string BIGRAMTUPLEFILE        = "tag2.tpl";
string TRIGRAMTUPLEFILE       = "tag3.tpl";
string FOURGRAMTUPLEFILE      = "tag4.tpl";
string PREFIXBIGRAMTUPLEFILE  = "prefix2.tpl";
string PREFIXTRIGRAMTUPLEFILE = "prefix3.tpl";
string WORDDICTFILE           = "words.fsa";
string TAGDICTFILE            = "tags.fsa";
string CONTEXTDICTFILE        = "context.fsa";
string USEDCONTEXTFILE        = "usedContext";
string WORDTAGLEXICONDICTFILE = "wordTagLex.fsa";

const string HEURISTICSFILE         = "heuristics";

string p_dataDir ="./MODELS/";



extern double g_addLogProbabilities(double value1, double value2);

long int 
  fourgramFaddNumber       = -1,
  trigramFaddNumber        = -1,
  bigramFaddNumber         = -1,
  contextFaddNumber        = -1,
  prefixBigramFaddNumber   = -1,
  prefixTrigramFaddNumber  = -1,
  wordTagFaddNumber        = -1,
  tagWordFaddNumber        = -1,
  wordFaddNumber           = -1,
  wordTagLexiconFaddNumber = -1;

int 
  g_model                  = 3;

double wordTagProb(list_of_words *wordTagList);


double ME_Probability(vector<string> &tagSequence, 
			     vector<string> &contextSequence,
				int index)
{
	char buffer[20];
	string key, temp;
	sprintf(buffer, "%d",index);
	key = buffer;
	double transitionProb;
		
		for(int i = 0;i<tagSequence.size();i++)
		{
			key +="+";
			if(tagSequence[i] =="xxx_sentence_start")
				tagSequence[i] = START_TAG;
			if(tagSequence[i] =="xxx_sentence_end")
				tagSequence[i] = END_TAG;
			key +=tagSequence[i];
		}
		for(int j = 0;j<contextSequence.size();j++)
		{
			key +="+";
			key +=contextSequence[j];
		}
		if(g_hashProb.find(key)!=g_hashProb.end())  //Èç¹ûÕÒµ½
		{
			double prob = g_hashProb[key];
			transitionProb = -log(prob);
		}
		else
		{
			//only occur in the end of sentence. 
			cout<<" I didn't find this string ~!@#$%^&*"<<key<<endl;
			transitionProb	= 0;
			exit(2);
		}
		return transitionProb;
}


double transitionProbability(vector<string> tagSequence, 
			     vector<string> contextSequence,
			     vector<string> words,
                             int transitionType,int index)
  // compute the probability of the transition defined by the
  // supplied tag n-gram, context n-gram and observed word(s)
{
  double
    wordProb,    // observation probability; Wj given Tj
    tagProb,     // tag n-gram probability;  Tj given Ti... and Ci
    contextProb; // context probability;     Cj given Tj    and Ci
  
  string
    destinationTag = tagSequence.back(),
    observedWord   = *words.begin();

  for(vector<string>::iterator vi=words.begin()+1 ; vi!=words.end() ; ++vi){
    observedWord += (" " + *vi);
  }
  
  // create list of words and tag
  list_of_words *wordTagList = new list_of_words;
  
  wordTagList->word       = observedWord.c_str();
  wordTagList->next       = new list_of_words;
  wordTagList->next->word = destinationTag.c_str();
  wordTagList->next->next = NULL;
  
  // get observation probability
  wordProb = wordTagProb(wordTagList);
  
  if(g_debug>2){
    cout << "word-tag:" << wordTagList->word << " ";
    cout << wordTagList->next->word << " " << wordProb << endl;
  }

  destroyWordList(wordTagList);
  
  // create list of tags and context
  vector<string> tagsPlusContext = tagSequence;
  
  tagsPlusContext.pop_back();
  for(vector<string>::iterator s=contextSequence.begin(); s!=contextSequence.end()-1 ; ++s){
    tagsPlusContext.push_back(*s);
  }
  tagsPlusContext.push_back(tagSequence.back());

  list_of_words *tagsPlusContextList = createWordList(tagsPlusContext);
  

  if(transitionType == 2 && g_mwu == "mwu")  // only possible mwu transition
    tagProb=0;
  else 
    // get tag n-gram probability
    if(tagsPlusContext.size() < 5){
      tagProb = ngramProb(tagsPlusContextList,1,tagsPlusContext.size(),INTERPOLATION_LIMIT,0) ;
    }else{
      cerr << "TAGGER ERROR: supplied n-gram is of unsupported size " << tagsPlusContext.size() << endl;
      exit(1);
    }
  
  if(g_debug>2){
    cout << "tag-" << tagsPlusContext.size() << "gram: ";
    list_of_words
      *temp = tagsPlusContextList;
    while(temp != NULL){
      cout << temp->word << " ";
      temp = temp->next;
    }
    cout << tagProb  << endl;
  }

  destroyWordList(tagsPlusContextList);
 
  // create list of context and tags
  vector<string> context = contextSequence;
  context.pop_back();         
  context.push_back(destinationTag);
  context.push_back(contextSequence.back());
  
  list_of_words *contextList = createWordList(context);
  
  // get context n-gram probability
  if(context.size() == 3){
    contextProb = ngramProb(contextList,1,3,3,1);
  }else{
    cerr << "TAGGER ERROR: supplied context n-gram is of unsupported size " << context.size() << endl;
    exit(1);
  }

  if(g_debug>2){
    cout << "context-" << context.size() << "gram: " ;
    for(vector<string>::iterator vi=context.begin() ; vi!=context.end() ; vi++){
      cout << (*vi) << " " ;
    }
    cout << contextProb << endl;
  }

  destroyWordList(contextList);
  
  // combine the three values
	
  if(g_hmm == 1)
  {
	/*wordProb = 100*ME_Probability(tagSequence, 
			     contextSequence,
			     index);*/
  	return(wordProb + tagProb + contextProb);
  }
  if(g_hmm == 0)
  {
	return contextProb;
  }
}

double wordTagProb(list_of_words *wordTagList)
  // retrieve probability of word given tag
{	
  list_of_numbers *probability = word_tuple_grams(wordTagList,wordTagFaddNumber);
  
  double value = (probability == NULL ? MAX_WORD_TAG_VALUE : probability->word);
  
  destroyNumberList(probability);

  if(value > MAX_WORD_TAG_VALUE)
    return(MAX_WORD_TAG_VALUE);

  return(value);
}

double tagWordProb(list_of_words *tagWordList)
  // retrieve probability of tag given word
{	
  list_of_numbers *probability = word_tuple_grams(tagWordList,tagWordFaddNumber);
  
  double value = (probability == NULL ? MAX_TAG_WORD_VALUE : probability->word);
  
  destroyNumberList(probability);

  if(value > MAX_TAG_WORD_VALUE)
    return(MAX_TAG_WORD_VALUE);

  return(value);
}

double ngramProb(list_of_words *tags, double totalLambda, int n, int interpolationLimit, int useContextFadd) 
  // compute probability of n gram (through interpolation with 
  // n-1 gram if n is actually above the interpolation limit)
  // weight probability with totalLambda (or use totalLambda as
  // total to be divided when doing further interpolation)
{
  /////////////////////////////////////////////////////////////
  // NOTE: perhaps value should be weighted by lambda even if 
  //       MAX_NGRAM_VALUE is used?
  /////////////////////////////////////////////////////////////
  
  double
    ngramValue,ngramLambda,
    lowerOrderValue,lowerOrderLambda;

  long int
    prefixFaddNumber,
    ngramFaddNumber;

  // get the correct fadd number
  if(useContextFadd){
    ngramFaddNumber = contextFaddNumber;
  }else{
    switch(n){
    case 2:
      ngramFaddNumber  = bigramFaddNumber;
      break;
    case 3:
      ngramFaddNumber  = trigramFaddNumber;
      prefixFaddNumber = prefixBigramFaddNumber;
      break;
    case 4:
      ngramFaddNumber  = fourgramFaddNumber;
      prefixFaddNumber = prefixTrigramFaddNumber;
      break;
    }
  }

  if(n > interpolationLimit){ 

    //////////////////////
    // do interpolation //
    //////////////////////
    
    // get prefix
    vector <string> prefixVector;
    list_of_words *current = tags;
    for(int i=0 ; i<n-1 ; ++i){
      prefixVector.push_back(current->word);
      current = current->next;
    }

    // create list of words with same contents
    list_of_words *prefixList = createWordList(prefixVector);
  
    // create list of numbers of data associated with prefix
    list_of_numbers *prefixData = word_tuple_grams(prefixList,prefixFaddNumber);
    
    // compute lambda
    if(prefixData != NULL){  

      int
	prefixCount     = prefixData->word,
	prefixDiversity = prefixData->next->word;

      ngramLambda = prefixCount/(prefixCount+g_diversity*prefixDiversity);


    }else{
      if(g_debug>4) {
	cout << "prefixData NULL! ";
	for(int i=0 ; i<n-1 ; ++i){
	  cout << prefixVector[i] << " ";
	}
	cout << endl;
      }
      
      // prefix not found: ngram lambda will be set to zero
      // return MAX_NGRAM_VALUE;
      ngramLambda = 0;
    }

    ngramLambda *= totalLambda;


    destroyWordList(prefixList);
    destroyNumberList(prefixData);
        
    // compute lambda remaining for lower n-gram
    lowerOrderLambda = totalLambda - ngramLambda;
      
    // compute probability of lower n-gram
    lowerOrderValue = ngramProb(tags->next,lowerOrderLambda,n-1,interpolationLimit,useContextFadd);

    // compute probability of n-gram
    if(ngramLambda > 0){
      
      list_of_numbers *ngramData = word_tuple_grams(tags,ngramFaddNumber);
      
      if(ngramData == NULL){
	// ngram not found; not considered in interpolation
	ngramValue  = lowerOrderValue;
      }else{
	// ngram found; combine with lower ngram value
	ngramLambda = -100*log(ngramLambda);
	ngramValue  = g_addLogProbabilities(ngramData->word+ngramLambda,lowerOrderValue);
      }
      destroyNumberList(ngramData);
    }else{
      // ngramLambda is 0; ngram value not considered in interpolation
      ngramValue = lowerOrderValue;
    }

  }else{

    //////////////////////
    // no interpolation //
    //////////////////////
    
    // use totalLambda for lambda
    ngramLambda = totalLambda;

    list_of_numbers *ngramData = word_tuple_grams(tags,ngramFaddNumber);
    
    if(ngramData == NULL){
//       if(useContextFadd) {
//         list_of_words *cw=tags;
//         if (cw -> word == cw -> next -> next -> word)
//           ngramValue = 0;
//         else
//           ngramValue = 1350;
//       } else 
      // ngram not found; use default bad value
      // ngramLambda = -100*log(ngramLambda);
        ngramValue  = MAX_NGRAM_VALUE; // + ngramLambda;
    }else{
      ngramLambda = -100*log(ngramLambda);
      ngramValue  = ngramData->word + ngramLambda;	
    }
    destroyNumberList(ngramData);
  }

  if (ngramValue > MAX_NGRAM_VALUE) { ngramValue = MAX_NGRAM_VALUE; }
  if (ngramValue < 0 ) { ngramValue = 0; }

  return(ngramValue);
}

list_of_words *createWordList(vector<string> words)
  // create list_of_words from all elements in vector 'words'
{
  list_of_words 
    *first   = new list_of_words,
    *current = first;
  
  current->next = NULL;
  for(vector<string>::iterator vi=words.begin() ; vi!=words.end() ; vi++){
    current->word = (*vi).c_str();
    if(vi!=words.end()-1){
      current->next = new list_of_words;
      current       = current->next;
    }else{
      current->next = NULL;
    }
  }
  return(first);
}

void destroyWordList(list_of_words *list)
  // remove such a linked list from memory
{
  list_of_words 
    *current = list,
    *next;
  
  while(current != NULL){
    next = current->next;
    delete(current);
    current = next;
  }
}

void destroyNumberList(list_of_numbers *list)
  // remove such a linked list from memory
{
  list_of_numbers 
    *current = list,
    *next;
  
  while(current != NULL){
    next = current->next;
    delete(current);
    current = next;
  }
}

int loadProbsFourgram(const char *tupleFile, 
		      const char *dictFile0, 
		      const char *dictFile1, 
		      const char *dictFile2,
		      const char *dictFile3)
  // initialize fourgram data
{
  list_of_words 
    *tuple = new list_of_words,
    *dict0 = new list_of_words,
    *dict1 = new list_of_words,
    *dict2 = new list_of_words,
    *dict3 = new list_of_words;
  
  tuple->word = tupleFile;
  dict0->word = dictFile0;
  dict1->word = dictFile1;
  dict2->word = dictFile2;
  dict3->word = dictFile3;
  
  tuple->next = dict0;
  dict0->next = dict1;
  dict1->next = dict2;
  dict2->next = dict3;
  dict3->next = NULL;
  
  fourgramFaddNumber = init_tuple(tuple);
  
  destroyWordList(tuple);
  
  return(fourgramFaddNumber != -1);
}

int loadProbsTrigram(const char *tupleFile, 
		     const char *dictFile0, 
		     const char *dictFile1, 
		     const char *dictFile2)
  // initialize trigram data
{
  list_of_words 
    *tuple = new list_of_words,
    *dict0 = new list_of_words,
    *dict1 = new list_of_words,
    *dict2 = new list_of_words;
  
  tuple->word = tupleFile;
  dict0->word = dictFile0;
  dict1->word = dictFile1;
  dict2->word = dictFile2;
  
  tuple->next = dict0;
  dict0->next = dict1;
  dict1->next = dict2;
  dict2->next = NULL;
  
  trigramFaddNumber = init_tuple(tuple);
  
  destroyWordList(tuple);
  
  return(trigramFaddNumber != -1);
}

int loadProbsBigram(const char *tupleFile, 
		    const char *dictFile0, 
		    const char *dictFile1)
  // initialize bigram data
{
  list_of_words 
    *tuple = new list_of_words,
    *dict0 = new list_of_words,
    *dict1 = new list_of_words;
  
  tuple->word = tupleFile;
  dict0->word = dictFile0;
  dict1->word = dictFile1;
  
  tuple->next = dict0;
  dict0->next = dict1;
  dict1->next = NULL;
  
  bigramFaddNumber = init_tuple(tuple);
  
  destroyWordList(tuple);
  
  return(bigramFaddNumber != -1);
}

int loadProbsWordTag(const char *tupleFile, 
		     const char *dictFile0, 
		     const char *dictFile1)
  // initialize word+tag data
{
  list_of_words 
    *tuple = new list_of_words,
    *dict0 = new list_of_words,
    *dict1 = new list_of_words;
  
  tuple->word = tupleFile;
  dict0->word = dictFile0;
  dict1->word = dictFile1;
  
  tuple->next = dict0;
  dict0->next = dict1;
  dict1->next = NULL;
  
  wordTagFaddNumber = init_tuple(tuple);
  if(wordTagFaddNumber)
    wordFaddNumber = init_dict(dictFile0,FADD_HASH);
  
  destroyWordList(tuple);	
  
  return(wordTagFaddNumber != -1);
}

int loadProbsTagWord(const char *tupleFile, 
		     const char *dictFile0, 
		     const char *dictFile1)
  // initialize tag+word data
{
  list_of_words 
    *tuple = new list_of_words,
    *dict0 = new list_of_words,
    *dict1 = new list_of_words;
  
  tuple->word = tupleFile;
  dict0->word = dictFile0;
  dict1->word = dictFile1;
  
  tuple->next = dict0;
  dict0->next = dict1;
  dict1->next = NULL;
  
  tagWordFaddNumber = init_tuple(tuple);

  destroyWordList(tuple);	
  
  return(tagWordFaddNumber != -1);
}

int loadProbsContextTag(const char *tupleFile, 
			const char *dictFile0, 
			const char *dictFile1,
			const char *dictFile2)
  // initialize context data
{
  list_of_words 
    *tuple = new list_of_words,
    *dict0 = new list_of_words,
    *dict1 = new list_of_words,
    *dict2 = new list_of_words;
  
  tuple->word = tupleFile;
  dict0->word = dictFile0;
  dict1->word = dictFile1;
  dict2->word = dictFile2;
  
  tuple->next = dict0;
  dict0->next = dict1;
  dict1->next = dict2;
  dict2->next = NULL;
  
  contextFaddNumber = init_tuple(tuple);
  
  destroyWordList(tuple);
  
  return(contextFaddNumber != -1);
}

int loadPrefixBigram(const char *tupleFile, 
		     const char *dictFile0, 
		     const char *dictFile1)
  // initialize prefix bigram data
{
  list_of_words 
    *tuple = new list_of_words,
    *dict0 = new list_of_words,
    *dict1 = new list_of_words;
  
  tuple->word = tupleFile;
  dict0->word = dictFile0;
  dict1->word = dictFile1;
  
  tuple->next = dict0;
  dict0->next = dict1;
  dict1->next = NULL;
  
  prefixBigramFaddNumber = init_tuple(tuple);
  
  destroyWordList(tuple);
  
  return(prefixBigramFaddNumber != -1);
}

int loadPrefixTrigram(const char *tupleFile, 
		      const char *dictFile0, 
		      const char *dictFile1,
		      const char *dictFile2)
  // initialize prefix tigram data
{
  list_of_words 
    *tuple = new list_of_words,
    *dict0 = new list_of_words,
    *dict1 = new list_of_words,
    *dict2 = new list_of_words;
  
  tuple->word = tupleFile;
  dict0->word = dictFile0;
  dict1->word = dictFile1;
  dict2->word = dictFile2;
  
  tuple->next = dict0;
  dict0->next = dict1;
  dict1->next = dict2;
  dict2->next = NULL;
  
  prefixTrigramFaddNumber = init_tuple(tuple);
  
  destroyWordList(tuple);
  
  return(prefixTrigramFaddNumber != -1);
}





void closeDataFiles()
  // close all tuples
{
  int n = g_model+1;

  if(n >= 2)
    close_tuple(bigramFaddNumber);
  if(n >= 3){
    close_tuple(trigramFaddNumber);
    close_tuple(prefixBigramFaddNumber);
  }
  if(n >= 4){
    close_tuple(fourgramFaddNumber);
    close_tuple(prefixTrigramFaddNumber);
  }

//  if(!g_baseline)
    close_tuple(contextFaddNumber);

  close_tuple(wordTagFaddNumber);

//  if(g_lexicalAnalysis)
//    guesser_closeData();
}

void tagger_close()
{
  closeDataFiles();
}


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
		     string wordTagLexiconDictFile)
  // load fadd data structures used by the tagger (data used by the
  // lexical analysis part (guesser.cc) is loaded separately)
{

  if(!loadProbsWordTag(wordTagTupleFile.c_str(),
		       wordDictFile.c_str(),
		       tagDictFile.c_str())) {
    cerr << "TAGGER ERROR: cannot load word-tag tuple" << endl;
    tagger_close();
    exit(1);
  }

  

  //if(!g_baseline){
    if(!loadProbsContextTag(contextTagTupleFile.c_str(),
			    contextDictFile.c_str(),
			    tagDictFile.c_str(),
			    contextDictFile.c_str())) {
      cerr << "TAGGER ERROR: cannot load context tuple" << endl;
      tagger_close();
      exit(1);
    }
 // }

  // "g_model" is the n-gram model we want to use; 2 for bigram etc.
  // Since we are now adding an extra position for context, we use
  // sequences of length n+1 when using an n-gram POS-tag model.

  int n = g_model+1;

  // note: we use a value for g_model of 0 to indicate the use of the 
  // baseline method (in which case not a single tag n-gram model is
  // needed); this means n will be 1, and no unnecessary models will 
  // be loaded below

  if(n >= 2){
    if(!(loadProbsBigram(bigramTupleFile.c_str(),
			 contextDictFile.c_str(),
			 tagDictFile.c_str()))) {
      cerr << "TAGGER ERROR: cannot load bigram tuple" << endl;
      tagger_close();
      exit(1);
    }
  }else{
    //if(!g_baseline){
      cerr << "TAGGER ERROR: g_model = " << g_model << "; should be 1 or 2 or 3" << endl;
      tagger_close();
      exit(1);
  //  }
  }

  if(n >= 3){
    if(!(loadProbsTrigram(trigramTupleFile.c_str(),
			  tagDictFile.c_str(),
			  contextDictFile.c_str(),
			  tagDictFile.c_str()))) {
      cerr << "TAGGER ERROR: cannot load trigram tuple" << endl;
      tagger_close();
      exit(1);
    }

    if(!(loadPrefixBigram(prefixBigramTupleFile.c_str(),
			  tagDictFile.c_str(),
			  contextDictFile.c_str()))) {
      cerr << "TAGGER ERROR: cannot load prefix bigram tuple" << endl;
      tagger_close();
      exit(1);
    }
  }

  if(n >= 4){
    if(!(loadProbsFourgram(fourgramTupleFile.c_str(),
			   tagDictFile.c_str(),
			   tagDictFile.c_str(),
			   contextDictFile.c_str(),
			   tagDictFile.c_str()))) {
      cerr << "TAGGER ERROR: cannot load fourgram tuple" << endl;
      tagger_close();
      exit(1);
    }

    if(!(loadPrefixTrigram(prefixTrigramTupleFile.c_str(),
			  tagDictFile.c_str(),
			  tagDictFile.c_str(),
			  contextDictFile.c_str()))) {
      cerr << "TAGGER ERROR: cannot load prefix trigram tuple" << endl;
      tagger_close();
      exit(1);
    }
  }
}


void tagger_loadContextLabels(string usedContextFileName)
  // load the context labels used in the trainingdata; these will be used
  // later in creating copies of states with all possible context labels
{
  ifstream USED_CONTEXT;
  USED_CONTEXT.open(usedContextFileName.c_str());
  if(!USED_CONTEXT){
    cerr << "TAGGER ERROR: cannot load used context labels file" << endl;
    tagger_close();
    exit(1);
  }
  string label;
  while(!USED_CONTEXT.eof()){
    getline(USED_CONTEXT,label);
    if (label.length()>0) {  // ignore empty string at end of file
      // cerr << "READING context " << label << endl;
      g_usedContextLabels.push_back(label);
    }
  }
  USED_CONTEXT.close();
}


void TRELLIS::reduceTags()
  // remove double occurrences of tag at same position, summing their probabilities
  // and storing the result in the first tag encountered, which is not removed
  // double occurrences occur due to introduction of context label; here we
  // are removing effect of context again...
{  
  // for every position in trellis
	double col_sum[1000];
	int row_num[1000];
	int col = 0;
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    // remove all except first in case of multiple occurrences of same tag
	double sum_col = 0;
	int row = 0;
    for(vector<TAGNODE>::iterator t1=v->begin() ; t1!=v->end() ; ++t1){
      if(!(t1->markedAsDeleted())){
	for(vector<TAGNODE>::iterator t2=t1+1 ; t2!=v->end() ; ++t2){
	  if(t2->getTagName() == t1->getTagName()){
	    double
	      prob1 = t1->getForwardBackwardProbability(),
	      prob2 = t2->getForwardBackwardProbability(),
	      sum   = g_addLogProbabilities(prob1,prob2);
	      if(g_hmm == 1)
		{
			g_nor_grid[col][row] = sum;
		}
		if(g_hmm == 0)
		{
			g_nor_ME_grid[col][row] = sum;
		}
		
		
		if(row ==0)
			col_sum[col] = sum;
		else 
			col_sum[col] = g_addLogProbabilities(col_sum[col],sum);
		
		//sum += g_grid[col][row] ;
	    
	    t1->setForwardBackwardProbability(sum);
	//	if(g_hmm == 1)
	//	{
	//		g_grid[col][row] += sum/100.0;
			
	//	}
	//	if(g_hmm == 0)
	//	{
	//		g_grid[col][row] += sum;
	//	}
	    t1->setContextName(t1->getContextName()+ " + " + t2->getContextName());
	    t2->markAsDeleted();
	    t2->setContextName(t2->getContextName()+" (SUBSUMED)");
	  }
	}
	row ++;
	row_num[col] = row;
      }
    }
		
    col ++;
  }


if(g_hmm == 1)
{
	col = 0;
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    // remove all except first in case of multiple occurrences of same tag
	int row = 0;
    for(vector<TAGNODE>::iterator t1=v->begin() ; t1!=v->end() ; ++t1){
      if(!(t1->markedAsDeleted())){
		if(row_num[col] == 1)
			g_nor_grid[col][row] = 0;
		else
			g_nor_grid[col][row] = (g_nor_grid[col][row]-col_sum[col])/100;
		row ++;	
	}
      }	
    col ++;
  }
}

if(g_hmm == 0)
{
	col = 0;
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    // remove all except first in case of multiple occurrences of same tag
	int row = 0;
    for(vector<TAGNODE>::iterator t1=v->begin() ; t1!=v->end() ; ++t1){
      if(!(t1->markedAsDeleted())){
		if(row_num[col] == 1)
			g_nor_ME_grid[col][row] = 0;
		else
			g_nor_ME_grid[col][row] = g_nor_ME_grid[col][row]-col_sum[col];
		row ++;	
	}
      }	
    col ++;
  }
}
}


void TRELLIS::reduceTags_NO_Context()
  // remove double occurrences of tag at same position, summing their probabilities
  // and storing the result in the first tag encountered, which is not removed
  // double occurrences occur due to introduction of context label; here we
  // are removing effect of context again...
{  
  // for every position in trellis
	double col_sum[1000];
	int row_num[1000];
	int col = 0;
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    // remove all except first in case of multiple occurrences of same tag
	double sum_col = 0;
	int row = 0;
    for(vector<TAGNODE>::iterator t1=v->begin() ; t1!=v->end() ; ++t1){
      if(!(t1->markedAsDeleted())){
	//for(vector<TAGNODE>::iterator t2=t1+1 ; t2!=v->end() ; ++t2){
	  //if(t2->getTagName() == t1->getTagName()){
	    double
	      prob1 = t1->getForwardBackwardProbability(),
	    //  prob2 = t2->getForwardBackwardProbability(),
	      sum   = prob1;
	      if(g_hmm == 1)
		{
			g_nor_grid[col][row] = sum;
		}
		if(g_hmm == 0)
		{
			g_nor_ME_grid[col][row] = sum;
		}
		
		
		if(row ==0)
			col_sum[col] = sum;
		else 
			col_sum[col] = g_addLogProbabilities(col_sum[col],sum);
		

	//	sum += g_grid[col][row] ;
	    
	    t1->setForwardBackwardProbability(sum);
	//	if(g_hmm == 1)
	//	{
	//		g_grid[col][row] += sum/100.0;
	//		
	//	}
	//	if(g_hmm == 0)
	//	{
	//		g_grid[col][row] += sum;
	//	}
	    t1->setContextName(t1->getContextName()+ " + " + t1->getContextName()); 
	row ++;
	row_num[col] = row;
      }
    }
		
    col ++;
  }


if(g_hmm == 1)
{
	col = 0;
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    // remove all except first in case of multiple occurrences of same tag
	int row = 0;
    for(vector<TAGNODE>::iterator t1=v->begin() ; t1!=v->end() ; ++t1){
      if(!(t1->markedAsDeleted())){
		if(row_num[col] == 1)
			g_nor_grid[col][row] = 0;
		else
			g_nor_grid[col][row] = (g_nor_grid[col][row]-col_sum[col])/100;
		row ++;	
	}
      }	
    col ++;
  }
}

if(g_hmm == 0)
{
	col = 0;
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    // remove all except first in case of multiple occurrences of same tag
	int row = 0;
    for(vector<TAGNODE>::iterator t1=v->begin() ; t1!=v->end() ; ++t1){
      if(!(t1->markedAsDeleted())){
		if(row_num[col] == 1)
			g_nor_ME_grid[col][row] = 0;
		else
			g_nor_ME_grid[col][row] = g_nor_ME_grid[col][row]-col_sum[col];
		row ++;	
	}
      }	
    col ++;
  }
}
}


void TRELLIS::Combine()
{
	int col = 0;
	for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
	// remove all except first in case of multiple occurrences of same tag
		int row = 0;
	for(vector<TAGNODE>::iterator t1=v->begin() ; t1!=v->end() ; ++t1){
	if(!(t1->markedAsDeleted())){
			//this is only HMM model here. so you must understand it now.
			//double 	sum = g_nor_grid[col][row];
			double 	sum = g_addLogProbabilities(g_nor_grid[col][row],g_nor_ME_grid[col][row]);
			g_nor_grid[col][row] = 0.0;
			g_nor_ME_grid[col][row] = 0.0;
			t1->setForwardBackwardProbability(sum);
			row ++;	
		}
	}	
	col ++;
	}
	
}

void TRELLIS::expandTags()
  // replace each tagnode with as many copies as there are different context
  // labels being used, assigning the different labels to the new tagnodes
{
  vvTAGNODE newTrellis(words.size()+2);
   
  // for every position in trellis
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    // for every tag at that position
    for(vector<TAGNODE>::iterator t=v->begin() ; t!=v->end() ; ++t){
      // for all context labels being used
      for(vector<string>::iterator c=g_usedContextLabels.begin() ; c!=g_usedContextLabels.end() ; ++c){
	// create a copy of the original tagnode
	TAGNODE newTagNode(*t);
	// set new context name
	newTagNode.setContextName(*c);
	// add new tag to new trellis
	newTrellis[newTagNode.getWordPos()+1].push_back(newTagNode);
      }
    }
  }
  // replace old trellis with new one
  trellis = newTrellis;
}




TRELLIS::TRELLIS(vector<string> words, 
		 vvTAGNODE p_trellis,
		 string startTag, 
		 string endTag)
  :
  words(words),           
  trellis(p_trellis) // NOTE: includes start and end dummy positions already
{
  countWordsAndTags(0);


  // note: it is important that this is done *before* adding dummy tagnodes
  // (below), since otherwise we would end up with dummy tagnodes that have
  // non-dummy context labels assigned to them
  
  // add dummy tagnodes at begin and end of trellis
  
  expandTags();
  

  
  if(startTag == "")
    startTag = SENTENCE_START;
  if(endTag == "")
    endTag   = SENTENCE_END;
  
  TAGNODE
    dummyTagNodeStart(startTag,words,-1,1,DEFAULT_TAG_ID),
    dummyTagNodeEnd(endTag,words,-1,1,DEFAULT_TAG_ID);

  dummyTagNodeStart.mark_l_reachable();
  
  dummyTagNodeStart.setContextName(DUMMY_CONTEXT);
  dummyTagNodeEnd.setContextName(DUMMY_CONTEXT);
  
  int historySize;
  
  historySize = 2;

  
  vector<string> 
    stateInternalHistoryStart(historySize,startTag),
  
  stateInternalContextHistory(CONTEXT_STATESIZE,DUMMY_CONTEXT);
  
  // add statenode to first dummy tagnode
  dummyTagNodeStart.addDummyState(stateInternalHistoryStart,stateInternalContextHistory);
  
  // add dummy tagnodes to first and last position of trellis
  trellis.begin()->push_back(dummyTagNodeStart);
  //(trellis.end()-1)->push_back(dummyTagNodeEnd);
  trellis[words.size()+1].push_back(dummyTagNodeEnd);
}

TRELLIS::~TRELLIS()
{}

void TRELLIS::computeForwardProbabilities()
  // forward computation; includes the adding of new states
{
  // assign initial Alpha (0) to all starting states
  for(vector<TAGNODE>::iterator vi=trellis.begin()->begin() ; vi!=trellis.begin()->end() ; ++vi){
    vi->setAllStatesTo(0,DIRECTION_FORWARD);
  }
  
  // for every vector v in the trellis except the last in-sentence one
  int index = 1;
  for(vvTAGNODE::iterator v=trellis.begin() ; v!=trellis.end()-1 ; ++v){
    index +=1;
    // for every TAGNODE t in v
    for(vector<TAGNODE>::iterator t=v->begin() ; t!=v->end() ; ++t){
      
      int tagSpan = t->getTagSpan();
      if (t->l_reachable()) {
      
	list<STATENODE> *associatedStates = t->getAssociatedStates();
	
	// for every STATENODE s in t
	for(list<STATENODE>::iterator s=associatedStates->begin() ; s!=associatedStates->end() ; ++s){
	  
	  // get iterator to vector at position v+tagSpan
	  vvTAGNODE::iterator v2(v+tagSpan);
	  
	  // for every TAGNODE t2 in vector v2
	  for(vector<TAGNODE>::iterator t2=v2->begin() ; t2!=v2->end() ; ++t2){
	    
	    int legal; 
		if(g_mwu == "mwu")
		{
			legal= MWUTool::legal_transition(s->getTagName(),t2->getTagName());
		}
		else if(g_mwu == "tag")
		{
			legal= MWUTool::legal_transition_no(s->getTagName(),t2->getTagName());
		}
		else if(g_mwu == "sim")
		{
			legal= MWUTool::legal_transition_simp_no(s->getTagName(),t2->getTagName());
		}
		else
		{
			exit(2);
			cout<<"please check g_mwu value"<<endl;
		}

	    if (legal) {
	      
	      // tagnode can be reached
	      t2->mark_l_reachable();
	      
	      // create new state, history based on s, add to tagNode t2
	      list<STATENODE>::iterator newState = t2->addNewState(s);
	 
	      // create transition
		vector<string> ttt = t->getObservedWords();
	     	string wl1 = ttt[0];
		 ttt = t2->getObservedWords();
	     	string w0 = ttt[0];

		string wr1;
		if(v==trellis.end()-2)
		{
			wr1 = END_WORD;
		}
		else {
			vvTAGNODE::iterator v3(v+tagSpan+1);
			vector<TAGNODE>::iterator t3=v3->begin();

		 	ttt = t3->getObservedWords();
	     		wr1 = ttt[0];
		}
		
				

	      double transitionProb = s->createTransition(newState,legal,index,wl1,w0,wr1); 
	      
	      // add transition probability to forward probability
	      newState->addToComputation(s,transitionProb,DIRECTION_FORWARD);
	    } else {
	      if (g_debug) {
		cout << "filtered " << s->getTagName() << " --> " << 
		  t2->getTagName() << endl;
	      }
	    }
	  }
	}
      }
    }
  }
}

void TRELLIS::computeBackwardProbabilities()
  // backward computation; uses trellis constructed during forward computation
{
  // assign initial Beta (0) to all final states
  for(vector<TAGNODE>::iterator vi=(trellis.rbegin())->begin() ; vi!=(trellis.rbegin())->end() ; ++vi){
    vi->setAllStatesTo(0,DIRECTION_BACKWARD);
    vi->setAllStatesReach();
  }
  
  // visiting in reversed order all vector v in trellis
  for(vvTAGNODE::reverse_iterator v=trellis.rbegin()+1 ; v!=trellis.rend() ; ++v){
 
    // for every TAGNODE t in v
    for(vector<TAGNODE>::iterator t=v->begin() ; t!=v->end() ; ++t){
      if(t->l_reachable()) {

	// for every STATENODE s in t
	
	for(list<STATENODE>::iterator s=t->getAssociatedStates()->begin() ; s!=t->getAssociatedStates()->end() ; ++s){

	  vector<TRANSITION> transitions = s->getTransitions();
	  
	  // for every state s2 that is reachable from s
	  for(vector<TRANSITION>::iterator tr=transitions.begin() ; tr!=transitions.end() ; ++tr){
	    
	    if (tr->getTargetState()->r_reachable()) {
	      s->mark_r_reachable();
	    	    
	    // add transition probability to backward probability
	    s->addToComputation(tr->getTargetState(),tr->getTransitionProbability(),DIRECTION_BACKWARD);
	    }
	  }
	} 
      }
    }
  }
}


void TRELLIS::computeForwardBackwardValues()
  // combine the results of the two computations
{
  for(vvTAGNODE::iterator vi=trellis.begin() ; vi!=trellis.end() ; ++vi){
    for(vector<TAGNODE>::iterator t=vi->begin() ; t!=vi->end() ; ++t){
      t->computeForwardBackwardValue();
    }		
  }
}

void TRELLIS::computeBaselineValues()
  // visit all tags and assign just word-tag probabilities as part of the baseline method
{
  
  
}

void TRELLIS::rankTags()
  // put tags in order of increasing forward/backward value
{
  for(vvTAGNODE::iterator vi=trellis.begin()+1 ; vi!=trellis.end()-1 ; ++vi){
    sort(vi->begin(),vi->end());
  }
}

void TRELLIS::keepNBestTags(int n)
  // keep only the n best tags, remove rest; call this function after rankTags;
  // tags (tagNodes) that are to be removed are actually marked as deleted
{
  for(vvTAGNODE::iterator vi=trellis.begin()+1 ; vi!=trellis.end()-1 ; ++vi){
    int number = 1;
    for(vector<TAGNODE>::iterator t=vi->begin() ; t!=vi->end() ; ++t){
      if(!(t->markedAsDeleted())){
	if(number++>n){
	  t->markAsDeleted();
	}
      }
    }
  }
}

void TRELLIS::keepPercentageOfTags(int p)
  // keep the best p percent of all tags; call this function after rankTags
{
  int 
    tagCount = 0,
    targetCount;
  
  // count total number of tags
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    for(vector<TAGNODE>::iterator t=v->begin() ; t!=v->end() ; ++t){
      if(!(t->markedAsDeleted())){
	tagCount++;
      }
    }
  }
    
  // compute number of tags to keep
  targetCount = p * tagCount / 100;
  
  // for all tags, compute their difference to the best
  for(vvTAGNODE::iterator vi=trellis.begin()+1 ; vi!=trellis.end()-1 ; ++vi){
    
    if(vi->size() > 0){ 
      
      // get first non-deleted tag (also best tag)
      vector<TAGNODE>::iterator t=vi->begin();
      while((t!=vi->end()) && (t->markedAsDeleted())) t++;
     
      if(t!=vi->end()){
	t->markAsBest();
	double best = t->getForwardBackwardProbability();
	
	// now compute difference with best tag for current and following tags
	for( ; t!=vi->end() ; ++t){
	  // only necessary if not deleted
	  if(!(t->markedAsDeleted())){
	    t->computeDifferenceWithBest(best);
	  }
	}
      }
    }
  }

  vector<TAGNODE*> allTags;

  // collect all pointers to tags in one vector "allTags"
  for(vvTAGNODE::iterator vi=trellis.begin()+1 ; vi!=trellis.end()-1 ; ++vi){
    for(vector<TAGNODE>::iterator t=vi->begin() ; t!=vi->end() ; ++t){
      // add only non-deleted tags
      if(!(t->markedAsDeleted())){
	allTags.push_back((TAGNODE *&) t);
      }
    }
  }
  
  // sort this vector of iterators on their differenceWithBest values
  sort(allTags.begin(),allTags.end(),TAGNODE_PTR_CMP());
  
  int seen = 0;
  
  double
    currentValue,
    lastValue = 0;
  
  // now mark bad tags; remove tag if: seen at least targetCount 
  // AND this is not the best tag for a given position
  for(vector<TAGNODE*>::iterator t=allTags.begin() ; t!=allTags.end() ; t++){
    currentValue = (*t)->getDifferenceWithBest();
    if((seen >= targetCount) && (!((*t)->markedAsBest()))){ 
      // tags with same value are both retained when using following check
      //if(currentValue > lastValue)
      (*t)->markAsDeleted();
    }
    lastValue = currentValue;
    seen++;
  }
}

void TRELLIS::StoreTags(double margin,map<int,int> & gold,int& right,int& sum)
  // keep only tags within margin of best; call this function after rankTags
{
	double zzz_threshold= margin;
	int index = 0;	
	for(vvTAGNODE::iterator vi=trellis.begin()+1 ; vi!=trellis.end()-1 ; ++vi)
	{
		// vector of tags could be empty
		if(vi->size())
		{
			int options = vi->size();
			//margin = zzz_threshold/(options*1.0f);
			
			// get first non-deleted tag (also best tag)
			vector<TAGNODE>::iterator t=vi->begin();
			while((t!=vi->end()) && (t->markedAsDeleted())) t++;
			sum++;
			if(t->getTagID() == gold[index])
			{
				right++;	
			}
			double bestPlusMargin = t->getForwardBackwardProbability();
			for(t = t+1 ; t!=vi->end() ; ++t)
			{
				if(!(t->markedAsDeleted()))
				{
					OPTION tag_Options;
					tag_Options.score = (t->getForwardBackwardProbability() - bestPlusMargin);
					if(t->getTagID() == gold[index])
					{
						tag_Options.tag = 1;
					}
					else
					{
						tag_Options.tag = 0;		
					}
					g_all_result.push_back(tag_Options);
				}
			}
		}
		index ++;	
	}
}


	
void TRELLIS::keepGoodTags(double margin,map<int,int> & gold,int& right,int& sum)
  // keep only tags within margin of best; call this function after rankTags
{
	double zzz_threshold= margin;
	int index = 0;	
	for(vvTAGNODE::iterator vi=trellis.begin()+1 ; vi!=trellis.end()-1 ; ++vi)
	{
		// vector of tags could be empty
		if(vi->size())
		{
			int options = vi->size();
			//margin = zzz_threshold/(options*1.0f);
			
			// get first non-deleted tag (also best tag)
			vector<TAGNODE>::iterator t=vi->begin();
			while((t!=vi->end()) && (t->markedAsDeleted())) t++;
			sum++;
			if(t->getTagID() == gold[index])
			{
				right++;	
			}
			//that means that threshold is zero. only keep the best one.
			if(margin<0.0001)
			{
				//index++;
				//continue;
				for(t = t+1 ; t!=vi->end() ; ++t)
				{
					if(!(t->markedAsDeleted()))
					{
						t->markAsDeleted();
					}
				}
			}

			double bestPlusMargin = t->getForwardBackwardProbability() + margin;
			for(t = t+1 ; t!=vi->end() ; ++t)
			{
				if(!(t->markedAsDeleted()))
				{
					if(t->getForwardBackwardProbability() > bestPlusMargin)
					{
						t->markAsDeleted();
					}
					else
					{
						sum++;
						if(t->getTagID() ==gold[index])
							right++;	
					}
				}
			}
		}
		index ++;	
	}
}

int TRELLIS::collectGoodTags(int *remainingTags,
                             int *remainingTagScores,
                             int debug)
  // collect id numbers of tags not marked as deleted
{
  int tagCount = 0;

  for(vvTAGNODE::iterator vi=trellis.begin()+1 ; vi!=trellis.end()-1 ; ++vi){
    for(vector<TAGNODE>::iterator t=vi->begin() ; t!=vi->end() ; ++t){
      if(!(t->markedAsDeleted())){
	remainingTags[tagCount] = t->getTagID();
        remainingTagScores[tagCount] = int((t->getForwardBackwardProbability())/100);
        tagCount++;
      }
    }
  }
  return(tagCount);
}

void TRELLIS::printTagging(fstream & fresult)
  // print words and tags in nice looking columns with | inbetween
{
  int maxLength = 0;
  vector<string>::iterator w;
  for(w=words.begin() ; w!=words.end() ; ++w){
    if(w->size()>maxLength) maxLength=w->size();
  }
  ++maxLength;
  w=words.begin();
  for(vvTAGNODE::iterator vi=trellis.begin()+1 ; vi!=trellis.end()-1 ; ++vi){
    int size = w->size();
    string tab(maxLength-size,' ');
      fresult << *w ;
    for(vector<TAGNODE>::iterator t=vi->begin() ; t!=vi->end() ; ++t){
      if(!(t->markedAsDeleted())){
	fresult << " " << t->getTagName();
      }
    }
    fresult << endl;
    ++w;
  }
}

int TRELLIS::compareWithCorrect(vector<pair<string,pair<int,int> > > corrTags,int debug)
  // return difference between our tagging and 'correct' tagging
{
  int 
    difference = 0,
    position   = 1; // since 0 is dummy position
  
  if(debug>1) cout << "size of trellis: [" << trellis.size() << "]" << endl;
  
  string
    first  = "NONE",
    second = "NONE";

  for(vector<pair<string,pair<int,int> > >::iterator vi=corrTags.begin() ; vi!=corrTags.end() ; ++vi){
    
    string tagName = vi->first;

    pair<int,int> tagPositions = vi->second;
    
    int 
      tagposBegin = tagPositions.first,
      tagposEnd   = tagPositions.second,
      available   = 0;
    
//     if(debug>1) cout << "correct tag is: [" << tagName << "] " << tagposBegin << "-" << tagposEnd << endl;

    string wronglyAssignedTag;
    
    if (debug>1) cout << "output#" << words[tagposBegin];
    for(vector<TAGNODE>::iterator t=trellis[tagposBegin+1].begin() ; t!=trellis[tagposBegin+1].end() ; ++t){
      if(!(t->markedAsDeleted())){
// 	if(debug>1){
// 	  cout << "  remaining   : [" << t->getTagName() << "] " << tagposBegin;
// 	  cout << "-" << tagposBegin + t->getTagSpan();
// 	}

        if(debug>1){
          cout << "|" << t -> getTagName();
        }

	if(t->getTagName() == tagName && t->getTagSpan() == tagposEnd-tagposBegin){
// 	  if(debug>1) cout << " CORRECT";
	  available = 1;
	}else{
	  wronglyAssignedTag = t->getTagName();
	}
      }
    }

	if(debug>1) cout << endl;
    
    if(!available){
      if(debug){
	cout << "MISSING TAG: " << words[tagposBegin] << " " << tagName << " " 
             << tagposBegin << "-" << tagposEnd << endl;
	cout << "MISTAKE: word= " << words[tagposBegin] << " assigned= " 
	     << wronglyAssignedTag << " correct= " << tagName << endl;
      }
      difference++;
    }

    // begin temporary output
    // print word|assignedTag|correctTag in order to do sign test on different models
    //string assignedTag;
    //if(available) 
    //  assignedTag = tagName;
    //else
    //  assignedTag = wronglyAssignedTag;
    //cout << words[tagposBegin] << "|" << assignedTag << "|" << tagName << endl;
    // end temporary output

    first  = second;
    second = tagName;
    
    position++; // moving to position to which next correct tag applies
  }
  
  if(debug>1) cout << endl;
  
  return(difference);
}

void TRELLIS::countWordsAndTags(int onlyNonDeleted)
  // count number of tags in trellis [not marked as deleted], also count words
{
  wordCount = 0;
  tagCount  = 0;
  
  list<pair<int,int> > seenWords;
  
  for(vvTAGNODE::iterator v=trellis.begin()+1 ; v!=trellis.end()-1 ; ++v){
    for(vector<TAGNODE>::iterator t=v->begin() ; t!=v->end() ; ++t){
      if((onlyNonDeleted && !(t->markedAsDeleted())) || (!onlyNonDeleted)){
	
	// a tag is always a unique tag
	tagCount++;
	
	// decide if this word should be added
	pair<int,int> correspondingWord(t->getWordPos(),t->getTagSpan());
	
	int countedAlready = 0;
	
	for(list<pair<int,int> >::iterator li=seenWords.begin() ; li!=seenWords.end() ; li++){
	  if(*li == correspondingWord){
	    countedAlready = 1;
	  }
	}

	if(!countedAlready){
	  seenWords.push_back(correspondingWord);
	  wordCount++;
	}
	
      }
    }
  }
}


void TRELLIS::print()
  // show trellis; for debugging
{
  cout << endl << "TRELLIS STRUCTURE:" << endl << endl;
  
  for(vvTAGNODE::iterator v=trellis.begin() ; v!=trellis.end() ; ++v){
    for(vector<TAGNODE>::iterator t=v->begin() ; t!=v->end() ; ++t){
      t->print();
      cout << endl;
    }
  }
}
