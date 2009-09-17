
#include "global.h"
#include "LogLinear.h"

#include <fstream>
#include <set>
#include "Verify.h"
#include "JBaseProc.h"
#include "JTriggerMgr.h"

using namespace std;
using namespace HitZy;


string APPLICATION = "dutch";
int sumTag = 0;
int min_sumTag = 0;
double min_value = 1;

string g_templateFileName = "data/"+APPLICATION+"/template.txt";
string g_trainingFileName = "data/"+APPLICATION+"/train";
string g_testingFileName = "data/"+APPLICATION+"/test";
int g_iteration = 500;
int g_syn_flag = 0;
int g_me = 0;
int g_crf = 0;
int g_gis = 0;
string g_TagIDMapFileName; 

//there are three options for g_flag_pos_options
//0-->use all possible tags, such as chunking
//1-->use dictionary
//2-->use option file 
//if I wang to use dictionary file, I must add a unknown tag into the tag lists
int g_flag_pos_options = 2;


int g_threshold = 0;
float g_sigma = 0.0f;

int g_right   = 0;
int g_sum = 0;
int g_sumTrainLine = 0;





const string ct_WordSeperator = " ";  
const string SEN_SPLIT = "W@#";
const string UNKNOWN_WORD = "unknown_word_xxx";
const string START_WORD = "start_word_xxx";
const string END_WORD = "end_word_xxx";
const string START_TAG = "start_tag_xxx";
const string END_TAG = "end_tag_xxx";
const string SPLIT_TAG = "&";


CLogLinear::CLogLinear(void)
{
} 

CLogLinear::~CLogLinear(void)
{
}


int CLogLinear::TagToID(string tag)
{
	if(m_mapPosID.find(tag) == m_mapPosID.end( ))
	{
		return -1;
	}
	else
	{
		return m_mapPosID[tag];
	}
}
string CLogLinear::IDToTag(int ID)
{
	if(m_mapPosTag.find(ID) == m_mapPosTag.end( ))
	{
		return "";
	}
	else
	{
		return m_mapPosTag[ID];
	}
}

void CLogLinear::AnalysizeTemplateString(string &str, int & pos_index, int & con_index)
{
	//from string [-2,0] get two integral -2 and 0;
	vector<string> temp;
	String_SeperateToList_WithTrim(str.substr(1,str.length()-2),temp,",");
	pos_index = atoi(temp[0].c_str());
	con_index = atoi(temp[1].c_str());	
}


int CLogLinear::LoadTemplate(string & templateFileName)
{
	ifstream fIn;	
	fIn.open(templateFileName.c_str(), ios::in);
	VERIFY_FILE(fIn);
	string line;
	int num = 0;
	m_filter_count.resize(20);
	string template_num;
	string template_str;
	string filter_count;

	do 
	{
		char chline[300];
		fIn.getline(chline,300);
		line.assign(chline);
		if(line !="")
		{
			vector<string> vectline;
			String_SeperateToList_WithTrim(line,vectline," ");
			template_str = vectline[0];
			filter_count = vectline[1];
			int f_num = atoi(vectline[1].c_str());

			String_SeperateToList_WithTrim(template_str,vectline,":");
			template_num = vectline[0];
			int t_num = atoi(vectline[0].c_str());
						
			m_filter_count[t_num]=f_num;			

			m_templateVect.push_back(line);
			num++;
			//this statement deal with the last line of file, which usually only 
			//contain a return.
			line = "";
		}
		
	}while(!fIn.eof());
	fIn.close();
	cout<<"the number of templates is "<<num<<endl; 
	return 1;
}


int CLogLinear::LoadPosTag(string APosIDMapFileName)
{
	ifstream fIn;	
	fIn.open(APosIDMapFileName.c_str(), ios::in);
	VERIFY_FILE(fIn);

	string PosTag;
	int PosID;
	pair<string,int> pairPosID;
	pair<int,string> pairPosTag;

	int mCount;  
	mCount = 0;
	m_mapPosID.clear();
	m_mapPosTag.clear();
	while(!fIn.eof())
	{
		fIn>>PosID;
		fIn>>PosTag;
		pairPosID.first = (PosTag);
		pairPosID.second = PosID;
		////////////////////////////////////
		pairPosTag.first = PosID;
		pairPosTag.second = PosTag;
		if(m_mapPosID.find(PosTag) == m_mapPosID.end())
		{
			m_mapPosID.insert(pairPosID);
			m_mapPosTag.insert(pairPosTag);
			mCount ++;
		}
	}
	
	fIn.close();
	return mCount-2;
}


int CLogLinear::GetPosList(string AFileName)
{
	FILE* fsample;
	fsample = fopen(AFileName.c_str(), "rt");  
	if (fsample == NULL)
	{
		cout << "Cannot open training file:" << AFileName << endl;
		cout << "Cannot continue run...." << endl;
		throw ("Cannot open sample file");
		return -1;
	}
	set<string> posMap;
	while (! feof(fsample))
	{
		char line[1000];
		fgets(line,1000,fsample);
		string sline;
		
		sline.assign(line);
		if(sline == ""||sline == "\n")
		{
			continue;
		}
		if(sline.substr(0,3) == SEN_SPLIT)
		{
			continue;
		}
	
		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");
		//maybe there are three columns, but you must be sure that the tag is the last column.
		string tag = vectline[vectline.size()-1];
		if(posMap.find(tag)==posMap.end())
		{
			posMap.insert(tag);
		}
	}

	////////////////////////////////////////////////
	fclose(fsample);

	//write back to file
	string fileName;
	if(g_me == 1)
	{
		fileName = g_TagIDMapFileName;
	}
	if(g_crf ==1)
	{
		fileName = "model/"+APPLICATION+"/CRF_TagIDMap_"+g_mwu;
	}
	fsample = fopen(fileName.c_str(), "w");
	int i = 0;
	set<string>::iterator s1_Iter; 
	s1_Iter = posMap.begin( );
	while(s1_Iter != posMap.end())
	{	
		string ttt = *s1_Iter;
		fprintf(fsample, "%d %s\n", i , ttt.c_str());
		s1_Iter++;
		i++;
	}
	
	if(g_flag_pos_options == 1)
	{
		//add one specifice tag, unknown_word
		fprintf(fsample, "%d %s\n", i , UNKNOWN_WORD.c_str());
		i++;
	}

	//add two specific Tags, means begining and ending of sentence.
	if(g_me == 1)
	{
		fprintf(fsample, "%d %s\n", 10000 , START_TAG.c_str());
		i++;
		fprintf(fsample, "%d %s\n", 10001 , END_TAG.c_str());
	}
	
	fclose(fsample);
	int num = posMap.size();
	return num; //return the number of Tag set.
}

int CLogLinear::FilterFeatures()
{
	cout << "Filtering Features ....." << endl;

	vector<FEATURE> mFilter;  
	vector<int> mFeatureCount;

	size_t mSize;
	mSize = m_vectFeatures.size();
	cout<<"before filtering, features count is "<<mSize<<endl;
	for ( int k = 0; k < mSize; k ++)
	{
		
		if (m_feature_count[k]> g_threshold)
		{
			mFilter.push_back(m_vectFeatures[k]);
			mFeatureCount.push_back(m_feature_count[k]);
		}
	}

	m_vectFeatures.clear();
	m_hashFeatures.clear();

	
	mSize = mFilter.size();
        char buffer[50];
	string key, temp;
	for (int k = 0; k < mSize; k ++)
	{
		FEATURE &mCurItem = mFilter[k];
		m_vectFeatures.push_back(mCurItem);
		sprintf(buffer, "%d",mCurItem.predType);
		
		key = buffer;

		//add them to the m_hashFeatures
		key+=SPLIT_TAG;
		key+=mCurItem.predData;
		key+=SPLIT_TAG;
		sprintf(buffer, "%d",mCurItem.outTag);
		string temp(buffer);
		key+=temp;
		if(mCurItem.previousTag != -1)
		{
			key+=SPLIT_TAG;
			sprintf(buffer, "%d",mCurItem.previousTag);
			temp.assign(buffer);
			key+=temp;
		}
		m_hashFeatures.insert(pair<string,int>(key,k));
	}


	cout<<"after filtering, the features count is "<< m_vectFeatures.size() << endl;

	m_feature_count.clear();
	m_feature_count.resize(m_vectFeatures.size());
	copy(mFeatureCount.begin(),mFeatureCount.end(),m_feature_count.begin());

	m_model_expectation.resize(m_vectFeatures.size());
	fill(m_model_expectation.begin(),m_model_expectation.end(),0.0f);
	

	m_feature_count_log.resize(m_vectFeatures.size());
	for(int i = 0;i<m_feature_count_log.size();i++)
	{
		m_feature_count_log[i] = log(m_feature_count[i]);
	}
	//m_feature_count and m_feature_count_log will be used in GIS and LBFGS. We only calculate them once and store in these two class variables. 
	cout<<"filtering features is ok"<<endl;
	return m_vectFeatures.size();

}









