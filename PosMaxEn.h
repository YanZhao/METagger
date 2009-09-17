#pragma once


#include "global.h"
#include <string>
#include <vector>



#include <iostream>
#include <map>
#include <math.h>
#include <queue>

#include "JBaseProc.h"
#include "Events.h"
#include "TagsSequence.h"
#include "LogLinear.h"
#include "Trellis.h"
#include "TagNode.h"
using namespace std;
#ifdef linux
using namespace __gnu_cxx;
#endif 


typedef enum tag_Pos_PredType
{
	pos_word = 0,
	pos_pre_word1 = 1,
	//pos_pre_word2 = 2,
	pos_suc_word1 = 3,
	pos_suc_word2 = 4,
	pos_pre_tag1 = 5,
	pos_pre_tag2 = 6,
	//pos_bi_tags = 7,
	//pos_bi_words_1 =8,
	//pos_bi_words_2 =9,
	//pos_bi_words_3 =10,
	//pos_bi_words_4 =11,

	
	pos_bi_tags = 1002,
	//pos_pre_tag2 = 1004,
	pos_pre_word2 = 1005,
	//pos_suc_word2 = 1005,
	pos_bi_words_1 =1008,
	pos_bi_words_2 =1009,
	pos_bi_words_3 =1010,
	pos_bi_words_4 =1011,
	
//	pos_pre_trigger = 7,
//	pos_suc_trigger = 8,
} POS_PRED_TYPE;

class CPosMaxEn : public CLogLinear
{
public:
	
	CPosMaxEn(void);
	~CPosMaxEn(void);
	
	int TrainModel(string& fileName);
	int Continue_TrainModel(string& fileName,int con_num);
	int TagFile(string strFileName, int AFileIndex = 0);
	int FB_FilterFile(string strFileName, int AFileIndex = 0);
	
private:
	virtual void LookupAndUpdateFeature(FEATURE &AFeature);
	int LoadFeatures(string AFeatureName, bool AMakeHash = true); 
	int LoadModel(int AFileIndex = 0);
	int FillInvvTAGNODE(vvTAGNODE& tags,vector<string>& vect_sent);
	
	int CollectEvents(string& fileName);
	double GIS(double& likelihood);
	void GIS_Estimate(void);
	template<class T> bool GetFeatureFromTemplate(T &ft, SAMPLE * wnd[], string & templateStr);


	int SaveFeatures(string AFeatureName);
	int SaveModel(int AFileIndex = 0);
	

	void ExtractFeatureFromWnd(SAMPLE * wnd[]);
	void ExtractEventFromWnd(SAMPLE * wnd[]);
	
	void CollectTxtEvents(string fileName,fstream& fout);

	string FB_FilterWindow_Context(vector<string>& vectIn ,int index);
	int GetProbability_Based_All_Context(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence , vector<int> & vectPOS, vector<double> & vectProbability_options,string & context_pre,string& context_cur);
	void FillWndContext(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence, int PresentPOS, SAMPLE* wnd_pointer[], SAMPLE wnd[],string& context_pre,string& context_cur);


	void FillInWnd(SAMPLE * pSample[], int index);
	int LoadPreTriggerPair(void);
	int LoadSucTriggerPair(void);
	int LoadDict(void);
	int ForceTagToID(string & pos);

	int GetDict(string& fileName);

	void LoadTiggerFeature(string AFileName);
	bool FindInPreTriggerPairs(string& key);
	bool FindInSucTriggerPairs(string& key);


	void SaveSamplesToTxt(string AFileName);
	void SaveFeaturesToTxt(string AFileName);
	void SavePredicatesToTxt(string AFileName);
	int GetPredictsFromFeatures(void);
	int FilterFeatures();

	int ReadSamplesAndCollectFeature(string& fileName);

	int IndexOfPredicate(PREDICATE &APre_Type);  
	int LookUpFeature(FEATURE& ft);
	

	bool GetCorrectionOptions(void);
	double FeatureModelExpecation(double & log_likelihood);
	//void LBFGS_Estimate(void);
	void LBFGS_Estimate_C(void);

	
	string N_Best_Result(vector<string>& vectIn, int& right,int& sum,double margin);
	string Single_Best_Result(vector<string>& vectIn);
	string TagWindow(vector<string>& vectIn ,int index);
	string FB_FilterWindow(vector<string>& vectIn ,int index);
	double FixCountFilter(int right, int sum, int all_num,double threshold,double& acc_result, double& threshold_result);
	string StoreAllOptions(vector<string>& vectIn,int& right,int& sum);

	string TagSentence(string& senIn);
	string FB_FilterSentence(string& senIn); 
	int GetProbability(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence, vector<int> & vectPOS, vector<double> & vectProbability);
	int GetProbability_Based_All(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence , vector<int> & vectPOS, vector<double> & vectProbability_options);
	void FillWnd(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence, int PresentPOS, SAMPLE* wnd_pointer[], SAMPLE wnd[]);
	int Filter_Unleagal(int lineNum,string & str);
	int FillInTagMap(int lineNum);
	bool IsValidFeature(int k);
	int GetTagFromEvents();


private:
	typedef priority_queue<SEQUENCE> PriorHeap;  
	PriorHeap * m_Heap, *m_NewHeap;
	void JHeapClear(PriorHeap *AHeap);          
 
	map<int, vector<int> > map_pos_options;
	vector< vector<string> > options_array;
	


	hash_set<string,StrHashTrait<string> > m_hashPreTriggers;
	hash_set<string,StrHashTrait<string> > m_hashSucTriggers;


	CEventList m_EventList; 
	
	string m_fileNameModel;	
	string m_fileNameModelOption;
	
	int m_numTag;
	int m_numFeatures;
	
	int m_corr; 
	int m_maxFeatureCount;
	int m_minFeatureCount;
	double m_correctFeatureE;
	double m_correctFeatureAlfa;

};



