#pragma once

#include "global.h"


#include "StrHashTrait.h"
#ifdef linux
#include <ext/hash_map>
#include <ext/hash_set>
#else 
#include <hash_map>
#include <hash_set>
#endif 
	
class CLogLinear
{
public:
	CLogLinear(void);
	virtual ~CLogLinear(void);

protected:
	virtual void LookupAndUpdateFeature(FEATURE &AFeature) = 0;
	int GetPosList(string AFileName);
	void AnalysizeTemplateString(string &str, int & pos_index, int & con_index);
	int TagToID(string tag);
	string IDToTag(int ID);
	int LoadPosTag(string APosIDMapFileName);
	int LoadTemplate(string & templateFileName);
	int FilterFeatures(); 
	int GetPredictsFromFeatures(void);
	void LBFGS_Estimate(void);
	
protected:
	vector<SAMPLE> m_vectSamples;
	SAMPLE m_BeginSample,m_EndSample;
	
	vector<FEATURE> m_vectFeatures;
	hash_map<string,int,StrHashTrait<string> > m_hashFeatures;

	vector<PREDICATE> m_vectPredicates;
	hash_map<string,int,StrHashTrait<string> > m_hashPredicates;


	vector<int> m_feature_count;
	vector<double> m_feature_count_log; //to avoid log calculation in GIS training
	
	vector<double> m_model_expectation;
	

	static const int WND_INDEX = 2;   //present word index in a window;
	static const int WND_SIZE = 2*WND_INDEX+1;  // window size;

	map<string,int> m_mapPosID;
	map<int,string> m_mapPosTag;
	
	vector<string> m_templateVect;
	vector<int> m_filter_count;

		
};



