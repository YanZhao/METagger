#include "PosMaxEn.h"
//#include "DictAndConvert.h"
#include <fstream>
#include <set>

#include "Verify.h"
#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <ext/numeric>
#include <iomanip>
#include <stdlib.h>
#include <float.h>
#include "mwutool.h"

//#include "lbfgs.h"


using namespace std;
using namespace HitZy;


map<int, int> map_pos_gold;
map<string, vector< int > > test_dict;
int g_debug = 0;
int g_decodingMethod = 0;
int g_context = 0;
int g_combine = 0;
string g_mwu ="mwu";
double g_filter_value = -1.0;
vector<tag_Options> g_all_result;
int g_event_split = 120000000;
//int g_event_split = 250000;

double result_fb_0 = 0;
double result_nb_0 = 0;
double result_fb_1 = 0;
double result_nb_1 = 0;
double result_fb_2 = 0;
double result_nb_2 = 0;
double result_fb_3 = 0;
double result_nb_3 = 0;
double result_fb_4 = 0;
double result_nb_4 = 0;

double threshold_fb_0 = 0;
double threshold_nb_0 = 0;
double threshold_fb_1 = 0;
double threshold_nb_1 = 0;
double threshold_fb_2 = 0;
double threshold_nb_2 = 0;
double threshold_fb_3 = 0;
double threshold_nb_3 = 0;
double threshold_fb_4 = 0;
double threshold_nb_4 = 0;


fstream file_0;
fstream file_1;
fstream file_2;
fstream file_3;
fstream file_4;


CPosMaxEn::CPosMaxEn(void)
{
	g_TagIDMapFileName = "model/"+APPLICATION+"/ME_TagIDMap_"+g_mwu;
	string title = "model/";
	if(g_context == 1)
	{
		m_fileNameModel = title+APPLICATION+"/MEModel_"+g_mwu+"_Con.bin";
	}
	else
	{
		m_fileNameModel = title+APPLICATION+"/MEModel_"+g_mwu+".bin";
	}

	m_fileNameModelOption = title+APPLICATION+"/MEModelOption.bin";

	options_array.resize(5000);
	
	m_BeginSample.word = START_WORD;
	m_BeginSample.tag = START_TAG;
	m_BeginSample.middle ="<DUMMY_CONTEXT>";

	m_EndSample.word = END_WORD;
	m_EndSample.tag = END_TAG;

	/*m_BeginSample.word = "";
	m_BeginSample.tag = "";
	m_BeginSample.middle ="<DUMMY_CONTEXT>";

	m_EndSample.word = "";
	m_EndSample.tag = "";*/

	
	m_Heap = new PriorHeap;
	m_NewHeap = new PriorHeap;

	//if I set g_Bigram true, I will add START_TAG to the taglist; 
	//g_Bigram is used mainly in CRF to decide Unigram and Bigram model. 
	g_Bigram = true;
}

CPosMaxEn::~CPosMaxEn(void)
{
	delete m_Heap;
	delete m_NewHeap;
}


/**
add sentence boundary, beyond boundary, set it as #B or #E! 
*/
void CPosMaxEn::FillInWnd(SAMPLE * pSample[], int index)
{
	pSample[WND_INDEX] = &m_vectSamples[index];
	for(int i = WND_INDEX-1;i>=0;i--)
	{
		if(m_vectSamples[index-(2-i)].word == "##")
		{
			for(int j = i;j>=0;j--)
			{
				pSample[j] =&m_BeginSample;
			}
			i = 0;
		}
		else
		{
			pSample[i] = &m_vectSamples[index-(2-i)];
		}
	}
	for(int i = WND_INDEX+1;i<WND_SIZE;i++)
	{
		if(m_vectSamples[index+(i-2)].word == "##")
		{
			for(int j = i;j<WND_SIZE;j++)
			{
				pSample[j] =&m_EndSample;
			}
			i = WND_SIZE;
		}
		else
		{
			pSample[i] = &m_vectSamples[index+(i-2)];
		}
	}
}


template<class T>
bool CPosMaxEn::GetFeatureFromTemplate(T &ft, SAMPLE * wnd[], string & templateStr)
{	
	vector<string> temp;
	String_SeperateToList_WithTrim(templateStr,temp,":");
	ft.predType = atoi(temp[0].c_str());
	string content = "";
	int pos_index,con_index;
	for(int i = 1;i<temp.size();i++)
	{
		AnalysizeTemplateString(temp[i],pos_index,con_index);
		
		if(con_index ==0 && wnd[pos_index+WND_INDEX]->word != "")
		{
			content+=wnd[pos_index+WND_INDEX]->word;
			content +=SPLIT_TAG;	
		}
		else if(con_index ==1 && wnd[pos_index+WND_INDEX]->middle =="")
		{
			content+=wnd[pos_index+WND_INDEX]->tag;
			content +=SPLIT_TAG;
		}

		else if(con_index ==1 /*&& wnd[WND_INDEX]->middle !=""*/&&wnd[pos_index+WND_INDEX]->middle!="")
		{
			content+=wnd[pos_index+WND_INDEX]->middle;
			content +=SPLIT_TAG;
		}
		else if(con_index ==2&& wnd[pos_index+WND_INDEX]->tag!="")
		{
			content+=wnd[pos_index+WND_INDEX]->tag;
			content +=SPLIT_TAG;
		}
		else
		{
			// for some bigram features [-2,1]:[-1,1], if one component [-1,1] don't exist, the 
			// whole feature will be empty, you can skip the other loop and jump out. 
			content = "";
			break;
		}	
	}
	if(content.length()>0)
	{
		content.erase(content.length()-1);
		ft.predData = content;
		return true;
	}
	return false;
}

void CPosMaxEn::ExtractFeatureFromWnd(SAMPLE * wnd[])
{

	FEATURE ft;
	ft.outTag = TagToID(wnd[WND_INDEX]->tag);

	for(int i = 0;i<m_templateVect.size();i++)
	{
		ft.predData = "";
		if(GetFeatureFromTemplate(ft, wnd,m_templateVect[i]))
		{
			LookupAndUpdateFeature(ft);
		}
	}
}

int CPosMaxEn::SaveModel(int AFileIndex)
{
	fstream f1;
	string mModel_FeatureName = File_FileName_AddIndex(m_fileNameModel, (int)g_sigma);
	mModel_FeatureName = File_FileName_AddIndex(mModel_FeatureName, AFileIndex);
	


	f1.open( mModel_FeatureName.c_str(),ios_base::out|ios_base::binary);
	VERIFY_FILE(f1);
	//////////////////////////////////////////////////
	
	vector<FEATURE>::iterator vIter;
	vector<FEATURE>::iterator vIterBegin = m_vectFeatures.begin();
	vector<FEATURE>::iterator vIterEnd = m_vectFeatures.end();

	for(vIter = vIterBegin;vIter!=vIterEnd;vIter++)
	{
            f1<<*vIter;
	}
	/////////////////////////////////////////////////
	f1.close();

/*	FILE * fp;
	string mModel_OptionFileName = File_FileName_AddIndex(m_fileNameModelOption, AFileIndex);
	fp = fopen(mModel_OptionFileName.c_str(), "wb");
	fwrite(&m_corr, sizeof(m_corr), 1, fp);
	fwrite(&m_maxFeatureCount, sizeof(m_maxFeatureCount), 1, fp);
	fwrite(&m_minFeatureCount, sizeof(m_minFeatureCount), 1, fp);
	fwrite(&m_correctFeatureE, sizeof(m_correctFeatureE), 1, fp);
	fwrite(&m_correctFeatureAlfa, sizeof(m_correctFeatureAlfa), 1, fp);
	fclose(fp);*/

	return 0;


}



int CPosMaxEn::SaveFeatures(string AFeatureName)
{
	fstream f1;
	f1.open( AFeatureName.c_str(),ios_base::out|ios_base::binary);
	VERIFY_FILE(f1);
	//////////////////////////////////////////////////
	
	vector<FEATURE>::iterator vIter;
	vector<FEATURE>::iterator vIterBegin = m_vectFeatures.begin();
	vector<FEATURE>::iterator vIterEnd = m_vectFeatures.end();

	for(vIter = vIterBegin;vIter!=vIterEnd;vIter++)
	{
		f1<<*vIter;
	}
	/////////////////////////////////////////////////
	f1.close();
	return 0;
}


void CPosMaxEn::ExtractEventFromWnd(SAMPLE * wnd[])
{
	EVENT ev;

	ev.outTag = TagToID(wnd[WND_INDEX]->tag);
	ev.count = 1;  

	int numPredicate = 0;
	PREDICATE predicate;
	int mLoc;

	for(int i = 0;i<m_templateVect.size();i++)
	{
		predicate.predData = "";
		if(GetFeatureFromTemplate(predicate, wnd,m_templateVect[i]))
		{
			mLoc = IndexOfPredicate(predicate);
			if (mLoc != -1)
			{
				ev.vectIndexPredicate.push_back(mLoc);
				numPredicate++;
			}
		}
	}

	if(numPredicate >0)
	{
		m_EventList.WriteEvent(ev);
	}
	else
	{
		cout<<"delete one event"<<endl;
	}
}

double CPosMaxEn::FeatureModelExpecation(double & log_likelihood)
{
	double pab[m_numTag];
    	int corr[m_numTag];  
    	int mPos=0, mNeg=0;
	/////////////////////////////////////////////////////////////
	
	EVENT mCurEvent;
	m_EventList.OpenEventList(OpenMode_Read);
	int sumEvent = 0;
	for(int i= 0;i<m_numTag;i++)
	{
		pab[i] = 0.0f;
		corr[i] = m_maxFeatureCount;
	}
	vector<int> pos_vect;
	int bi_array[m_numTag+10][10];
	
	//cout<<"beging to read each event"<<endl;
	for(int i= 0;i<m_numTag;i++)
		{
			pab[i] = 1.0;
			corr[i] = m_maxFeatureCount;
		}
	while (m_EventList.ReadEvent(mCurEvent) == true)
	{
		pos_vect.clear();
		sumEvent++;
		
		if(m_EventList.TotalEventCount()>1000000 && sumEvent% (m_EventList.TotalEventCount()/10) == 0)
		{
			cout<< "Training event = "<<sumEvent<<endl;
		}

		for(int i = 0;i<mCurEvent.vectIndexPredicate.size();i++)
		{			
			PREDICATE & mCurPredicate = m_vectPredicates[mCurEvent.vectIndexPredicate[i]];  //找到当前Predicate引用
			vector<pair<int,int> >::iterator s1_Iter = mCurPredicate.indexFeature.begin();
			while(s1_Iter != mCurPredicate.indexFeature.end())
			{	
				pab[s1_Iter->first]+=m_vectFeatures[s1_Iter->second].alpha;
				if(corr[s1_Iter->first]==m_maxFeatureCount)
				{
					pos_vect.push_back(s1_Iter->first);
					pab[s1_Iter->first]-=1.0;
				}
				bi_array[s1_Iter->first][m_maxFeatureCount-corr[s1_Iter->first]] = s1_Iter->second;
				corr[s1_Iter->first]--;	
				s1_Iter++;
			}
			//
		}

		///////////////
		double psum = 0.0;
		vector<int>::iterator it;
		for(it=pos_vect.begin();it!=pos_vect.end();it++)
		{
			int i = *it;
			pab[i] = exp(pab[i]);
			psum += pab[i];
		}
		psum += (m_numTag-pos_vect.size());
		
		log_likelihood -= log(pab[mCurEvent.outTag]/psum);
		
		for(it=pos_vect.begin();it!=pos_vect.end();it++)
		{
			int k = *it;
			for(int j = 0;j<(m_maxFeatureCount-corr[k]);j++)
			{
				int point = bi_array[k][j];
				m_model_expectation[point] +=pab[k]/psum;
			}
			pab[k] = 1.0;
			corr[k] = m_maxFeatureCount;
		}
	}
	m_EventList.CloseEventList();

	return (double) 0 ;

}

double CPosMaxEn::GIS(double& likelihood)
{

	double log_likelihood;

	double prevision = FeatureModelExpecation(likelihood);

	////////////////////////////////////////////////////////////////
	//update lamda of each feature. 
	double m_inv_maxFeatureCount = 1.0f / m_maxFeatureCount;
	for (int k = 0; k < m_vectFeatures.size(); k ++)
	{
		FEATURE & mCurFeature = m_vectFeatures[k];
	
		if(m_model_expectation[k]>0.0f)
		{
			mCurFeature.alpha += m_inv_maxFeatureCount * (m_feature_count_log[k] - log(m_model_expectation[k]));
		}
	}

	fill(m_model_expectation.begin(),m_model_expectation.end(),0.0f);

	return prevision;
}


int CPosMaxEn::LoadPreTriggerPair(void)
{
	const string mFileName = "PosMaxEnData\\PreTriggerPair.txt";
	if (File_Exist(mFileName) == false)
	{
		cout << "Can not open file :" << mFileName << endl;
		return -1;
	}
	fstream f1;
	f1.open( mFileName.c_str(),ios_base::in);
	//VERIFY_FILE(f1);
	string A;
	string B;
	string pos;
	string key;
    while(true)
	{	
		
		f1>>A;
		if(f1.eof())
		{
			break;
		}
		f1>>B;
		f1>>pos;
		key = A+"+"+B+"+"+pos;
		m_hashPreTriggers.insert(key);
	}
	f1.close();
	return 0;
}

int CPosMaxEn::LoadSucTriggerPair(void)
{
	const string mFileName = "PosMaxEnData\\SucTriggerPair.txt";
	if (File_Exist(mFileName) == false)
	{
		cout << "Can not open file :" << mFileName << endl;
		return -1;
	}
	fstream f1;
	f1.open( mFileName.c_str(),ios_base::in);
	//VERIFY_FILE(f1);
	string A;
	string B;
	string pos;
	string key;
    while(true)
	{	
		
		f1>>A;
		if(f1.eof())
		{
			break;
		}
		f1>>B;
		f1>>pos;
		key = A+"+"+B+"+"+pos;
		m_hashSucTriggers.insert(key);
	}
	f1.close();
	return 0;
}

bool CPosMaxEn::FindInPreTriggerPairs(string& key)
{
	if(m_hashPreTriggers.find(key)!=m_hashPreTriggers.end())
	{
		return true;
	}
	return false;
}

bool CPosMaxEn::FindInSucTriggerPairs(string& key)
{
	if(m_hashSucTriggers.find(key)!=m_hashSucTriggers.end())
	{
		return true;
	}
	return false;
}



void CPosMaxEn::JHeapClear(PriorHeap *AHeap)
{   //
	while (AHeap->size() > 0)
	{
		AHeap->pop();
	}
}

int CPosMaxEn::LoadDict(void)
{
	FILE* fsample;
	string fileName = "model/"+APPLICATION+"/dict";
	fsample = fopen(fileName.c_str(), "rt");  //

	if (fsample == NULL)
	{
		cout << "Cannot open sample file:" << endl;
		cout << "Cannot continue run...." << endl;
		throw ("Cannot open sample file");
		return -1;
	}

	string word = "";	
	while (! feof(fsample))
	{
		char line[10000];
		fgets(line,10000,fsample);
		string sline;
		sline.assign(line);
		
		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");
		word = vectline[0];
		
		int numPos= atoi(vectline[1].c_str());
		vector<int> posVector;
		for(int i = 0;i<numPos ;i++)
		{
			string pos = vectline[2+i];
			if(m_mapPosID.find(pos) != m_mapPosID.end())
			{
				posVector.push_back(TagToID(pos));
			}
		}
		pair<string, vector<int> > pairT;
		pairT.first = word;
		pairT.second = posVector;
		test_dict.insert(pairT);		
	}
	////////////////////////////////////////////////
	fclose(fsample);

	cout << "Read Dict OK" << endl;	
	return 1;

}



int CPosMaxEn::Continue_TrainModel(string& fileName,int con_num)
{
	////////////////////////////////////////////
	//the first step;
	GetPosList(fileName);
	m_numTag = LoadPosTag(g_TagIDMapFileName);
	cout<<"the number of tag is "<<m_numTag<<endl;
	cout<<"The TagIDMap.txt has been built"<<endl;
	
	//load template file
	LoadTemplate(g_templateFileName);

	///////////////////////////////////////////////////////
	//the second step
	LoadModel(con_num);
	const string mFilterFeatureName = "model/"+APPLICATION+"/FilterFeature.bin"; 
	SaveFeatures(mFilterFeatureName);
	
	GetPredictsFromFeatures();

	cout << "Unload Features ...." ;
	m_vectFeatures.clear();
	
	cout << " OK " << endl;
	
	//m_EventList.SetEventOperatorMode(OperatorMode_File); 	
	////////////////////////////////////////////////////////////
	//the fourth step
	CollectEvents(fileName);	

	
	LoadFeatures(mFilterFeatureName, false);  
	cout << " OK " << endl;
	cout << "Begin training ..... " << endl;
	
//	while(true)
	double min = 0.0,na,delta;
	double b = 0.0;
	for(int j=0; j<100; j++)
	{
		//na = GIS();
		
		cout<<"the "<<j<<" na="<<na<<endl;
		delta = na-b;
		cout<<"第"<<j<<"迭代"<<"  精度："<<delta<<endl;
		b = na;
		if( delta<=min )
		{
			cout<<"The Model has reached at the precision"<<endl;
			cout<<"The times of iteration is:  "<<j+1<<endl; 
			break;
		}

		
		if ((j + 1) % 3 == 0) {SaveModel(con_num+j + 1);}
	}
	cout << "Save Model....." ;
	SaveModel(0);   
	SaveFeaturesToTxt("model/"+APPLICATION+"/LastFeatures.txt");
	cout << " OK " << endl;
	return 0;
	
	
}
int CPosMaxEn::GetTagFromEvents()
{
	EVENT mCurEvent;
	m_EventList.OpenEventList(OpenMode_Read);
	map<int,int> ZyTagMap;
	while (m_EventList.ReadEvent(mCurEvent) == true)
	{
		if(ZyTagMap.find(mCurEvent.outTag) == ZyTagMap.end())
		{
			ZyTagMap.insert(pair<int,int> (mCurEvent.outTag,1));
		}
	}
	m_EventList.CloseEventList();
	return ZyTagMap.size();
}


int CPosMaxEn::TrainModel(string& fileName)
{

	cout << "Training......" << endl;
	////////////////////////////////////////////
	//Get Tag and ID map and load template file
	GetPosList(fileName);
	m_numTag = LoadPosTag(g_TagIDMapFileName);
	cout<<"the number of tag is "<<m_numTag<<endl;
	LoadTemplate(g_templateFileName);
//  GetDict(fileName);
	cout<<"************************************************"<<endl;


	///////////////////////////////////////////////////////
	//Get and filter all the features from training corpora
	ReadSamplesAndCollectFeature(fileName);
	FilterFeatures();	
	const string mFilterFeatureName = "model/"+APPLICATION+"/FilterFeature.bin"; 
	SaveFeatures(mFilterFeatureName);
	if(g_context == 1)
		SaveFeaturesToTxt("model/"+APPLICATION+"/Features_Con.txt");
	SaveFeaturesToTxt("model/"+APPLICATION+"/Features.txt");
	//output the text and binary format of features. you can check features in text file manually. 
	cout<<"The Features.txt has been built"<<endl;
	cout<<"************************************************"<<endl;
	  
	////////////////////////////////////////////////////////////
	//Get predicts from Features 
	GetPredictsFromFeatures();
	cout<<"************************************************"<<endl;

	////////////////////////////////////////////////////////////
	//Get Event from training corpora
	
	CollectEvents(fileName);
	cout<<"Collecting Events is over"<<endl;
	cout<<"************************************************"<<endl;
	
	//cout<<"number of tag from event is"<<GetTagFromEvents()<<endl;

	////////////////////////////////////////////////
	//get the m_maxFeatureCount through loop all events 	
	//GetCorrectionOptions();
 

	///////////////////////////////////////////////////
	//selection of training method
	if(g_gis)
	{
		GIS_Estimate();
	}	
	else
	{
		//LBFGS_Estimate();
		LBFGS_Estimate_C();
	}
	
	
	cout << "Save the last model" ;
	//SaveModel(0);   
	SaveFeaturesToTxt("model/"+APPLICATION+"/LastFeatures.txt");
	cout << "Training is ok" << endl;
	return 0;
}

void CPosMaxEn::LBFGS_Estimate_C(void)
{
	cout << "Begin LBFGS training ..... " << endl;
	int n = m_vectFeatures.size();
	double* g = new double[n]; 
	double* x = new double[n];
	for(int j = 0;j<n;j++)
	{
		x[j] = m_vectFeatures[j].alpha ;
	}	
	int m = 5;
	double f;
	int correct; 


	cout<<"num"<<"\t"<<"log-likelihood"<<"\t"<<"accuracy"<<endl;

	int flag_LBFGS = 0;
	int j = 0;

	m_numFeatures = m_vectFeatures.size();
	int hessian = 5;
	int ws_size = m_numFeatures * (2 * hessian + 1) + 2 * hessian;
    	double* ws = new double[ws_size];
	double* diag = new double[m_numFeatures];
	int * iprint = new int[2];
	int iflag  = 0;

	int acc_max = 0;
	int acc_file_index = 0;

	for(j=0; j<g_iteration; j++)
	{
		f = 0.0;
		correct = 0;
		//cout<<"memory has been allocated"<<endl;
		double acc = FeatureModelExpecation(f);

		cout<<j<<"\t";
		cout<<setw(10)<<-1*f<<"\t";
		cout<<acc<<endl;
		
		for(int index = 0;index<n;index++)
		{
			g[index] = m_model_expectation[index] - m_feature_count[index];
		}

		if(g_sigma>0.0f)
		{
			for(int index = 0;index<n;index++)
			{
				double penality = x[index]/g_sigma;
				g[index] += penality;
				f+=(penality*x[index])/2;
			}
		}
		
		
		double xtol = DBL_EPSILON; // machine precision
		double eps_for_convergence = 0.00001;
		int diagco = 0;
		
		lbfgs(&m_numFeatures, &hessian, x, &f, g, &diagco, diag, iprint, &eps_for_convergence, &xtol, ws, &iflag);
    

		 if (iflag < 0) {
          		 
			  cout<<"lbfgs routine stops with an error"<<endl;
			  flag_LBFGS = 1;
			  break;
          		//  throw runtime_error("lbfgs routine stops with an error");
      		  } else if (iflag == 0) {
         	 	  cout<<"Training terminats succesfully i"<<endl;
			flag_LBFGS = 2;
           		 break;
		}
		fill(m_model_expectation.begin(),m_model_expectation.end(),0.0f);

		for(int index = 0;index<n;index++)
		{
			m_vectFeatures[index].alpha = x[index];
		}	

		if ((j + 1) % 20 == 0 && g_syn_flag == 1) 
		{
			cerr<<" iter_num="<<j+1<<" ";
			
			if(g_decodingMethod == 0 ||g_decodingMethod == 1)
			{
				TagFile(g_testingFileName,j+1);
			}
			if(g_decodingMethod == 2||g_decodingMethod == 3 ||
			g_decodingMethod == 4||g_decodingMethod == 5 )
			{
				FB_FilterFile(g_testingFileName,j+1);
			}

			if(g_right>acc_max)
			{
				acc_max = g_right;
				acc_file_index = j+1;

				int f_index = int(g_sigma)+1;
				string mPosFileName = File_FileName_AddSuffix(g_testingFileName, "_ME"); 
				string oldf = File_FileName_AddIndex(mPosFileName, j+1); 
				string newf = File_FileName_AddIndex(mPosFileName, f_index); 
				//rename(oldf.c_str(),newf.c_str());
				//SaveModel(f_index);
			}
			m_numTag = LoadPosTag(g_TagIDMapFileName);
			SaveModel(j+1);
		}
	}
	
	int f_index = int(g_sigma)+1;
	string mPosFileName = File_FileName_AddSuffix(g_testingFileName, "_ME"); 
	string oldf = File_FileName_AddIndex(mPosFileName, j+1); 
	string newf = File_FileName_AddIndex(mPosFileName, f_index); 
	rename(oldf.c_str(),newf.c_str());

	
	if (flag_LBFGS> 0 && g_syn_flag == 1)
	{ 
		cerr<<" iter_num="<<j+1<<" ";
			
		if(g_decodingMethod == 0 ||g_decodingMethod == 1)
		{
			TagFile(g_testingFileName,j+1);
		}
		if(g_decodingMethod == 2 || g_decodingMethod == 3 ||
		g_decodingMethod == 4 || g_decodingMethod == 5 )
		{
			FB_FilterFile(g_testingFileName,j+1);
		}
	}
	if(flag_LBFGS == 1)
	{
		cerr<<"g_templateFileName="<<g_templateFileName;
		cerr<<" c="<<g_threshold;
		cerr<<" g="<<g_sigma;
		cerr<<" iter_num="<<j+1;
		cerr<<" "<<-1<<endl;
	}
	delete [] g;
	delete [] ws;
	delete [] diag;
	delete [] iprint;
}



void CPosMaxEn::GIS_Estimate(void)
{
	cout << "Begin GIS training ..... " << endl;
	double min = 0.0,acc,likelihood;

	cout<<"num"<<"\t"<<"log-likelihood"<<"\t"<<"accuracy"<<endl;

	for(int j=0; j<g_iteration; j++)
	{	
		acc = GIS(likelihood);
		
		cout<<j<<"\t";
		cout<<setw(10)<<-1*likelihood<<"\t";
		cout<<acc<<endl;
	
		if ((j + 1) % 20 == 0 && g_syn_flag == 1) 
		{
			g_right = 0;
			g_sum = 0;
			TagFile(g_testingFileName,j+1);
			m_numTag = LoadPosTag(g_TagIDMapFileName);
			cerr<<"g_templateFileName="<<g_templateFileName;
			cerr<<" c="<<g_threshold;
			cerr<<" g="<<g_sigma;
			cerr<<" iter_num="<<j+1;
			cerr<<" "<<double(g_right*100)/g_sum<<endl;
		}
	}
}


int CPosMaxEn::CollectEvents(string& fileName)
{
	cout<<"Collect Events .........."<<endl;
	SAMPLE* wnd[WND_SIZE];
	for (int k = 0; k < WND_SIZE; k ++)
	{
		wnd[k] = 0;
	}

	// 6000000 means that the training file is big enough, we can't load all events into the memory. we use file on disk to store all events. 
	if (g_sumTrainLine > g_event_split) { m_EventList.SetEventOperatorMode(OperatorMode_File); }  
	
	m_EventList.OpenEventList(OpenMode_Write);  
	int mNewEventCount = 0;   

	fstream fsample;
	fsample.open(fileName.c_str(),ios::in);
	VERIFY_FILE(fsample);

	SAMPLE mSample;
	m_vectSamples.clear();
	m_vectSamples.push_back(m_BeginSample);
	m_vectSamples.push_back(m_BeginSample);
	
	int sumTrainLine = 0;
	do 
	{
		sumTrainLine ++;
		if(sumTrainLine % 10000 == 0)
		{
			cout << ".";
			cout.flush();
		}
		if(sumTrainLine % 100000 == 0)
		{
			cout << sumTrainLine;
			cout.flush();
		}
		char line[1000];
		fsample.getline(line,1000);
		string sline;
		sline.assign(line);
		if(sline == ""||sline == "\n")
		{
			continue;
		}
		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");

		
		mSample.word = vectline[0];
		
		if(mSample.word.substr(0,3) == SEN_SPLIT)
		{
			m_vectSamples.push_back(m_EndSample);
			m_vectSamples.push_back(m_EndSample);
			for(int i = 0;i<m_vectSamples.size()-4;i++)
			{
				wnd[0] = &m_vectSamples[i];
				wnd[1] = &m_vectSamples[i+1];
				wnd[2] = &m_vectSamples[i+2];
				wnd[3] = &m_vectSamples[i+3];
				wnd[4] = &m_vectSamples[i+4];		
				{
					ExtractEventFromWnd(wnd);
				}
			}
			
			m_vectSamples.clear();	
			m_vectSamples.push_back(m_BeginSample);
			m_vectSamples.push_back(m_BeginSample);
		}
		else
		{
			if(vectline.size()>2)
			{
				mSample.middle = vectline[vectline.size()-2];
			}
			mSample.tag = vectline[vectline.size()-1];
			m_vectSamples.push_back(mSample);
		}
		
	}while(!fsample.eof());
	////////////////////////////////////////////////
	fsample.close();
	
	if(m_vectSamples.size()>0)
	{
		m_vectSamples.push_back(m_EndSample);
		m_vectSamples.push_back(m_EndSample);
		for(int i = 0;i<m_vectSamples.size()-4;i++)
		{
			wnd[0] = &m_vectSamples[i];
			wnd[1] = &m_vectSamples[i+1];
			wnd[2] = &m_vectSamples[i+2];
			wnd[3] = &m_vectSamples[i+3];
			wnd[4] = &m_vectSamples[i+4];
			{
				ExtractEventFromWnd(wnd);
			}
		}
	}
	m_EventList.CloseEventList();
	cout << "Events Count is " << m_EventList.TotalEventCount() << endl;
	cout<<"Collect Events is ok"<<endl;

	return 0;
}

int CPosMaxEn::ReadSamplesAndCollectFeature(string& fileName)
{
	cout << "collecting features...... " << endl;
	
	fstream fsample;
	fsample.open(fileName.c_str(),ios::in);
	VERIFY_FILE(fsample);

	SAMPLE mSample;
	m_vectSamples.push_back(m_BeginSample);
	m_vectSamples.push_back(m_BeginSample);
	SAMPLE* wnd[WND_SIZE];
	g_sumTrainLine = 0;
	do 
	{
		g_sumTrainLine ++;
		if(g_sumTrainLine % 10000 == 0)
		{
			cout << ".";
			cout.flush();
		}
		if(g_sumTrainLine % 100000 == 0)
		{
			cout <<g_sumTrainLine;
			cout.flush();
		}
		char line[1000];
		fsample.getline(line,1000);
		string sline;
		sline.assign(line);
		if(sline == ""||sline == "\n")
		{
			continue;
		}
		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");

		
		mSample.word = vectline[0];
		if(mSample.word.substr(0,3)==SEN_SPLIT)
		{
			m_vectSamples.push_back(m_EndSample);
			m_vectSamples.push_back(m_EndSample);
			for(int i = 0;i<m_vectSamples.size()-4;i++)
			{
				wnd[0] = &m_vectSamples[i];
				wnd[1] = &m_vectSamples[i+1];
				wnd[2] = &m_vectSamples[i+2];
				wnd[3] = &m_vectSamples[i+3];
				wnd[4] = &m_vectSamples[i+4];
				{
					ExtractFeatureFromWnd(wnd);
				}
			}
			
			m_vectSamples.clear();	
			m_vectSamples.push_back(m_BeginSample);
			m_vectSamples.push_back(m_BeginSample);
		}
		else
		{
			if(vectline.size()>2)
			{
				mSample.middle = vectline[vectline.size()-2];
			}
			mSample.tag = vectline[vectline.size()-1];
			m_vectSamples.push_back(mSample);
		}
		
	}while(!fsample.eof());
	////////////////////////////////////////////////
	fsample.close();

	//deal with left part in the end of file 
	if(m_vectSamples.size()>0)
	{
		m_vectSamples.push_back(m_EndSample);
		m_vectSamples.push_back(m_EndSample);
		for(int i = 0;i<m_vectSamples.size()-4;i++)
		{
			wnd[0] = &m_vectSamples[i];
			wnd[1] = &m_vectSamples[i+1];
			wnd[2] = &m_vectSamples[i+2];
			wnd[3] = &m_vectSamples[i+3];
			wnd[4] = &m_vectSamples[i+4];
			{
				ExtractFeatureFromWnd(wnd);
			}
		}
	}

	cout << "collecting features is OK" << endl;
	return 0;

}


int CPosMaxEn::GetDict(string& fileName)
{
	cout << "Get Dictionary...... " << endl;
	
	fstream fsample;
	fsample.open(fileName.c_str(),ios::in);
	VERIFY_FILE(fsample);

	string word;
	string tag;
	int sumTrainLine = 0;
	do 
	{
		sumTrainLine ++;
		if(sumTrainLine % 10000 == 0)
		{
			cout << ".";
			cout.flush();
		}
		if(sumTrainLine % 100000 == 0)
		{
			cout <<sumTrainLine;
			cout.flush();
		}
		char line[1000];
		fsample.getline(line,1000);
		string sline;
		sline.assign(line);
		if(sline == ""||sline == "\n")
		{
			continue;
		}
		vector<string> vectline;
		if(sline.substr(0,3)!=SEN_SPLIT)
		{
			String_SeperateToList_WithTrim(sline,vectline," ");
			word = vectline[0];
			tag = vectline[vectline.size()-1];
			if(test_dict.find(word) != test_dict.end())
			{
				vector<int> &pos_list = test_dict[word];
				bool exist = false;
				for(int i = 0;i<pos_list.size();i++)
				{
					if(pos_list[i] == TagToID(tag))
					{
						exist = true;
						break;
					}
				}
				if(exist == false)
				{
					pos_list.push_back(TagToID(tag));
				}
			}
			else
			{
				pair<string, vector<int> > pairT;
				pairT.first = word;
				vector<int> pos_list;
				pos_list.push_back(TagToID(tag));
				pairT.second = pos_list;
				test_dict.insert(pairT);	
			}
		}
		
	}while(!fsample.eof());
	////////////////////////////////////////////////
	fsample.close();


	FILE* fdict;
	string fileNameDict = "model/"+APPLICATION+"/dict";
	fdict = fopen(fileNameDict.c_str(), "w");
	map<string,vector<int> >::iterator s1_Iter; 
	s1_Iter = test_dict.begin( );
	while(s1_Iter != test_dict.end())
	{	
		pair<string, vector<int> > pairT = *s1_Iter;
		fprintf(fdict, "%s ",  pairT.first.c_str());
		vector<int> & pos_l = pairT.second;

		fprintf(fdict, "%d ",  pos_l.size());
		int i = 0;
		for(;i<pos_l.size()-1;i++)
		{
			fprintf(fdict, "%s ",  IDToTag(pos_l[i]).c_str());
		}
		//the last POS tag don't need follow a space but a newline. 
		fprintf(fdict, "%s\n",  IDToTag(pos_l[i-1]).c_str());
		s1_Iter++;
	}
	fclose(fdict);
	cout << "Get Dictionary is OK" << endl;
	return 0;

}





int CPosMaxEn::LoadFeatures(string AFeatureName, bool AMakeHash)
{
	
	FILE * fp;
	fp = fopen(AFeatureName.c_str(), "r");
	if (fp == NULL) return -1;  
	fclose(fp);
	//
   	 fstream f1;
	f1.open( AFeatureName.c_str(),ios_base::in|ios_base::binary);
	VERIFY_FILE(f1);
	
    while(true)
	{	
		FEATURE fTemp;
		f1>>fTemp;
		if(!f1.eof())
		{
			m_vectFeatures.push_back(fTemp);
		}
		else
		{
			break;
		}
	}
	f1.close();

	if (AMakeHash == true)  
	{
		vector<FEATURE>::iterator vIter;
		vector<FEATURE>::iterator vIterBegin = m_vectFeatures.begin();
		vector<FEATURE>::iterator vIterEnd = m_vectFeatures.end();

		int index = 0;
		for(vIter = vIterBegin;vIter!=vIterEnd;vIter++)
		{
			char buffer[20];
			sprintf(buffer, "%d",vIter->predType);
			
			
			//add them to the m_hashFeatures
			string key(buffer);
			key+=SPLIT_TAG;
			key+=vIter->predData;
			key+=SPLIT_TAG;
			sprintf(buffer, "%d",vIter->outTag);
			//_itoa(vIter->outTag,buffer,10);
			string temp(buffer);
			key+=temp;
			typedef pair <string, int> Int_Pair;
			m_hashFeatures.insert ( Int_Pair ( key, index));
			index++;
		}
	}
	///////////////////////////////////////////////////////////////////		
	
	return 0;
}

int CPosMaxEn::LoadModel(int AFileIndex)
{
	//string mModel_FeatureName = File_FileName_AddIndex(m_fileNameModel, AFileIndex);

	string mModel_FeatureName = File_FileName_AddIndex(m_fileNameModel, (int)g_sigma);
	mModel_FeatureName = File_FileName_AddIndex(mModel_FeatureName, g_iteration);


	if (File_Exist(mModel_FeatureName) == false)
	{
		cout << "Model File Does not Exist:" << mModel_FeatureName << endl;
		cout << "Load Model Fail." << endl;
        return -1;
	}
	

	fstream f1;
	f1.open( mModel_FeatureName.c_str(),ios_base::in|ios_base::binary);
	VERIFY_FILE(f1);


	m_vectSamples.clear();

	m_vectFeatures.clear();
	m_hashFeatures.clear();

	m_vectPredicates.clear();
	m_hashPredicates.clear();

	
   	 while(true)
	{	
		FEATURE fTemp;
		f1>>fTemp;
		if(!f1.eof())
		{
			m_vectFeatures.push_back(fTemp);
		}
		else
		{
			break;
		}
	}
	f1.close();

	

	vector<FEATURE>::iterator vIter;
	vector<FEATURE>::iterator vIterBegin = m_vectFeatures.begin();
	vector<FEATURE>::iterator vIterEnd = m_vectFeatures.end();

	int index = 0;
	for(vIter = vIterBegin;vIter!=vIterEnd;vIter++)
	{
		char buffer[20];
		sprintf(buffer, "%d", vIter->predType);
		//_itoa(vIter->predType,buffer,10);
		//add them to the m_hashFeatures
		string key(buffer);
		key+=SPLIT_TAG;
		key+=vIter->predData;
		key+=SPLIT_TAG;
		sprintf(buffer, "%d", vIter->outTag);
		//_itoa(vIter->outTag,buffer,10);
		string temp(buffer);
		key+=temp;
		typedef pair <string, int> Int_Pair;
		m_hashFeatures.insert ( Int_Pair ( key, index));
		index++;
	}
	///////////////////////////////////////////////////////////////////		
	/*FILE * fp;
	string mModel_OptionFileName = File_FileName_AddIndex(m_fileNameModelOption, AFileIndex);
	fp = fopen(mModel_OptionFileName.c_str(), "rb");
	fread(&m_corr, sizeof(m_corr), 1, fp);
	fread(&m_maxFeatureCount, sizeof(m_maxFeatureCount), 1, fp);
	fread(&m_minFeatureCount, sizeof(m_minFeatureCount), 1, fp);
	fread(&m_correctFeatureE, sizeof(m_correctFeatureE), 1, fp);
	fread(&m_correctFeatureAlfa, sizeof(m_correctFeatureAlfa), 1, fp);
	fclose(fp);*/

	//////////////////////////////////////////////////////////////////////

	return 0;

	
}




void CPosMaxEn::SaveSamplesToTxt(string AFileName)
{
	FILE *fp;
	fp = fopen(AFileName.c_str(), "w");
	if (fp == NULL)
	{
		cout << "Debug Error:Cannot create file : " << AFileName << endl;
		exit(0);;
	}

	unsigned int k;
	for (k = 0 ; k < m_vectSamples.size(); k ++)
	{
		fprintf(fp, "%s  %s\n", m_vectSamples[k].word.c_str(), m_vectSamples[k].tag.c_str());
	}

	fclose(fp);

}

int CPosMaxEn::GetPredictsFromFeatures(void)
{
	//create m_vectPredicate and m_hashPredicate
	cout<<"Collecting predicts from feature......"<<endl;
	vector<FEATURE>::iterator vIter;
	vector<FEATURE>::iterator vIterBegin = m_vectFeatures.begin();
	vector<FEATURE>::iterator vIterEnd = m_vectFeatures.end();

	int index = 0;
	
	for(vIter = vIterBegin;vIter!=vIterEnd;vIter++)
	{	
		if(vIter->predData != "") //for eade feature in CRF, the predData is empty
		{
			char buffer[20];
			sprintf(buffer,"%d",vIter->predType);
			
			string key(buffer);
			key+=SPLIT_TAG;
			key+=vIter->predData;
	
			if(m_hashPredicates.find(key)!=m_hashPredicates.end())
			{
				int hash_index = m_hashPredicates[key];
				PREDICATE &temp = m_vectPredicates[hash_index];
				pair<int,int> pValue;
				pValue.first = vIter->outTag;
				pValue.second = index;
				temp.indexFeature.push_back(pValue);				
			}
			else
			{
				PREDICATE temp;
				temp.predType = vIter->predType;
				temp.predData = vIter->predData;
	
				pair<int,int> pValue;
				pValue.first = vIter->outTag;
				pValue.second = index;
				temp.indexFeature.push_back(pValue);

				m_vectPredicates.push_back(temp);
				int value = (int)m_vectPredicates.size()-1;	
				typedef pair <string, int> Int_Pair;
				m_hashPredicates.insert(Int_Pair(key,value));
			}
			index++; 	
		}
	}	

	cout<<"the size of Predicates is "<<m_vectPredicates.size()<<endl;
	cout<<"Collecting predicts is ok"<<endl;

	return 0;
}


void CPosMaxEn::SaveFeaturesToTxt(string AFileName)
{
	fstream f1;
	f1.open(AFileName.c_str(),ios::out);
	VERIFY_FILE(f1);
//f1 <<"predType\tpredData\toutTag\tcount\tE\talpha\tmod"<<endl;

	unsigned int k;
	for (k = 0; k < m_vectFeatures.size(); k ++)
	{
//		f1 << m_vectFeatures[k] << endl;
		f1 <<m_vectFeatures[k].predType << "\t" <<m_vectFeatures[k].predData ;
		int length = m_vectFeatures[k].predData.length();
		int numTab = length/8;
		for(;numTab <5;numTab++)
		{
			f1<<"\t";
		}
		f1<<IDToTag(m_vectFeatures[k].outTag);
		length = IDToTag(m_vectFeatures[k].outTag).length();
		numTab = length/8;
		for(;numTab <5;numTab++)
		{
			f1<<"\t";
		}
		/*f1<<IDToTag(m_vectFeatures[k].outTag);
		length = IDToTag(m_vectFeatures[k].outTag).length();
		numTab = length/8;
		for(;numTab <5;numTab++)
		{
			f1<<"\t";
		}*/		

		f1<<m_vectFeatures[k].alpha<<"\t"<<m_feature_count[k]<<endl;
		
	}
	///////////////////////////////////////////////////////////////////		
	f1.close();

}

void CPosMaxEn::SavePredicatesToTxt(string AFileName)
{
	FILE * fp;
	fp = fopen(AFileName.c_str(), "w");
	if (fp == NULL)
	{
		cout << "cannot create Predicate out file : " << AFileName << endl;
		cout << "application terminated" << endl;
		exit(1);
	}

	/////////////////
	unsigned int k;
	for (k = 0 ; k < m_vectPredicates.size(); k ++)
	{
		fprintf(fp, "%d\t%s", m_vectPredicates[k].predType, m_vectPredicates[k].predData.c_str());
		//for (int t = 0 ; t < m_numTag; t ++)
		//{
		//	fprintf(fp, "\t%d", m_vectPredicates[k].indexFeature[t]);
		//}
		
		fprintf(fp, "\n");
	}
	/////////////////
	fclose(fp);
}


void CPosMaxEn::LookupAndUpdateFeature(FEATURE &AFeature)
{

	char buffer[50];
	sprintf(buffer, "%d",AFeature.predType);
	//_itoa(AFeature.predType,buffer,10);

		//add them to the m_hashFeatures
	string key(buffer);
	key+=SPLIT_TAG;
	key+=AFeature.predData;
	key+=SPLIT_TAG;
	sprintf(buffer, "%d",AFeature.outTag);
	string temp(buffer);
	key+=temp;
	if(AFeature.previousTag != -1)
	{
		key+=SPLIT_TAG;
		sprintf(buffer, "%d",AFeature.previousTag);
		temp.assign(buffer);
		key+=temp;
	}
	
	if(m_hashFeatures.find(key)!=m_hashFeatures.end())  
	{
		int index = m_hashFeatures[key];
		FEATURE &temp = m_vectFeatures[index];
		m_feature_count[index]++;
	}
	else
	{
		m_vectFeatures.push_back(AFeature);
		m_feature_count.push_back(1);
		int value = (int)m_vectFeatures.size() - 1; 
		m_hashFeatures.insert(pair<string,int>(key,value));
	}
}




bool CPosMaxEn::GetCorrectionOptions(void)  
{   
	EVENT mT;
	m_EventList.OpenEventList(OpenMode_Read);  

	if (m_EventList.ReadEvent(mT) == true)
	{
        	m_maxFeatureCount = mT.vectIndexPredicate.size();
		m_minFeatureCount = m_maxFeatureCount;
	}
	else
	{
		cout << "Do not Exist Event in GetCorrectionOptions()" << endl;
		m_EventList.CloseEventList();
		return false;
	}

	while (m_EventList.ReadEvent(mT) == true)
	{
		int nSize = mT.vectIndexPredicate.size();
		if(nSize>m_maxFeatureCount)
		{
			m_maxFeatureCount = nSize;
		}
		if(nSize<m_minFeatureCount)
		{
			m_minFeatureCount = nSize;
		}
	}

		

	int mCorrectCount = 0; 
	m_EventList.OpenEventList(OpenMode_Read);
	while (m_EventList.ReadEvent(mT))
	{
		mCorrectCount+= (m_maxFeatureCount - mT.vectIndexPredicate.size()) * mT.count;
	}
	m_correctFeatureE = log((double)mCorrectCount);
	m_correctFeatureAlfa = 0.0f; 
 
	m_EventList.CloseEventList();
	return true;
}


int CPosMaxEn::LookUpFeature(FEATURE& AFeature)
{
	char buffer[20];
	sprintf(buffer,"%d",AFeature.predType);
	//_itoa(AFeature.predType,buffer,10);

	string key(buffer);
	key+=SPLIT_TAG;
	key+=AFeature.predData;
	key+=SPLIT_TAG;
	sprintf(buffer,"%d",AFeature.outTag);
	//_itoa(AFeature.outTag,buffer,10);
	string temp(buffer);
	key+=temp;

	if(m_hashFeatures.find(key)!=m_hashFeatures.end()) //如果找到
	{
		return m_hashFeatures[key];
	}
	else
	{
		return -1;
	}
}




int CPosMaxEn::IndexOfPredicate(PREDICATE &APre_Type)  
{
	

	char buffer[20];
	sprintf(buffer,"%d",APre_Type.predType);
	

		//add them to the m_hashFeatures
	string key(buffer);
	key+=SPLIT_TAG;
	key+=APre_Type.predData;

	int index = -1;   
	if(m_hashPredicates.find(key)!=m_hashPredicates.end())  
	{
		index = m_hashPredicates[key];
	}
	return index;
}
int CPosMaxEn::ForceTagToID(string & pos)
{
	int ID = TagToID(pos);
	if(ID>=0)
	{
		return ID ;
	}
	else
	{
		pair<string,int> pairPosID;
		pair<int,string> pairPosTag;

		pairPosID.first = pos;
		pairPosID.second = m_numTag;

		pairPosTag.first = m_numTag++;
		pairPosTag.second = pos;

		m_mapPosID.insert(pairPosID);
		m_mapPosTag.insert(pairPosTag);
		return 	pairPosID.second;
	}
}

int CPosMaxEn::FillInTagMap(int lineNum)
{
	pair<int, vector<int> > Pair;
	vector<int> p_o;

	Pair.first = 0;
	p_o.clear();
	p_o.push_back(TagToID(START_TAG));
	Pair.second = p_o;
	map_pos_options.insert(Pair);
	Pair.first = 1;
	map_pos_options.insert(Pair);
	
	
	for(int i = 0;i<lineNum;i++)
	{
		p_o.clear();
		for(int j = 0;j<options_array[i].size();j++)
		{
			string pos_cur = options_array[i][j];
			p_o.push_back(ForceTagToID(pos_cur));	
		}
		
		Pair.first = 2+i;
		Pair.second = p_o;
		map_pos_options.insert(Pair);
	}

	Pair.first = lineNum+2;
	p_o.clear();
	p_o.push_back(TagToID(END_TAG));
	Pair.second = p_o;
	map_pos_options.insert(Pair);
	Pair.first = lineNum+3;
	map_pos_options.insert(Pair);
			
}

int CPosMaxEn::Filter_Unleagal(int lineNum,string & senIn)
{
	
	vector<string>  vectWord;
	
	string Sep = " ";
	String_SeperateAppendToList(senIn,vectWord,Sep);

	for(int i = 0;i<lineNum-1;i++)
	{
		for(int j = 0;j<options_array[i+1].size();j++)
		{
			string pos_cur = options_array[i+1][j];
			bool flag = false;
			for(int k = 0;k< options_array[i].size();k++)
			{
				string pos_pre = options_array[i][k];
				if(g_mwu == "mwu")
				{	
					if(MWUTool::legal_transition(pos_pre,pos_cur) != 0)
					{
						flag = true;
						break;
					}
				}
				else if(g_mwu == "tag")
				{
					if(MWUTool::legal_transition_no(pos_pre,pos_cur) != 0)
					{
						flag = true;
						break;
					}
				}
				else if(g_mwu == "sim")
				{
					if(MWUTool::legal_transition_simp_no(pos_pre,pos_cur) != 0)
					{
						flag = true;
						break;
					}
				}
				else
				{
					exit(2);
					cout<< "please check the g_mwu value"<<endl;
				}
			}
			if(flag == false)
			{
				options_array[i+1].erase(options_array[i+1].begin()+j);
				j--;
			}
		}
	}
	
	for(int i = lineNum-2;i>=0;i--)
	{
		for(int j = 0;j<options_array[i].size();j++)
		{
			string pos_pre = options_array[i][j];
			bool flag = false;
			for(int k = 0;k< options_array[i+1].size();k++)
			{
				string pos_cur = options_array[i+1][k];
				if(g_mwu == "mwu")
				{
					if(MWUTool::legal_transition(pos_pre,pos_cur) != 0)
					{
						flag = true;
						break;
					}
				}
				else if(g_mwu == "tag")
				{
					if(MWUTool::legal_transition_no(pos_pre,pos_cur) != 0)
					{
						flag = true;
						break;
					}
				}
				else if(g_mwu == "sim")
				{
					if(MWUTool::legal_transition_simp_no(pos_pre,pos_cur) != 0)
					{
						flag = true;
						break;
					}
				}
				else
				{
					exit(2);
					cout<< "please check the g_mwu value"<<endl;
				}
			}
			if(flag == false)
			{
				options_array[i].erase(options_array[i].begin()+j);
				j--;
			}
		}
	}
	
	// in order to output to compare with original new_test_options.
	/*for(int i = 0;i<lineNum;i++)
	{
			cerr<<vectWord[i]<<" ";
			for(int j = 0;j<options_array[i].size();j++)
			{
					cerr<<options_array[i][j];
					cerr<<" ";
			}	
			cerr<<endl;
	}*/
}

int CPosMaxEn::FillInvvTAGNODE(vvTAGNODE& tags,vector<string>& vect_sent)
{
	//
	tags.resize(vect_sent.size()+2);
	for(int i = 0;i<vect_sent.size();i++)
	{
		vector<int> & v_pos = map_pos_options[i+2];
		for(int j= 0;j<v_pos.size();j++)
		{
			TAGNODE tagNode(IDToTag(v_pos[j]),vect_sent,i,1,v_pos[j]);
			tags[i+1].push_back(tagNode);
		}
	}
	return 0;
}
double CPosMaxEn::FixCountFilter(int right, int sum, int all_num,double threshold,double& acc_result, double& threshold_result)
{
	if(g_all_result.empty())
	{
		return 0.0;
	}
	//cout<<"sum"<<sum<<endl;
	//cout<<"right"<<right<<endl;
	int all = all_num - sum ;
	
	for(int i = 0;i<all;i++)
	{
		if(g_all_result[i].tag == 1)
			right ++;
	}
	double acc = double(right*100)/sum;
	acc_result = acc;
	threshold_result = g_all_result[all].score;
	cerr<<" ";
	//cerr.precision(5);
	cerr<<g_all_result[all].score<<"\t";
	cerr<<threshold<<"\t"<<acc<<"\t";

	if(acc>threshold) cerr<<"(+)"<<endl;
	else cerr<<"(-)"<<endl;
	return acc; 

}


int CPosMaxEn::FB_FilterFile(string strFileName, int AFileIndex)
{

	string name_file_0 = "result_0";
	name_file_0 = File_FileName_AddIndex(name_file_0, int(g_sigma));
	file_0.open (name_file_0.c_str(), fstream::out|fstream::app  );

	string name_file_1 = "result_1";
	name_file_1 = File_FileName_AddIndex(name_file_1, int(g_sigma));
	file_1.open (name_file_1.c_str(), fstream::out|fstream::app );

	string name_file_2 = "result_2";
	name_file_2 = File_FileName_AddIndex(name_file_2, int(g_sigma));
	file_2.open (name_file_2.c_str(), fstream::out|fstream::app );

	string name_file_3 = "result_3";
	name_file_3 = File_FileName_AddIndex(name_file_3, int(g_sigma));	
	file_3.open (name_file_3.c_str(), fstream::out|fstream::app );

	string name_file_4 = "result_4";
	name_file_4 = File_FileName_AddIndex(name_file_4, int(g_sigma));
	file_4.open (name_file_4.c_str(), fstream::out|fstream::app );



	g_all_result.clear();	
	g_right = 0;
	g_sum = 0;
	cout << " Begining Test......" << endl;
	if(g_syn_flag == 0)
	{
		//if training and testing are performed at the same time. 
		//these data has been produced by training process, don't need to load them again.
		m_numTag = LoadPosTag(g_TagIDMapFileName);
		if(!g_flag_pos_options)
			LoadDict();
		LoadTemplate(g_templateFileName);
		
		LoadModel(0);
		GetPredictsFromFeatures();
	}
	
	string mPosFileName = File_FileName_AddSuffix(strFileName, "_me_fb"); 
	string mstrNewFileNameWithIndex = File_FileName_AddIndex(mPosFileName, AFileIndex); 
	fstream fPosFile,fPosResultFile,fPosOptionsFile;
	fPosFile.open(strFileName.c_str(),ios::in);
	if(g_filter_value >-0.5 || g_decodingMethod ==0)
	{
		fPosResultFile.open(mstrNewFileNameWithIndex.c_str(),ios::out);
		VERIFY_FILE(fPosResultFile);
	}
	if(g_flag_pos_options)
	{
		string fileNameOption = g_testingFileName+"_options";
		fPosOptionsFile.open(fileNameOption.c_str(),ios::in);
		VERIFY_FILE(fPosOptionsFile);
	}

	VERIFY_FILE(fPosFile);
	

	string strPosLine;
	int line = 0;
	char ch_num[20];

	string sSentence = "";
	string pos_options_line;
	int lineNum = 0;
	vector<string> vect_sent;
	double threshold_1 = 0.0;
	double threshold_2 = 1.0;
	double threshold_3 = 2.0;
	double threshold_4 = 5.0;
	do
	{
		char chline[10000];
		fPosFile.getline(chline,10000);
		string sline;
		sline.assign(chline);

		if(g_flag_pos_options)
		{	
			fPosOptionsFile.getline(chline,10000);
			pos_options_line.assign(chline);
		}

		if(sline=="")
			continue;

		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");
		string word = vectline[0];
				
		if(word.substr(0,3) == "W@#")
		{
		 
			if(word =="W@#WR-P-P-H-0000000005.p.2.s.2W@#")
			{
				int kkk = 0;
			}
			
			if(sSentence != "")
			{
				try
				{
					g_hashProb.clear();

					Filter_Unleagal(lineNum,sSentence);
					FillInTagMap(lineNum);
					strPosLine = FB_FilterSentence(sSentence);
				
					vvTAGNODE tags;
					FillInvvTAGNODE(tags,vect_sent);
					InitGrid();
					
					if(g_combine == 1 )
					{
						g_hmm = 1;
						TRELLIS trellis(vect_sent,tags,SENTENCE_START,SENTENCE_END);
						trellis.computeForwardProbabilities();
						trellis.computeBackwardProbabilities();
						trellis.computeForwardBackwardValues();
						if(g_context == 1)
						{
							trellis.reduceTags();
						}
						else
						{
							trellis.reduceTags_NO_Context();
						}
						//trellis.rankTags();
						if(g_debug)
						{	
							trellis.print();
						}
						//trellis.StoreTags(0,map_pos_gold,g_0_right,g_0_sum);
					}
					{
						g_hmm = 0;
						TRELLIS trellis(vect_sent,tags,SENTENCE_START,SENTENCE_END);
						trellis.computeForwardProbabilities();
						trellis.computeBackwardProbabilities();
						trellis.computeForwardBackwardValues();
						if(g_context == 1)
						{
							trellis.reduceTags();
						}
						else
						{
							trellis.reduceTags_NO_Context();
						}
						if(g_combine ==1)
						{
							trellis.Combine();
						}
						trellis.rankTags();
						
						if(g_debug)
						{	
							trellis.print();
						}
						if(g_syn_flag == 1)
						{
							trellis.StoreTags(0,map_pos_gold,g_0_right,g_0_sum);
						}
						if(g_syn_flag == 0 )
						{
							trellis.StoreTags(0,map_pos_gold,g_0_right,g_0_sum);
							if(g_filter_value>=0.0 )
							{
								trellis.keepGoodTags(g_filter_value,map_pos_gold,g_1_right,g_1_sum);
								trellis.printTagging(fPosResultFile);
							}
						}
					}
					/*trellis.keepGoodTags(threshold_1,map_pos_gold,g_0_right,g_0_sum);
					
					trellis.keepGoodTags(threshold_2,map_pos_gold,g_1_right,g_1_sum);
					trellis.keepGoodTags(threshold_3,map_pos_gold,g_2_right,g_2_sum);
					trellis.keepGoodTags(threshold_4,map_pos_gold,g_3_right,g_3_sum);*/
					
					
					//trellis.printTagging(fPosResultFile);
				}
				catch(...)
				{
					if(g_filter_value>-0.5 )
					{
						fPosResultFile.close();
					}
					throw;
				}
				sSentence = "";
				vect_sent.clear();
			}
			//cerr<<word<<endl;
			if(g_filter_value>-0.5)
			{
				fPosResultFile<<word<<endl;
			}
			map_pos_options.clear();
			map_pos_gold.clear();
			for(int zzzz = 0;zzzz<lineNum;zzzz++)
			{
				options_array[zzzz].clear();
			}
			lineNum = 0;
		}
		else
		{
			string pos_tag = vectline[vectline.size()-1];
			pair<int, int > Pair;
			Pair.first = lineNum;
			Pair.second = ForceTagToID(pos_tag);	
			map_pos_gold.insert(Pair);	
	
			sSentence += word;
			sSentence +=" ";
			vect_sent.push_back(word);
			if(g_flag_pos_options)
			{
				String_SeperateToList_WithTrim(pos_options_line,vectline," ");	
				string word_opt = vectline[0];
				if(word_opt != word)
				{
					cout<<"error in pos_options"<<endl;
					exit(2);
				}
				for(int i = 1;i<vectline.size();i++)
				{	
					options_array[lineNum].push_back(vectline[i]);
				}
			}
			lineNum++;
			
		}
		line++;
		
		
		if(line%10000 == 0)
		{
			cout<< "has dealt with ";
			cout<< line;
			cout<< " lines";
			cout<<endl;
		}
	
	}while(!fPosFile.eof());

	if(sSentence != "")
	{
		g_hashProb.clear();

		Filter_Unleagal(lineNum,sSentence);
		FillInTagMap(lineNum);
		strPosLine = FB_FilterSentence(sSentence);
				
		vvTAGNODE tags;
		FillInvvTAGNODE(tags,vect_sent);

		TRELLIS trellis(vect_sent,tags,SENTENCE_START,SENTENCE_END);
		trellis.computeForwardProbabilities();
		trellis.computeBackwardProbabilities();
		trellis.computeForwardBackwardValues();
		if(g_context == 1)
		{
			trellis.reduceTags();
		}
		else
		{
			trellis.reduceTags_NO_Context();
		}
						
		trellis.rankTags();
		if(g_debug)
		{	
			trellis.print();
		}
		//trellis.keepGoodTags(threshold_1,map_pos_gold,g_0_right,g_0_sum);
		//trellis.keepGoodTags(threshold_2,map_pos_gold,g_1_right,g_1_sum);
		//trellis.keepGoodTags(threshold_3,map_pos_gold,g_2_right,g_2_sum);
		//trellis.keepGoodTags(threshold_4,map_pos_gold,g_3_right,g_3_sum);
					
		if(g_syn_flag == 1)
		{
			trellis.StoreTags(0,map_pos_gold,g_0_right,g_0_sum);
		}
		if(g_syn_flag == 0 )
		{
			trellis.StoreTags(0,map_pos_gold,g_0_right,g_0_sum);
			trellis.keepGoodTags(g_filter_value,map_pos_gold,g_1_right,g_1_sum);
			trellis.printTagging(fPosResultFile);
		}

		map_pos_options.clear();
		map_pos_gold.clear();
		for(int zzzz = 0;zzzz<lineNum;zzzz++)
		{
			options_array[zzzz].clear();
		}
		lineNum = 0;
	}

	fPosFile.close();
	if(g_filter_value>-0.5)
	{
		fPosResultFile.close();
	}
	if(g_flag_pos_options)
	{
		fPosOptionsFile.close();	
	}

	cerr<<"g_templateFileName="<<g_templateFileName;
	cerr<<" c="<<g_threshold;
	cerr<<" g="<<g_sigma<<endl;

	sort(g_all_result.begin(),g_all_result.end());
	
	/*FixCountFilter(g_0_right,g_0_sum,32110,93.53,result_fb_0, threshold_fb_0);
	FixCountFilter( g_0_right,g_0_sum,32599,94.3538,result_fb_1, threshold_fb_1);
	FixCountFilter( g_0_right,g_0_sum,33160,94.8926,result_fb_2, threshold_fb_2);
	FixCountFilter( g_0_right,g_0_sum,34301,95.7241,result_fb_3, threshold_fb_3);
	FixCountFilter( g_0_right,g_0_sum,35542,96.2722,result_fb_4, threshold_fb_4);*/

	/*FixCountFilter(g_0_right,g_0_sum,32110,93.7456,result_fb_0, threshold_fb_0);
	FixCountFilter( g_0_right,g_0_sum,32633,94.4815,result_fb_1, threshold_fb_1);
	FixCountFilter( g_0_right,g_0_sum,33198,95.0202,result_fb_2, threshold_fb_2);
	FixCountFilter( g_0_right,g_0_sum,34351,95.9296,result_fb_3, threshold_fb_3);
	FixCountFilter( g_0_right,g_0_sum,35655,96.5026,result_fb_4, threshold_fb_4);*/

	//training by tiny 
	
	/*
	FixCountFilter(g_0_right,g_0_sum,32110,93.5223,result_fb_0, threshold_fb_0);
  	FixCountFilter( g_0_right,g_0_sum,33217,94.7586,result_fb_1, threshold_fb_1);
  	FixCountFilter( g_0_right,g_0_sum,34502,95.6587,result_fb_2, threshold_fb_2);
  	FixCountFilter( g_0_right,g_0_sum,35833,96.2628,result_fb_3, threshold_fb_3);*/
	//FixCountFilter( g_0_right,g_0_sum,44386,97.8698,result_fb_4, threshold_fb_4);

	/*//training by mini
	FixCountFilter(g_0_right,g_0_sum,32110,94.8116,result_fb_0, threshold_fb_0);
  	FixCountFilter( g_0_right,g_0_sum,33958,96.4995,result_fb_1, threshold_fb_1);
  	FixCountFilter( g_0_right,g_0_sum,34491,96.7393,result_fb_2, threshold_fb_2);
  	FixCountFilter( g_0_right,g_0_sum,35108,96.9916,result_fb_3, threshold_fb_3);*/

	//training by large
	FixCountFilter(g_0_right,g_0_sum,32110,96.1289,result_fb_0, threshold_fb_0);
  	FixCountFilter( g_0_right,g_0_sum,33588,97.4899,result_fb_1, threshold_fb_1);
  	FixCountFilter( g_0_right,g_0_sum,34628,97.9975,result_fb_2, threshold_fb_2);
  	FixCountFilter( g_0_right,g_0_sum,35902,98.4522,result_fb_3, threshold_fb_3);


	//Chinese
	/*FixCountFilter(g_0_right,g_0_sum,246209,95.8076,result_fb_0, threshold_fb_0);
	FixCountFilter( g_0_right,g_0_sum,257994,97.5626,result_fb_1, threshold_fb_1);
	FixCountFilter( g_0_right,g_0_sum,264326,98.169,result_fb_2, threshold_fb_2);
	FixCountFilter( g_0_right,g_0_sum,282862,99.0516,result_fb_3, threshold_fb_3);
	FixCountFilter( g_0_right,g_0_sum,325130,99.4395,result_fb_4, threshold_fb_4);
	*/

	
	/*//English
	FixCountFilter(g_0_right,g_0_sum,256488,96.9523,result_fb_0, threshold_fb_0);
	FixCountFilter( g_0_right,g_0_sum,265068,98.1036,result_fb_1, threshold_fb_1);
	FixCountFilter( g_0_right,g_0_sum,275923,98.7321,result_fb_2, threshold_fb_2);
	FixCountFilter( g_0_right,g_0_sum,294122,99.1329,result_fb_3, threshold_fb_3);
	FixCountFilter( g_0_right,g_0_sum,315401,99.3547,result_fb_4, threshold_fb_4);
	*/


	file_0<<result_fb_0<<endl;
	file_1<<result_fb_1<<endl;
	file_2<<result_fb_2<<endl;
	file_3<<result_fb_3<<endl;
	file_4<<result_fb_4<<endl;
	



	g_0_right = 0; g_0_sum = 0;
	g_1_right = 0; g_1_sum = 0;
	g_2_right = 0; g_2_sum = 0;
	g_3_right = 0; g_3_sum = 0;

	file_0.close();
	file_1.close();
	file_2.close();
	file_3.close();
	file_4.close();

	return line;
}

int CPosMaxEn::OutputForMalouf()
{
	vector< vector< string> > malouf;
	malouf.resize(m_numTag);
	
	EVENT mCurEvent;
	m_EventList.OpenEventList(OpenMode_Read);
	fstream f1;
	f1.open("malouf",ios::out);

	while (m_EventList.ReadEvent(mCurEvent) == true)
	{	//f1<<m_numTag<<endl;
		string a = OutputMalouf(mCurEvent);
		f1<<a;
		malouf[mCurEvent.outTag].push_back(a);
	}
	m_EventList.CloseEventList();

/*	fstream f1;
	f1.open("malouf",ios::out);

	for(int i = 0;i<m_numTag;i++)
	{
		f1<< malouf[i].size()<<endl;;
		for(int j = 0;j<malouf[i].size();j++)
		{
			f1<< malouf[i][j]<<endl;
		}
	}*/
	f1.close();
}
string CPosMaxEn::OutputMalouf(EVENT & event)
{
	string result,temp ;
	result = "";
	temp = "";
	int sum = 0;
	int sumPos = 0;
	for(int k = 0;k<m_numTag;k++)
	{
		sum = 0;
		temp = "";
		if(k != event.outTag)
		{
			temp+="0 ";
		}
		else
		{
			temp+="1 ";
		}
			string linetemp = "";
			for(int i = 0;i<event.vectIndexPredicate.size();i++)
			{			
				PREDICATE & mCurPredicate = m_vectPredicates[event.vectIndexPredicate[i]];  
	
				vector<pair<int,int> >::iterator s1_Iter = mCurPredicate.indexFeature.begin();
				
				while(s1_Iter != mCurPredicate.indexFeature.end())
				{	
					//pair<int,int> pValue = *s1_Iter;
					if(s1_Iter->first == k)
					{
						char buffer[20];
						sprintf(buffer, "%d", s1_Iter->second);
						string key(buffer);
						linetemp += key;
						linetemp += " 1 ";
						sum ++;
					}
					s1_Iter++;
				}
				//
			}
			if(sum >0)
			{
				char buffer[20];
				sprintf(buffer, "%d", sum);
				string key(buffer);
				temp +=key;
				temp +=" ";
				temp += linetemp;
				result +=temp;
				result +="\n";
				sumPos++;
			}
	}
	char buffer[20];
	sprintf(buffer, "%d", sumPos);
	string key(buffer);
				
	string last_result = key + "\n";
	last_result += result; 
	cout<<last_result<<endl;
	return last_result;
}

int CPosMaxEn::TagFile(string strFileName, int AFileIndex)
{
	g_right = 0; g_sum = 0;
	g_0_right = 0; g_0_sum = 0;
	g_1_right = 0; g_1_sum = 0;
	g_2_right = 0; g_2_sum = 0;
	g_3_right = 0; g_3_sum = 0;
	g_all_result.clear();

	cout << " Begining Test......" << endl;
	if(g_syn_flag == 0)
	{
		//if training and testing are performed at the same time. 
		//these data has been produced by training process, don't need to load them again.
		m_numTag = LoadPosTag(g_TagIDMapFileName);
		if(!g_flag_pos_options)
			LoadDict();
		LoadTemplate(g_templateFileName);
		LoadModel(0);
		GetPredictsFromFeatures();
	}
	
	//strFileName = "data/"+APPLICATION+"/"+strFileName;
	string mPosFileName = File_FileName_AddSuffix(strFileName, "_me_nb"); 
	string mstrNewFileNameWithIndex = File_FileName_AddIndex(mPosFileName, AFileIndex); 
	fstream fPosFile,fPosResultFile,fPosOptionsFile;
	fPosFile.open(strFileName.c_str(),ios::in);
	if(g_filter_value>-0.5 ||g_decodingMethod ==0)
	{
		fPosResultFile.open(mstrNewFileNameWithIndex.c_str(),ios::out);
		VERIFY_FILE(fPosResultFile);
	}
	if(g_flag_pos_options)
	{
		string fileNameOption = g_testingFileName+"_options";
		fPosOptionsFile.open(fileNameOption.c_str(),ios::in);
		VERIFY_FILE(fPosOptionsFile);
	}

	VERIFY_FILE(fPosFile);
	

	string strPosLine;
	int line = 0;
	char ch_num[20];

	string sSentence = "";
	string pos_options_line;
	int lineNum = 0;
	do
	{
		char chline[10000];
		fPosFile.getline(chline,10000);
		string sline;
		sline.assign(chline);

		if(g_flag_pos_options)
		{	
			fPosOptionsFile.getline(chline,10000);
			pos_options_line.assign(chline);
			//cerr<<pos_options_line<<endl;
		}

		if(sline=="")
		{
			continue;
		}
		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");
		string word = vectline[0];
		
		
		if(word.substr(0,3) == "W@#")
		{
		 
			//if(word =="W@#WR-P-P-H-0000000002.p.1.s.2W@#")
			//{
			//	int kkk = 0;
			//}
			
			if(sSentence != "")
			{
				try
				{
					Filter_Unleagal(lineNum,sSentence);
					FillInTagMap(lineNum);
					strPosLine = TagSentence(sSentence);
					if(g_filter_value>-0.5|| g_decodingMethod == 0)
					{
						fPosResultFile<<strPosLine;
					}
				}
				catch(...)
				{
					if(g_filter_value>-0.5 || g_decodingMethod == 0)
					{
						fPosResultFile.close();
					}
					throw;
				}
				sSentence = "";
			}
			//cerr<<word<<endl;
			if(g_filter_value>-0.5 || g_decodingMethod == 0)
			{
				fPosResultFile<<word<<endl;
			}
			map_pos_options.clear();
			map_pos_gold.clear();
			for(int zzzz = 0;zzzz<lineNum;zzzz++)
			{
				options_array[zzzz].clear();
			}
			lineNum = 0;
		}
		else
		{
			string pos_tag = vectline[vectline.size()-1];
			pair<int, int > Pair;
			Pair.first = lineNum;
			Pair.second = ForceTagToID(pos_tag);	
			map_pos_gold.insert(Pair);	
	
			sSentence += word;
			sSentence +=" ";
			if(g_flag_pos_options)
			{
				String_SeperateToList_WithTrim(pos_options_line,vectline," ");	
				string word_opt = vectline[0];
				if(word_opt != word)
				{
					cout<<"error in pos_options"<<endl;
					exit(2);
				}
				for(int i = 1;i<vectline.size();i++)
				{	
					options_array[lineNum].push_back(vectline[i]);
				}
			}
			//g_sum ++;
			lineNum++;
		}
		line++;
		
		
		if(line%10000 == 0)
		{
			cout<< "has dealt with ";
			cout<< line;
			cout<< " lines";
			cout<<endl;
		}
	
	}while(!fPosFile.eof());

	if(sSentence != "")
	{
		Filter_Unleagal(lineNum,sSentence);
		FillInTagMap(lineNum);
		strPosLine = TagSentence(sSentence);
		if(g_filter_value>-0.5 || g_decodingMethod == 0)
		{
			fPosResultFile<<strPosLine;
		}
		map_pos_options.clear();
		map_pos_gold.clear();
		for(int zzzz = 0;zzzz<lineNum;zzzz++)
		{
			options_array[zzzz].clear();
		}
		lineNum = 0;
	}
	
	fPosFile.close();
	if(g_filter_value>-0.5 || g_decodingMethod == 0)
	{
		fPosResultFile.close();
	}
	if(g_flag_pos_options)
	{
		fPosOptionsFile.close();	
	}
	cerr<<"g_templateFileName="<<g_templateFileName;
	cerr<<" c="<<g_threshold;
	cerr<<" g="<<g_sigma;

	if(g_decodingMethod == 0)
	{	
		cerr<<" "<<double(g_right*100)/g_sum<<endl;
	}
	if(g_decodingMethod == 1)
	{
		sort(g_all_result.begin(),g_all_result.end());
		cerr<<endl;
		if(APPLICATION == "dutch")
		{
			//dutch
			FixCountFilter(g_0_right,g_0_sum,32110,93.5223,result_fb_0, threshold_fb_0);
			FixCountFilter( g_0_right,g_0_sum,33217,94.7586,result_fb_1, threshold_fb_1);
			FixCountFilter( g_0_right,g_0_sum,34502,95.6587,result_fb_2, threshold_fb_2);
			FixCountFilter( g_0_right,g_0_sum,35833,96.2628,result_fb_3, threshold_fb_3);
			FixCountFilter( g_0_right,g_0_sum,44386,97.8698,result_fb_4, threshold_fb_4);
		}
		else if(APPLICATION == "chinese")
		{
			//Chinese
			FixCountFilter(g_0_right,g_0_sum,246209,95.8076,result_fb_0, threshold_fb_0);
			FixCountFilter( g_0_right,g_0_sum,257994,97.5626,result_fb_1, threshold_fb_1);
			FixCountFilter( g_0_right,g_0_sum,264326,98.169,result_fb_2, threshold_fb_2);
			FixCountFilter( g_0_right,g_0_sum,282862,99.0516,result_fb_3, threshold_fb_3);
			FixCountFilter( g_0_right,g_0_sum,325130,99.4395,result_fb_4, threshold_fb_4);
			
		}
		else if(APPLICATION == "english")
		{
			//English
			FixCountFilter(g_0_right,g_0_sum,256488,96.9523,result_fb_0, threshold_fb_0);
			FixCountFilter( g_0_right,g_0_sum,265068,98.1036,result_fb_1, threshold_fb_1);
			FixCountFilter( g_0_right,g_0_sum,275923,98.7321,result_fb_2, threshold_fb_2);
			FixCountFilter( g_0_right,g_0_sum,294122,99.1329,result_fb_3, threshold_fb_3);
			FixCountFilter( g_0_right,g_0_sum,315401,99.3547,result_fb_4, threshold_fb_4);
		}
	}
	
	return line;
}


string CPosMaxEn::FB_FilterSentence(string& senIn) 
{
	///////////////////////////////////////////////////////////////////
	min_sumTag++;
	sumTag++;
	if (senIn == "") return "";   
	JHeapClear(m_Heap);
	JHeapClear(m_NewHeap);
	
	vector<string>  vectWord;
	//vectWord.push_back("#B");
	//vectWord.push_back("#B");
	vectWord.push_back(START_WORD);
	vectWord.push_back(START_WORD);

	string Sep = " ";
	senIn = Trim(senIn);
	String_SeperateAppendToList(senIn,vectWord,Sep);
	//vectWord.push_back("#E");
	//vectWord.push_back("#E");
	vectWord.push_back(END_WORD);
	vectWord.push_back(END_WORD);
	
	///////////////////////////////////////////////////////////////////

	
	SEQUENCE seq;
	seq.score = 0;    
	seq.tagSequence.clear();
	seq.tagSequence.push_back(TagToID(START_TAG));
	seq.tagSequence.push_back(TagToID(START_TAG));

	//jwHeap;
	m_Heap->push(seq);


	//////////////////////////////////////////////////////////////////
	int mWordSize = vectWord.size();
	string strSen;
	for(int i = 2;i<mWordSize - 1;i++)
	{
		//strSen += FB_FilterWindow(vectWord,i);  //index代表窗口中间元素
		strSen += FB_FilterWindow_Context(vectWord,i);
	}

	
	return "";
}


string CPosMaxEn::TagSentence(string& senIn) 
{
	///////////////////////////////////////////////////////////////////
	min_sumTag++;
	sumTag++;
	if (senIn == "") return "";   
	JHeapClear(m_Heap);
	JHeapClear(m_NewHeap);
	
	vector<string>  vectWord;
	//vectWord.push_back("#B");
	//vectWord.push_back("#B");
	vectWord.push_back(START_WORD);
	vectWord.push_back(START_WORD);

	string Sep = " ";
	senIn = Trim(senIn); //because there is space in the end of senIn
	String_SeperateAppendToList(senIn,vectWord,Sep);
	
	//vectWord.push_back("#E");
	//vectWord.push_back("#E");
	vectWord.push_back(END_WORD);
	vectWord.push_back(END_WORD);
	
	///////////////////////////////////////////////////////////////////

	
	SEQUENCE seq;
	seq.score = 0;    
	seq.tagSequence.clear();
	seq.tagSequence.push_back(TagToID(START_TAG));
	seq.tagSequence.push_back(TagToID(START_TAG));

	//jwHeap;
	m_Heap->push(seq);


	//////////////////////////////////////////////////////////////////
	int mWordSize = vectWord.size();
	string strSen;
	for(int i = 2;i<mWordSize - 2;i++)
	{
		strSen += TagWindow(vectWord,i);  //index代表窗口中间元素
	}

	

	if(g_decodingMethod == 1)
	{
		/*strSen = N_Best_Result(vectWord,g_1_right,g_1_sum,1);
		strSen = N_Best_Result(vectWord,g_2_right,g_2_sum,2);
		strSen = N_Best_Result(vectWord,g_3_right,g_3_sum,5);*/
		
		strSen = StoreAllOptions(vectWord,g_0_right,g_0_sum);
	}

	if(g_decodingMethod == 0)
	{
		strSen = Single_Best_Result(vectWord);
	}

	return strSen;
}


string CPosMaxEn::FB_FilterWindow(vector<string>& vectIn ,int index)
{

	string strWord = vectIn[index];
	vector<int> vectPos;
	vector<int> vectPos_L2;
	vector<int> vectPos_L1;

	if(g_flag_pos_options)
	{
		vectPos_L2 = map_pos_options[index-2];
		vectPos_L1 = map_pos_options[index-1];
		vectPos = map_pos_options[index];
	}
	else
	{
		if(test_dict.find(strWord) != test_dict.end())
		{
			vectPos = test_dict[strWord];
		}
		else
		{
			vectPos.push_back(TagToID(UNKNOWN_WORD));
		}
	}
	

	vector<double> vectProbability;

	
	for(int i = 0; i<vectPos_L2.size();i++)
	{
		for(int j = 0; j<vectPos_L1.size();j++)
		{
			int pos_L2 = vectPos_L2[i];
			int pos_L1 = vectPos_L1[j];
			//cout<<IDToTag(vectPos_L2[i])<<endl;
			//cout<<IDToTag(vectPos_L1[j])<<endl;

			//if(vectPos.size() ==1 && IDToTag(vectPos[0])==END_TAG)
			if(vectIn.size()-2 == index)
			{
				char buffer[20];
				string key, temp;
				sprintf(buffer, "%d",index);
				key = buffer;
				key +="+";
				key +=IDToTag(vectPos_L2[i]);
				key +="+";
				key +=IDToTag(vectPos_L1[j]);
				key +="+";
				key +=END_TAG;
				pair<string,double> item(key,1.0f);
				g_hashProb.insert(item);
				if(g_debug)
				{
					cout<<"   "<<key<<" " <<1.0<<endl;
				}
				continue;
			}

			if(true)
			{
				SEQUENCE mSeqTemp;
				mSeqTemp.tagSequence.push_back(vectPos_L2[i]);
				mSeqTemp.tagSequence.push_back(vectPos_L1[j]);
				
				vectProbability.clear();
				GetProbability_Based_All(vectIn,index, mSeqTemp, vectPos,vectProbability);

				for(int k = 0; k<vectPos.size();k++)
				{
					char buffer[20];
					string key, temp;
					sprintf(buffer, "%d",index);
					key = buffer;
					key +="+";
					key +=IDToTag(vectPos_L2[i]);
					key +="+";
					key +=IDToTag(vectPos_L1[j]);
					key +="+";
					key +=IDToTag(vectPos[k]);
					double prob = vectProbability[k];
					
					//if(prob<0.0000001f)
					//{
					//	cerr<<"  0 Change prob"<<key<<" "<<"from"<< prob <<"To 0.0000001f"<<endl; 
					//	prob = 0.0000001f;
					//}
					if(prob>1.0f)
					{
						cerr<<"  1 Change prob"<<key<<" "<<"from"<< prob <<"To 1.0f"<<endl;
						prob = 1.0f;
					}
					//double prob = vectValue[k];
					pair<string,double> item(key,prob);
					//deal with zero probability!
	
					g_hashProb.insert(item);
					if(g_debug)
					{
						cout<<"   "<<key<<" " <<prob<<endl;
					}
				}
				if(g_debug)
				{
					cout<<"   "<<"============================================"<<endl;
				}
			}
			else
			{
// 				for(int k = 0; k<vectPos.size();k++)
// 				{
// 					char buffer[20];
// 					string key, temp;
// 					sprintf(buffer, "%d",index);
// 					key = buffer;
// 					key +="+";
// 					key +=IDToTag(vectPos_L2[i]);
// 					key +="+";
// 					key +=IDToTag(vectPos_L1[j]);
// 					key +="+";
// 					key +=IDToTag(vectPos[k]);
// 					double prob = vectProbability[k];
// 					if(MWUTool::legal_transition(IDToTag(vectPos_L1[j]),IDToTag(vectPos[k]))==2)
// 					{
// 						prob = 1.0f;
// 						pair<string,double> item(key,prob);
// 			
// 						g_hashProb.insert(item);
// 						cout<<"   "<<key<<" " <<prob<<endl;
// 						break;
// 					}
// 					
// 				}
// 				cout<<"   "<<"============================================"<<endl;
			}
			
		}
	}
	return "";
}

string CPosMaxEn::FB_FilterWindow_Context(vector<string>& vectIn ,int index)
{

	string strWord = vectIn[index];
	vector<int> vectPos;
	vector<int> vectPos_L2;
	vector<int> vectPos_L1;

	if(g_flag_pos_options)
	{
		vectPos_L2 = map_pos_options[index-2];
		vectPos_L1 = map_pos_options[index-1];
		vectPos = map_pos_options[index];
	}
	else
	{
		if(test_dict.find(strWord) != test_dict.end())
		{
			vectPos = test_dict[strWord];
		}
		else
		{
			vectPos.push_back(TagToID(UNKNOWN_WORD));
		}
	}
	

	vector<double> vectProbability;
	vector<string> vectContext;
	if(g_context == 1)
	{
		vectContext.push_back("post");
		vectContext.push_back("pre");
	}
	else
	{
		vectContext.push_back(DUMMY_CONTEXT);
		vectContext.push_back(DUMMY_CONTEXT);
	}

	
	for(int i = 0; i<vectPos_L2.size();i++)
	{
		int pos_L2 = vectPos_L2[i];
		for(int j = 0; j<vectPos_L1.size();j++)
		{
			int pos_L1 = vectPos_L1[j];
			for(int k = 0;k<vectContext.size();k++)
			{	
				string context_pre =  vectContext[k];
				//cout<<IDToTag(vectPos_L2[i])<<endl;
				//cout<<IDToTag(vectPos_L1[j])<<endl;
	
				//if(vectPos.size() ==1 && IDToTag(vectPos[0])==END_TAG)
				if(index == 2)
				{
					SEQUENCE mSeqTemp;
					mSeqTemp.tagSequence.push_back(vectPos_L2[i]);
					mSeqTemp.tagSequence.push_back(vectPos_L1[j]);
				
					vectProbability.clear();
					string zzzyyy = DUMMY_CONTEXT;
					for(int l = 0;l<vectContext.size();l++)
					{
						string context_cur =  vectContext[l];
						GetProbability_Based_All_Context(vectIn,index, mSeqTemp,vectPos,vectProbability,zzzyyy,context_cur);
						for(int k = 0; k<vectPos.size();k++)
						{
							char buffer[20];
							string key, temp;
							sprintf(buffer, "%d",index);
							key = buffer;
							key +="+";
							key +=IDToTag(vectPos_L2[i]);
							key +="+";
							key +=IDToTag(vectPos_L1[j]);
							key +="+";
							key +=IDToTag(vectPos[k]);
							key +="+";
							key +=DUMMY_CONTEXT;
							key +="+";
							key +=context_cur;
							
							double prob = vectProbability[k];
							
							pair<string,double> item(key,prob);
							if(g_hashProb.find(key) == g_hashProb.end())
							{
								g_hashProb.insert(item);
							}
							
							if(g_debug)
							{
								cout<<"   "<<key<<" " <<prob<<endl;
							}
						}
					}
					if(g_debug)
					{
						cout<<"   "<<"============================================"<<endl;
					}
					break;
				}				

				if(vectIn.size()-2 == index)
				{
					//for(int l = 0;l<vectContext.size();l++)
						{
							//string context_cur =  vectContext[l];
							char buffer[20];
							string key, temp;
							sprintf(buffer, "%d",index);
							key = buffer;
							key +="+";
							key +=IDToTag(vectPos_L2[i]);
							key +="+";
							key +=IDToTag(vectPos_L1[j]);
							key +="+";
							key +=END_TAG;
							key +="+";
							key +=context_pre;
							key +="+";
							key +=DUMMY_CONTEXT;
							
							pair<string,double> item(key,1.0f);
							g_hashProb.insert(item);
							if(g_debug)
							{
								cout<<"   "<<key<<" " <<1.0<<endl;
							}
						}
							continue;
				}
				
				for(int l = 0;l<vectContext.size();l++)
				{	
					string context_cur =  vectContext[l];
					if(true)
					//if(MWUTool::ht_mwu(IDToTag(pos_L1)) < 1)
					{
						SEQUENCE mSeqTemp;
						mSeqTemp.tagSequence.push_back(vectPos_L2[i]);
						mSeqTemp.tagSequence.push_back(vectPos_L1[j]);
						
							vectProbability.clear();
							GetProbability_Based_All_Context(vectIn,index, mSeqTemp, vectPos,vectProbability,context_pre,context_cur);
							
			
							for(int k = 0; k<vectPos.size();k++)
							{
								char buffer[20];
								string key, temp;
								sprintf(buffer, "%d",index);
								key = buffer;
								key +="+";
								key +=IDToTag(vectPos_L2[i]);
								key +="+";
								key +=IDToTag(vectPos_L1[j]);
								key +="+";
								key +=IDToTag(vectPos[k]);
								key +="+";
								key +=context_pre;
								key +="+";
								key +=context_cur;							
								double prob = vectProbability[k];
								
								//if(prob<0.0000001f)
								//{
								//	cerr<<"  0 Change prob"<<key<<" "<<"from"<< prob <<"To 0.0000001f"<<endl; 
								//	prob = 0.0000001f;
								//}
								if(prob>1.0f)
								{
									cerr<<"  1 Change prob"<<key<<" "<<"from"<< prob <<"To 1.0f"<<endl;
									prob = 1.0f;
								}
								//double prob = vectValue[k];
								pair<string,double> item(key,prob);
								//deal with zero probability!
				
								g_hashProb.insert(item);
								if(g_debug)
								{
									cout<<"   "<<key<<" " <<prob<<endl;
								}
							}
					}
					else
					{
						for(int k = 0; k<vectPos.size();k++)
						{
							char buffer[20];
							string key, temp;
							sprintf(buffer, "%d",index);
							key = buffer;
							key +="+";
							key +=IDToTag(vectPos_L2[i]);
							key +="+";
							key +=IDToTag(vectPos_L1[j]);
							key +="+";
							key +=IDToTag(vectPos[k]);
							key +="+";
							key +=context_pre;
							key +="+";
							key +=context_cur;
							//double prob = vectProbability[k];
						if(MWUTool::legal_transition(IDToTag(pos_L1),IDToTag(vectPos[k]))==2)
							{
								double prob = 1.0f;
								pair<string,double> item(key,prob);
					
								g_hashProb.insert(item);
								if(g_debug)
								{
									cout<<"   "<<key<<" " <<prob<<endl;
								}
								break;
							}
							
						}
	// 					cout<<"   "<<"============================================"<<endl;
					}
				
				}
			}
			if(g_debug)
			{
				cout<<"   "<<"============================================"<<endl;
			}
		}
	}
	return "";
}


string CPosMaxEn::TagWindow(vector<string>& vectIn ,int index)
{

	string strWord = vectIn[index];
	vector<int> vectPos;

	if(g_flag_pos_options)
	{
		vectPos = map_pos_options[index];
	}
	else
	{
		if(test_dict.find(strWord) != test_dict.end())
		{
			vectPos = test_dict[strWord];
		}
		else
		{
			vectPos.push_back(TagToID(UNKNOWN_WORD));
		}
	}
	

	vector<double> vectProbability;
	int mMaxCount = 10;
	if (mMaxCount > m_Heap->size()) {mMaxCount = m_Heap->size();}
	

	JHeapClear(m_NewHeap); 
	for ( int k = 0; k < mMaxCount ; k ++)
	{
		SEQUENCE mCurSequence = m_Heap->top();
		m_Heap->pop();
		//
		vectProbability.clear();

		vector<int>& vectPosID = mCurSequence.tagSequence;
		int mTag1 = -1;
		if (vectPosID.size() >= 1) {mTag1 = vectPosID[vectPosID.size() - 1];}
		if(true)
		//if(MWUTool::ht_mwu(IDToTag(mTag1)) < 1)
		{
			GetProbability_Based_All(vectIn,index, mCurSequence, vectPos,vectProbability);
		}
		else
		{
// 			for(int i = 0;i<vectPos.size();i++)
// 			{
// 				if(MWUTool::legal_transition(IDToTag(mTag1),IDToTag(vectPos[i])) ==1)
// 				{
// 					SEQUENCE temp;
// 					temp.tagSequence = mCurSequence.tagSequence;
// 					temp.tagSequence.push_back(vectPos[i]);
// 					temp.score = mCurSequence.score ;
// 					m_NewHeap->push(temp);
// 				}
// 			}	
// 			continue;
		}
		

		for (int i = 0; i < vectPos.size(); i ++)
		{
			if(g_mwu == "mwu")
			{
				if(MWUTool::legal_transition(IDToTag(mTag1),IDToTag(vectPos[i])) != 0)
				{
					SEQUENCE temp;
					temp.tagSequence = mCurSequence.tagSequence;
					temp.tagSequence.push_back(vectPos[i]);
					temp.score = mCurSequence.score + log(vectProbability[i]);
					m_NewHeap->push(temp);
				}
			}
			else if(g_mwu== "tag")
			{
				if(MWUTool::legal_transition_no(IDToTag(mTag1),IDToTag(vectPos[i])) != 0)
				{
					SEQUENCE temp;
					temp.tagSequence = mCurSequence.tagSequence;
					temp.tagSequence.push_back(vectPos[i]);
					temp.score = mCurSequence.score + log(vectProbability[i]);
					m_NewHeap->push(temp);
				}
			}
			else if(g_mwu == "sim")
			{
				if(MWUTool::legal_transition_simp_no(IDToTag(mTag1),IDToTag(vectPos[i])) != 0)
				{
					SEQUENCE temp;
					temp.tagSequence = mCurSequence.tagSequence;
					temp.tagSequence.push_back(vectPos[i]);
					temp.score = mCurSequence.score + log(vectProbability[i]);
					m_NewHeap->push(temp);
				}		
			}
			else
			{
				exit(2);
				cout<<"please check g_mwu value"<<endl;
			}
			

		}

	}
	
	PriorHeap *mTempHeap;
	mTempHeap = m_Heap;
	m_Heap = m_NewHeap;
	m_NewHeap = mTempHeap;
	/////////////////////////////
	return "";
}

string CPosMaxEn::StoreAllOptions(vector<string>& vectIn,int& right,int& sum)
{
	vector<string > pos_result;
	pos_result.resize(5000);
	string result = "";
	PriorHeap mLastHeap = *m_Heap;

	int mMaxCount = mLastHeap.size();
	if(mMaxCount == 0)
	{
		cout<<"there is error in N_Best_result"<<endl;
		for(int iter = 2;iter<vectIn.size()-3;iter++)
		{
			cout<<vectIn[iter]<<endl;
		}
		exit(1);
	}
	vector< vector<int> > pos_array;
	pos_array.resize(5000);
	if (mMaxCount>=10)
		mMaxCount = 10;
	double max_score = 0.0f;
	for(int i = 0;i<mMaxCount;i++)
	{
		SEQUENCE mCurSequence = mLastHeap.top();
		mLastHeap.pop();
		if(i == 0)
		{
			max_score = mCurSequence.score;
			for(int j = 2 ;j<mCurSequence.tagSequence.size();j++)
			{
				int pos_id = mCurSequence.tagSequence[j];
				pos_result[j] = IDToTag(pos_id);
				pos_result[j] +=" ";
				pos_array[j].push_back(pos_id);
				if(pos_id == map_pos_gold[j-2])
				{
					right++;
				}
				sum++;
			}
			continue;
		}
		
		double value = max_score-mCurSequence.score;
		
		for(int j = 2 ;j<mCurSequence.tagSequence.size();j++)
		{
			int pos_id = mCurSequence.tagSequence[j];
			int flag = false;
			for(int k = 0;k<pos_array[j].size();k++)
			{
				if(pos_array[j][k] == pos_id)
					flag = true;
			}
			if(flag == false)
			{
				OPTION tag_Options;
				tag_Options.score = value;
				pos_array[j].push_back(pos_id);
				if(pos_id == map_pos_gold[j-2])
				{
					tag_Options.tag = 1;
				}
				else
				{
					tag_Options.tag = 0;		
				}
				g_all_result.push_back(tag_Options);
				if(g_filter_value>-0.5 && value<=g_filter_value)
				{
					pos_result[j] += IDToTag(pos_id);
					pos_result[j] +=" ";	
				}			
			}
		}
	}

	for(int j = 2 ;j<vectIn.size()-2;j++)
	{
		result += vectIn[j];
		result += " ";
		result +=pos_result[j];
		result +="\n";
	}
	
	return result;
}


string CPosMaxEn::N_Best_Result(vector<string>& vectIn,int& right,int& sum,double margin)
{
	PriorHeap mLastHeap = *m_Heap;

	int mMaxCount = mLastHeap.size();
	if(mMaxCount == 0)
	{
		cout<<"there is error in N_Best_result"<<endl;
		for(int iter = 2;iter<vectIn.size()-3;iter++)
		{
			cout<<vectIn[iter]<<endl;
		}
		exit(1);
	}
	vector< vector<int> > pos_array;
	pos_array.resize(5000);
	if (mMaxCount>=10)
		mMaxCount = 10;
	double max_score = 0.0f;
	for(int i = 0;i<mMaxCount;i++)
	{
		SEQUENCE mCurSequence = mLastHeap.top();
		mLastHeap.pop();
		if(i == 0)
		{
			max_score = mCurSequence.score;
		}
		if((max_score-mCurSequence.score)<=margin)
		{
			for(int j = 2 ;j<mCurSequence.tagSequence.size();j++)
			{
				int pos_id = mCurSequence.tagSequence[j];
				int flag = false;
				for(int k = 0;k<pos_array[j].size();k++)
				{
					if(pos_array[j][k] == pos_id)
						flag = true;
				}
				if(flag == false)
				{
					pos_array[j].push_back(pos_id);
				}
			}
		}
		else
		{
			break;
		}

	}
	string str;
	for(int i = 2;i<vectIn.size()-3;i++)
	{
		str+=vectIn[i];
		str+=" ";
		for(int k = 0;k<pos_array[i].size();k++)
		{
			if(pos_array[i][k] == map_pos_gold[i-2])
			{
				right++;
			}
			//string tag = 
			str+= IDToTag(pos_array[i][k]);
			str+=" ";
			sum ++;
		}
		str+="\n";
	}
	return str;
}


string CPosMaxEn::Single_Best_Result(vector<string>& vectIn)
{

	int mMaxCount = m_Heap->size();
	if(mMaxCount == 0)
	{
		cout<<"there is error in N_Best_result"<<endl;
		for(int iter = 2;iter<vectIn.size()-3;iter++)
		{
			cout<<vectIn[iter]<<endl;
		}
		exit(1);
	}
	vector< vector<int> > pos_array;
	pos_array.resize(5000);

	SEQUENCE mCurSequence = m_Heap->top();
	m_Heap->pop();
	for(int j = 2 ;j<mCurSequence.tagSequence.size();j++)
	{
		int pos_id = mCurSequence.tagSequence[j];
		pos_array[j].push_back(pos_id);
	}
	
		
	string str;
	for(int i = 2;i<vectIn.size()-3;i++)
	{
		str+=vectIn[i];
		str+=" ";
		
		if(pos_array[i][0] == map_pos_gold[i-2])
		{
			g_right++;
		}
			//string tag = 
		str+= IDToTag(pos_array[i][0]);
		g_sum++;	
		str+="\n";
	}
	return str;
}

void CPosMaxEn::FillWnd(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence, int PresentPOS, SAMPLE* wnd_pointer[], SAMPLE wnd[])
{
	vector<int>& vectPosID = ASequence.tagSequence;
	int mTag1 = -1, mTag2 = -1;
	if (vectPosID.size() >= 1) {mTag1 = vectPosID[vectPosID.size() - 1];}
	if (vectPosID.size() >= 2) {mTag2 = vectPosID[vectPosID.size() - 2];}

	wnd[0].word = vectIn[AIndex-2];
	if(mTag2 == -1)
		wnd[0].tag = "";
	else
		wnd[0].tag = IDToTag(mTag2);
	
	wnd[1].word = vectIn[AIndex-1];
	if(mTag1 == -1)
		wnd[1].tag = "";
	else
		wnd[1].tag = IDToTag(mTag1);
	
	wnd[2].word = vectIn[AIndex];
	wnd[2].tag = IDToTag(PresentPOS);

	wnd[3].word = vectIn[AIndex+1];
	wnd[4].word = vectIn[AIndex+2];

	for(int i = 0;i<WND_SIZE;i++)
	{
		wnd_pointer[i] = & wnd[i];
	}
}

void CPosMaxEn::FillWndContext(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence, int PresentPOS, SAMPLE* wnd_pointer[], SAMPLE wnd[],string& context_pre,string& context_cur)
{
	vector<int>& vectPosID = ASequence.tagSequence;
	int mTag1 = -1, mTag2 = -1;
	if (vectPosID.size() >= 1) {mTag1 = vectPosID[vectPosID.size() - 1];}
	if (vectPosID.size() >= 2) {mTag2 = vectPosID[vectPosID.size() - 2];}

	wnd[0].word = vectIn[AIndex-2];
	if(mTag2 == -1)
		wnd[0].tag = "";
	else
		wnd[0].tag = IDToTag(mTag2);
	

	wnd[1].word = vectIn[AIndex-1];
	if(mTag1 == -1)
		wnd[1].tag = "";
	else
		wnd[1].tag = IDToTag(mTag1);
	wnd[1].middle = context_pre;
	
	wnd[2].word = vectIn[AIndex];
	wnd[2].tag = IDToTag(PresentPOS);
	wnd[2].middle = context_cur;

	wnd[3].word = vectIn[AIndex+1];
	wnd[4].word = vectIn[AIndex+2];

	for(int i = 0;i<WND_SIZE;i++)
	{
		wnd_pointer[i] = & wnd[i];
	}
}

int CPosMaxEn::GetProbability_Based_All(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence , vector<int> & vectPOS, vector<double> & vectProbability_options)
{
	
	/*if(vectIn[AIndex] =="leven")
	{
		int zzzz = 5;
	}*/

	SAMPLE* wnd_pointer[WND_SIZE];
	SAMPLE wnd[WND_SIZE];
		
	FillWnd(vectIn, AIndex, ASequence,0, wnd_pointer,wnd);

	PREDICATE predicate;
	int mLoc;
	
	//vector<double>  vectProbability;
	//vectProbability.resize(m_numTag);	

	double sum = 0.0f;
	vector<double> prab;
	vector<int> corr;
	vector<int> pos_vect;
	prab.resize(m_numTag,1.0);
	corr.resize(m_numTag,10);
	//prab.assign(m_numTag,0.0);

	for(int i = 0;i<m_templateVect.size();i++)
	{
		predicate.predData = "";
		if(GetFeatureFromTemplate(predicate, wnd_pointer,m_templateVect[i]))
		{
			mLoc = IndexOfPredicate(predicate);
			if (mLoc != -1)
			{
				PREDICATE & mCurPredicate = m_vectPredicates[mLoc];
				vector<pair<int,int> >::iterator s1_Iter = mCurPredicate.indexFeature.begin();
				while(s1_Iter != mCurPredicate.indexFeature.end())
				{	
					prab[s1_Iter->first]+=m_vectFeatures[s1_Iter->second].alpha;
					if(corr[s1_Iter->first]==10)
					{
						pos_vect.push_back(s1_Iter->first);
						prab[s1_Iter->first]-=1.0;
					}
					corr[s1_Iter->first]--;	
					s1_Iter++;
				}

				

			}
		}
	}

/*	for(int i = 0;i<m_numTag;i++)
	{
		prab[i] = exp(prab[i]);
		sum+=prab[i];
	}
	for(int i = 0;i<m_numTag;i++)
	{
		vectProbability[i] = prab[i]/sum;
	}*/
	double psum = 0.0f;
	vector<int>::iterator it;
	for(it=pos_vect.begin();it!=pos_vect.end();it++)
	{
		int i = *it;
		prab[i] = exp(prab[i]);
		psum += prab[i];
	}
	psum += (m_numTag-pos_vect.size());


	//return the result back;
	for(int i = 0;i<vectPOS.size();i++)
	{
		//vectProbability_options.push_back(vectProbability[vectPOS[i]]);
		vectProbability_options.push_back(prab[vectPOS[i]]/psum);
	}
}

int CPosMaxEn::GetProbability_Based_All_Context(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence , vector<int> & vectPOS, vector<double> & vectProbability_options,string & context_pre, string& context_cur)
{
	
	/*if(vectIn[AIndex] =="leven")
	{
		int zzzz = 5;
	}*/

	SAMPLE* wnd_pointer[WND_SIZE];
	SAMPLE wnd[WND_SIZE];
		
	FillWndContext(vectIn, AIndex, ASequence,0, wnd_pointer,wnd,context_pre,context_cur);

	PREDICATE predicate;
	int mLoc;

	double sum = 0.0f;
	vector<double> prab (m_numTag,1.0);
	vector<int> corr (m_numTag,20);
	vector<int> pos_vect ;

	for(int i = 0;i<m_templateVect.size();i++)
	{
		predicate.predData = "";
		if(GetFeatureFromTemplate(predicate, wnd_pointer,m_templateVect[i]))
		{
			mLoc = IndexOfPredicate(predicate);
			if (mLoc != -1)
			{
				PREDICATE & mCurPredicate = m_vectPredicates[mLoc];
				vector<pair<int,int> >::iterator s1_Iter = mCurPredicate.indexFeature.begin();
				while(s1_Iter != mCurPredicate.indexFeature.end())
				{	
					prab[s1_Iter->first]+=m_vectFeatures[s1_Iter->second].alpha;
					if(corr[s1_Iter->first]==20)
					{
						pos_vect.push_back(s1_Iter->first);
						prab[s1_Iter->first]-=1.0;
					}
					corr[s1_Iter->first]--;	
					s1_Iter++;
				}

				

			}
		}
	}

	double psum = 0.0f;
	vector<int>::iterator it;
	for(it=pos_vect.begin();it!=pos_vect.end();it++)
	{
		int i = *it;
		prab[i] = exp(prab[i]);
		psum += prab[i];
	}
	psum += (m_numTag-pos_vect.size());


	//return the result back;
	for(int i = 0;i<vectPOS.size();i++)
	{
		vectProbability_options.push_back(prab[vectPOS[i]]/psum);
	}
	

	

	/*
	vector<int>::iterator it;
	//for(it=pos_vect.begin();it!=pos_vect.end();it++)
	for(int i = 0;i<m_numTag;i++)
	{
	//	int i = *it;
		prab[i] = exp(prab[i]);
		sum+=prab[i];
	}
	//for(it=pos_vect.begin();it!=pos_vect.end();it++)
	for(int i = 0;i<m_numTag;i++)
	{
	//	int i = *it;
		vectProbability[i] = prab[i]/sum;
	}

	//return the result back;
	for(int i = 0;i<vectPOS.size();i++)
	{
		vectProbability_options.push_back(vectProbability[vectPOS[i]]);
	}*/
}


int CPosMaxEn::GetProbability(vector<string>& vectIn ,int AIndex, SEQUENCE & ASequence , vector<int> & vectPOS, vector<double> & vectProbability)
{
	
	/*if(vectIn[AIndex] =="leven")
	{
		int zzzz = 5;
	}*/
	double sum = 0.0f;
	vector<double> prab;
	for(int i = 0;i<vectPOS.size();i++)
	{
		FEATURE ft;
		ft.outTag = vectPOS[i];

		SAMPLE* wnd_pointer[WND_SIZE];
		SAMPLE wnd[WND_SIZE];
		
		FillWnd(vectIn, AIndex, ASequence,vectPOS[i], wnd_pointer,wnd);
		
		int countFeature = 0;
		double all_features = 0.0f; 
		for(int j = 0;j<m_templateVect.size();j++)
		{
			ft.predData = "";
			if(GetFeatureFromTemplate(ft, wnd_pointer,m_templateVect[j]))
			{
				int index = LookUpFeature(ft);
				if (index != -1)
				{
					FEATURE & ftTemp = m_vectFeatures[index];
					all_features += ftTemp.alpha;
					countFeature++;
				}
			}
		}

		if(countFeature == 0)
		{
			prab.push_back(0.0f);
		}
		else
		{
			prab.push_back(exp(all_features));
			sum += exp(all_features); 
		}
	}

	if(sum >0.0f)
	{
		for(int i = 0;i<vectPOS.size();i++)
		{
			double value = prab[i]/sum;
			if(value>0.0f)
			{
				vectProbability.push_back(value);
			}
			else
			{
				vectProbability.push_back(0.00000001f);
			}
		}
	}
	else
	{
		for(int i = 0;i<vectPOS.size();i++)
		{
			vectProbability.push_back(1.0f/vectPOS.size());
		}
	}
}

bool CPosMaxEn::IsValidFeature(int k)
{
	int type = m_vectFeatures[k].predType;
	int count = m_filter_count[type];
	if(m_feature_count[k]>count)
		return true;
	return false;
}

int CPosMaxEn::FilterFeatures()
{
	cout << "Filtering Features ....." << endl;

	vector<FEATURE> mFilter;  
	vector<int> mFeatureCount;

	size_t mSize;
	mSize = m_vectFeatures.size();
	cout<<"before filtering, features count is "<<mSize<<endl;
	for ( int k = 0; k < mSize; k ++)
	{
		
		//if (m_feature_count[k]> g_threshold)
		if(IsValidFeature(k))
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




void CPosMaxEn::LoadTiggerFeature(string AFileName)
{


}




