#pragma once

#include <string>
#include <vector>
#include <map>
#include <time.h>
#include <iostream>

//this is for compatiable between Window and linux
#include "StrHashTrait.h"
#ifdef linux
#include <ext/hash_map>
#include <ext/hash_set>
#else 
#include <hash_map>
#include <hash_set>
#endif 

/////////////////////////////////////////////////////////
//here, we store some basic data structure, such as feature, predicate, and event. it's good starting 
//point for you to navigate the whole project. 

using namespace std;

typedef struct tag_Options
{
	int tag;
	double score;

	bool operator==(const tag_Options &right) const;
    	bool operator!=(const tag_Options &right) const;
    	bool operator<(const tag_Options &right) const;
    	bool operator>(const tag_Options &right) const;
    	bool operator<=(const tag_Options &right) const;
    	bool operator>=(const tag_Options &right) const;
}OPTION;

inline bool tag_Options::operator==(const tag_Options &right) const
{
	if (score == right.score)
		return true;
	else
		return false;
}

inline bool tag_Options::operator!=(const tag_Options &right) const
{
	if (score != right.score)
		return true;
	else
		return false;
}


inline bool tag_Options::operator<(const tag_Options &right) const
{
	if (score < right.score)
		return true;
	else
		return false;
}

inline bool tag_Options::operator>(const tag_Options &right) const
{
	if (score > right.score)
		return true;
	else
		return false;
}

inline bool tag_Options::operator<=(const tag_Options &right) const
{
	if (score <= right.score)
		return true;
	else
		return false;
}

inline bool tag_Options::operator>=(const tag_Options &right) const
{
	if (score >= right.score)
		return true;
	else
		return false;
}




typedef struct tag_Sample
	{
		tag_Sample()
		{	
			word = "";
			middle = "";
			tag = "";
		}
		string word;
		string middle;
		string tag;
	}SAMPLE;

	typedef struct tag_Feature
	{
		tag_Feature()
		{	
			predType = -1;
			predData = "";
			outTag = -1;
			previousTag = -1;
			alpha = 0;
		}

		int predType;
		string predData;
		int outTag;
		int previousTag; //this is only used in CRF model

		double alpha;
		friend ostream &operator<<(ostream &Out,tag_Feature &cFeature);
		friend istream &operator>>(istream &In,tag_Feature &cFeature);

	}FEATURE;


	typedef struct tag_Predicate
	{
		int predType;
		string predData;
		vector<pair<int,int> > indexFeature;
		//map<int,int> indexFeature;
		//map<int, int> indexBigramFeature;
	}PREDICATE;

	typedef struct tag_Event
	{
		int outTag;
		int BigramTag;
		int count;
		
		vector<int> vectIndexPredicate;
		friend ostream &operator<<(ostream &Out,tag_Event &cEvent);
		friend istream &operator>>(istream &In,tag_Event &cEvent);
	}EVENT;

typedef enum tag_Event_Operator_Mode  
{
	OperatorMode_Memory,  
	OperatorMode_File,    
}EVENT_OPERATOR_MODE;     

typedef enum tag_Event_List_Open_Mode
{
	OpenMode_Read,  
	OpenMode_Write,  
}EVENT_LIST_OPEN_MODE;

inline ostream &operator<<(ostream &Out,FEATURE &cFeature)
{ 
	
	Out.write(reinterpret_cast<char *>(&cFeature.predType),sizeof( int));

	int iWordLength = static_cast<unsigned int>(cFeature.predData.length());
	iWordLength++;
	Out.write(reinterpret_cast<char *>(&iWordLength),sizeof( int));
	Out.write(reinterpret_cast<const char *>(cFeature.predData.c_str()),iWordLength*sizeof(char));

	
	Out.write(reinterpret_cast<char *>(&cFeature.outTag),sizeof( int));
	Out.write(reinterpret_cast<char *>(&cFeature.previousTag),sizeof( int));

	Out.write(reinterpret_cast<char *>(&cFeature.alpha),sizeof(double));
	return Out;
}
inline istream &operator>>(istream &In,FEATURE &cFeature)
{
	char temp[100];
	In.read(reinterpret_cast<char *>(&cFeature.predType),sizeof(int));

	int iWordLength;
	In.read(reinterpret_cast<char *>(&iWordLength),sizeof(int));
	if(In.eof())
	{
		return In;
	}

	if(iWordLength>=100)
	{
		cout << "iWordLength" << iWordLength << endl;
		char * mT = new char[iWordLength + 1];
    	In.read(reinterpret_cast<char *>(mT),iWordLength*sizeof(char));
    	cFeature.predData.assign(mT);
		delete [] mT;
	}
	else
	{
    	In.read(reinterpret_cast<char *>(temp),iWordLength*sizeof(char));
    	cFeature.predData.assign(temp);  
	}
	In.read(reinterpret_cast<char *>(&cFeature.outTag),sizeof(int));
	In.read(reinterpret_cast<char *>(&cFeature.previousTag),sizeof(int));

	In.read(reinterpret_cast<char *>(&cFeature.alpha),sizeof(double));
	return In;
}

extern int g_maxSenLen ;
extern int g_maxPosList ;
extern int g_decodingMethod;
extern int g_flag_pos_options;
extern int g_threshold;
extern int g_gis;
extern float g_sigma;
extern string g_templateFileName;
extern string g_trainingFileName ;
extern string g_testingFileName ;
extern int g_syn_flag;
extern int g_me;
extern int g_crf;
extern string APPLICATION ;
extern int g_iteration;
extern int g_context;
const string DUMMY_CONTEXT          = "<DUMMY_CONTEXT>";
extern vector<string>
  g_usedContextLabels;

extern int g_right  ;
extern int g_sum ;
extern int g_sumTrainLine ;

extern int g_1_right;
extern int g_1_sum;

extern int g_2_right;
extern int g_2_sum;

extern int g_3_right;
extern int g_3_sum;

extern int g_0_right;
extern int g_0_sum;

extern int g_debug;
extern int sumTag ;
extern int min_sumTag ;
extern double min_value ;
extern int g_event_split;


extern const string ct_WordSeperator;  
extern const string SEN_SPLIT;
extern const string UNKNOWN_WORD ;
extern const string START_WORD ;
extern const string END_WORD ;
extern const string START_TAG ;
extern const string END_TAG ;
extern const string SPLIT_TAG ;

//extern map<int, vector<int> > map_pos_options;
extern map<int, int> map_pos_gold;
extern map<string, vector< int > > test_dict;
extern int g_Bigram ;
extern int g_combine;

extern clock_t startClk;
extern hash_map<string,double,StrHashTrait<string> > g_hashProb;
extern hash_map<string,double,StrHashTrait<string> > g_hashContextProb;
extern vector<tag_Options> g_all_result;


extern bool g_hmm ;
extern string g_mwu;
extern string g_TagIDMapFileName;
static const int EVENT_SPLIT = -100;

void InitGrid(void);


/*--------------------------------------------------------------------------------*/
extern "C" {
    // interface to LBFGS optimization written in FORTRAN
    extern void lbfgs(int * n, int * m, double * x, double * f, double * g,
		       int * diagco, double * diag, int * iprint, double * eps,
		       double * xtol, double * w, int * iflag);		       
}
/*--------------------------------------------------------------------------------*/








