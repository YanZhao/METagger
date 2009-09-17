/***************************************************************************
 *   Copyright (C) 2008 by Yan Zhao   *
 *   yzhao@holde   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "CRF.h"
#include "JBaseProc.h"
#include "Verify.h"
#include <fstream>
#include <math.h>

using namespace std;


int g_Bigram = 0;
//int g_debug = 0;
int g_sumSen = 0;
int g_lookSen = 0;

CPosCRF::CPosCRF(void)
{
	m_BeginSample.word = "";
	m_BeginSample.tag = "";
	if(g_Bigram)
	{
		m_fileNameModel = "model/"+APPLICATION+"/CRFBigram.bin";
	}
	else
	{
		m_fileNameModel = "model/"+APPLICATION+"/CRFUnigram.bin";
	}
}

CPosCRF::~CPosCRF(void)
{
}

int CPosCRF::ReadSamplesAndCollectFeature(string& fileName)
{
	cout << "collecting features...... " << endl;
	
	fstream fsample;
	fsample.open(fileName.c_str(),ios::in);
	VERIFY_FILE(fsample);

	SAMPLE mSample;
	m_BeginSample.tag = "";
	m_vectSamples.push_back(m_BeginSample);
	m_BeginSample.tag = START_TAG;
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
		//if(mSample.word == "W@#AD20030408-48-2-7W@#")
		
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
			m_BeginSample.tag = "";
			m_vectSamples.push_back(m_BeginSample);
			m_BeginSample.tag = START_TAG;
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

template<class T>
bool CPosCRF::GetFeatureFromTemplate(T &ft, SAMPLE * wnd[], string & templateStr)
{	
	vector<string> temp;
	String_SeperateToList_WithTrim(templateStr,temp,":");
	//ft.predType = atoi(temp[0].c_str());
	string content = "";
	content += temp[0];
	content += SPLIT_TAG;
	int pos_index,con_index;
	for(int i = 1;i<temp.size();i++)
	{
		AnalysizeTemplateString(temp[i],pos_index,con_index);
		
		if(con_index ==0 && wnd[pos_index+WND_INDEX]->word != "")
		{
			content+=wnd[pos_index+WND_INDEX]->word;
			content +=SPLIT_TAG;	
		}
		else if(con_index ==1 && wnd[WND_INDEX]->middle ==""&&wnd[pos_index+WND_INDEX]->tag!="")
		{
			content+=wnd[pos_index+WND_INDEX]->tag;
			content +=SPLIT_TAG;
		}

		else if(con_index ==1 && wnd[WND_INDEX]->middle !=""&&wnd[pos_index+WND_INDEX]->middle!="")
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

void CPosCRF::SaveFeaturesToTxt(string AFileName)
{
	fstream f1;
	f1.open(AFileName.c_str(),ios::out);
	VERIFY_FILE(f1);
//f1 <<"predType\tpredData\toutTag\tcount\tE\talpha\tmod"<<endl;

	unsigned int k;
	int length;
	for (k = 0; k < m_vectFeatures.size(); k ++)
	{
		
		f1 <<m_vectFeatures[k].predType << "\t" ;
		if(m_vectFeatures[k].predData !="")
		{
			f1<<m_vectFeatures[k].predData ;
			length = m_vectFeatures[k].predData.length();
		}
		else
		{
			f1<<"***";
			length = 3;
		}
		int numTab = length/8;
		for(;numTab <4;numTab++)
		{
			f1<<"\t";
		}
		
		if(m_vectFeatures[k].previousTag>-1)
		{
			if(m_vectFeatures[k].predType == 3)
			{
				f1<<IDToTag(m_vectFeatures[k].previousTag);
				length = IDToTag(m_vectFeatures[k].previousTag).length();
			}
			else
			{
				f1<<IDToBigramTag(m_vectFeatures[k].previousTag);
				length = IDToBigramTag(m_vectFeatures[k].previousTag).length();
			}
			
		}
		else
		{	
			f1<<"***";
			length = 3;
		}

		numTab = length/8;
		for(;numTab <4;numTab++)
		{
			f1<<"\t";
		}
		
		if(m_vectFeatures[k].outTag >-1)
		{
			if(m_vectFeatures[k].predType == 1 ||m_vectFeatures[k].predType == 3)
			{
				f1<<IDToTag(m_vectFeatures[k].outTag);
				length = IDToTag(m_vectFeatures[k].outTag).length();
			}
			else
			{
				f1<<IDToBigramTag(m_vectFeatures[k].outTag);
				length = IDToBigramTag(m_vectFeatures[k].outTag).length();
			}
		}
		else
		{	
			f1<<"***";
			length = 3;
		}
		numTab = length/8;
		for(;numTab <4;numTab++)
		{
			f1<<"\t";
		}

		f1<<m_vectFeatures[k].alpha<<endl;
		
	}
	///////////////////////////////////////////////////////////////////		
	f1.close();

}

void CPosCRF::ExtractFeatureFromWnd(SAMPLE * wnd[])
{

	FEATURE ft;
	//produce state 1 feature
	ft.predType = State_Feature_1;
	ft.outTag = TagToID(wnd[WND_INDEX]->tag);
	for(int i = 0;i<m_templateVect.size();i++)
	{
		ft.predData = "";
		if(GetFeatureFromTemplate(ft, wnd,m_templateVect[i]))
		{
			LookupAndUpdateFeature(ft);
		}
	}
	
	//produce state 2 feature
	if(g_Bigram == true)
	{
		ft.predType = State_Feature_2;
		string strKey = wnd[WND_INDEX-1]->tag+SPLIT_TAG+wnd[WND_INDEX]->tag;
		if(BigramTagToID(strKey)>-1)
		{
			ft.outTag = BigramTagToID(strKey);
		}
		for(int i = 0;i<m_templateVect.size();i++)
		{
			ft.predData = "";
			if(GetFeatureFromTemplate(ft, wnd,m_templateVect[i]))
			{
				LookupAndUpdateFeature(ft);
			}
		}
	}
	
	//produce edge 1 feature
	ft.predType = Edge_Feature_1;
	ft.predData = "";	
	ft.outTag = TagToID(wnd[WND_INDEX]->tag);
	ft.previousTag = TagToID(wnd[WND_INDEX-1]->tag);
	//I feel a little surprised about this.
	//m_numTag-1 equals the ID of Tag START_TAG;
	if(g_Bigram == true)
	{
		//when I use Bigram, I don't want to include the START_TAG&First_Tag 
		if(ft.outTag>-1&&(ft.previousTag>-1&&ft.previousTag!= (m_numTag-1))) 
		{
			LookupAndUpdateFeature(ft);
		}
	}
	else
	{
		if(ft.outTag>-1&&ft.previousTag>-1) 
		{
			LookupAndUpdateFeature(ft);
		}
	}

	//produce edge 2 feature
	if(g_Bigram == true)
	{
		ft.predType = Edge_Feature_2;
		string strKey = wnd[WND_INDEX-2]->tag+SPLIT_TAG+wnd[WND_INDEX-1]->tag;
		ft.previousTag = BigramTagToID(strKey);
		strKey = wnd[WND_INDEX-1]->tag+SPLIT_TAG+wnd[WND_INDEX]->tag;
		ft.outTag = BigramTagToID(strKey);
	
		if(ft.outTag>-1&&ft.previousTag>-1)
		{
			LookupAndUpdateFeature(ft);
		}
	}
}

void CPosCRF::ExtractEventFromWnd(SAMPLE * wnd[],vector<EVENT> &vectEvent,bool flag)
{
	EVENT ev;

	ev.outTag = TagToID(wnd[WND_INDEX]->tag);
	ev.BigramTag = BigramTagToID(wnd[WND_INDEX-1]->tag +SPLIT_TAG+wnd[WND_INDEX]->tag);
	ev.count = 1;  

	int numPredicate = 0;
	PREDICATE predicate;
	int mLoc;


	predicate.predType = State_Feature_1;
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

	predicate.predType = State_Feature_2;
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
	//when training , I need store all the events in the training into m_EventList
	//when tagging, I only need store all the events in a sentence into vectEvent
	if(flag == 0)
	{
		m_EventList.WriteEvent(ev);
	}
	else
	{
		vectEvent.push_back(ev);
	}
	
}

int CPosCRF::IndexOfPredicate(PREDICATE &APre_Type)  
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

int CPosCRF::CollectEvents(string& fileName)
{
	cout<<"Collect Events .........."<<endl;
	SAMPLE* wnd[WND_SIZE];
	vector<EVENT> vectTemp;
	for (int k = 0; k < WND_SIZE; k ++)
	{
		wnd[k] = 0;
	}

	// 6000000 means that the training file is big enough, we can't load all events into the memory. we use file on disk to store all events. 
	if (g_sumTrainLine > 6000000) 
	{ 
		m_EventList.SetEventOperatorMode(OperatorMode_File); 
	}  
	
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

		if(mSample.word == "W@#AD20030118-1-2-3W@#")
		{
			g_lookSen = g_sumSen;
		}
		
		
		if(mSample.word.substr(0,3) == SEN_SPLIT && m_vectSamples.size()>2)
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
					ExtractEventFromWnd(wnd,vectTemp,0);
				}
			}
			EVENT ev;
			ev.outTag = EVENT_SPLIT;
			g_sumSen++;
			m_EventList.WriteEvent(ev);
			m_vectSamples.clear();	
			m_vectSamples.push_back(m_BeginSample);
			m_vectSamples.push_back(m_BeginSample);
			
		}
		else
		{
			mSample.tag = vectline[vectline.size()-1];
			m_vectSamples.push_back(mSample);
		}
		
	}while(!fsample.eof());
	////////////////////////////////////////////////
	fsample.close();
	
	if(m_vectSamples.size()>2)
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
				ExtractEventFromWnd(wnd,vectTemp,0);
			}
		}
		EVENT ev;
		ev.outTag = EVENT_SPLIT;
		g_sumSen++;
		m_EventList.WriteEvent(ev);
		
	}
	m_EventList.CloseEventList();
	cout << "Events Count is " << m_EventList.TotalEventCount() << endl;
	cout <<"Number of Sentence is "<<g_sumSen<<endl;
	cout<<"Collect Events is ok!"<<endl;

	return 0;
}

void CPosCRF::LookupAndUpdateFeature(FEATURE &AFeature)
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

int CPosCRF::GetBigramPosList()
{
	cout<<"building BigramTag Map..."<<endl;
	map<string,int>::iterator it1 =m_mapPosID.begin();
	int id = 0;
	while(it1 != m_mapPosID.end())
	{
		map<string,int>::iterator it2 = m_mapPosID.begin();
		while(it2 != m_mapPosID.end())
		{
			id = it1->second*m_numTag + it2->second;

			pair<string, int> ptemp1;
			ptemp1.first = it1->first +SPLIT_TAG+it2->first;
			ptemp1.second = id;

			pair<int, string> ptemp2;
			ptemp2.first = id;
			ptemp2.second = it1->first +SPLIT_TAG+it2->first;
	
			m_mapBigramPosID.insert(ptemp1);
			m_mapIDBigramPos.insert(ptemp2);
			it2++;
		}
		it1++;	
	}

	/////////////////////////////////////////////////////////////
	FILE* fout;
	string fileName = "model/"+APPLICATION+"/BigramTagIDMap";
	fout = fopen(fileName.c_str(), "w");
	int i = 0;
	map<string,int>::iterator s1_Iter; 
	s1_Iter = m_mapBigramPosID.begin( );
	while(s1_Iter != m_mapBigramPosID.end())
	{	
		pair<string, int> ptemp1 = *s1_Iter;
		fprintf(fout, "%d %s\n",  ptemp1.second, ptemp1.first.c_str());
		s1_Iter++;
	}
	fclose(fout);
	cout<<"building BigramTag Map finish"<<endl;
	return 0;
}

void CPosCRF::GetEdgeFeature()
{
	//get all edge features;
	m_vectEdgeFeatures.clear();
	Edge_1_Index.clear();
	int num_edge1 = 0;
	int num_edge2 = 0;
	for (int k = 0; k < m_vectFeatures.size(); k ++)
	{
		if(m_vectFeatures[k].predType == Edge_Feature_1)
		{
			num_edge1++;
			m_vectEdgeFeatures.push_back(k);
			//for Bigram model, the edge1 will not be added to SparseMatrix.
			if(g_Bigram == false)
			{
				SparseMatrix.AddToMap(m_vectFeatures[k].previousTag, m_vectFeatures[k].outTag,0.0);
			}
			else
			{
				Edge_1_Index.push_back(m_vectFeatures[k].previousTag*m_numTag+m_vectFeatures[k].outTag);
			}
		}
		if(m_vectFeatures[k].predType == Edge_Feature_2)
		{
			num_edge2++;
			m_vectEdgeFeatures.push_back(k);
			SparseMatrix.AddToMap(m_vectFeatures[k].previousTag, m_vectFeatures[k].outTag,0.0);
		}
	}

	if(g_Bigram)
	{
		SparseMatrix.BuildRowColIndex(m_numBigramTag);
	}
	else
	{
		SparseMatrix.BuildRowColIndex(m_numTag);
	}

	cout<<"Size of Edge1 is "<<num_edge1<<endl;
	cout<<"Size of Edge2 is "<<num_edge2<<endl;
}

int CPosCRF::Train(string& fileName)
{
	LoadTemplate(g_templateFileName);

	GetPosList(fileName);
	m_numTag = LoadPosTag("model/"+APPLICATION+"/CRF_TagIDMap");
	cout<< "number of Tag "<<m_numTag<<endl;
	if(g_Bigram == true)
	{
		m_numBigramTag = m_numTag*m_numTag;
		GetBigramPosList();
	}

	ReadSamplesAndCollectFeature(fileName);

	FilterFeatures();

	GetEdgeFeature();	

	SaveFeaturesToTxt("model/"+APPLICATION+"/Features.txt");

	GetPredictsFromFeatures();

	CollectEvents(fileName);

	LBFGS_Estimate();
	
	SaveModel(0);
	
	SaveFeaturesToTxt("model/"+APPLICATION+"/LastFeatures.txt");

	cout << "Training is ok" << endl;
	return 0;

}
void CPosCRF::InitTrain()
{
	cout<<"Now Init training...."<<endl;
	m_numFeatures =  m_vectFeatures.size();
	gradlogli = new double[m_numFeatures];
	
	//Mi = new doublematrix(num_labels,num_labels);

	if(g_Bigram == true)
	{
		Vi =  new doublevector(m_numBigramTag);
		Vi_Edge1 = new doublevector(m_numBigramTag);
		alpha = new doublevector(m_numBigramTag);
    		next_alpha = new doublevector(m_numBigramTag);
    		temp_vector = new doublevector(m_numBigramTag);
	}
	else
	{
		Vi =  new doublevector(m_numTag);
		alpha = new doublevector(m_numTag);
    		next_alpha = new doublevector(m_numTag);
    		temp_vector = new doublevector(m_numTag);
	}

	lambda = new double[m_numFeatures];
	temp_lambda = new double[m_numFeatures];

    	// allocate memory for vector of feature expectations    
    	ExpF = new double[m_numFeatures];   
	for(int k = 0;k<m_numFeatures;k++)
	{
		ExpF[k] = 0;
	} 

    	// allocate memory for ws (workspace)
    	// this memory is only for LBFGS optimization
    	
	diag = new double[m_numFeatures];
	iprint = new int[2];

	double init_valu = 0.05;
	for (int i = 0; i < m_numFeatures; i++) 
	{
		lambda[i] = init_valu;
		temp_lambda[i] = init_valu;
    	}
	 
	cout<<"Init training finished"<<endl;
}

void CPosCRF::Likelihood_Gradient_Bigram_Event(EVENT & event, EVENT & pre_event, int position,double& seq_logli )
{	
	for(int i = 0;i<event.vectIndexPredicate.size();i++)
	{			
		PREDICATE & mCurPredicate = m_vectPredicates[event.vectIndexPredicate[i]]; 
 
		vector<pair<int,int> >::iterator s1_Iter = mCurPredicate.indexFeature.begin();
		while(s1_Iter != mCurPredicate.indexFeature.end())
		{	
			if(mCurPredicate.predType == State_Feature_1)
			{
				//pair<int,int> pValue = *s1_Iter;
				if(s1_Iter->first == event.outTag)
				{
					seq_logli += lambda[s1_Iter->second];
				}
			
				for(int i = 0;i<m_numTag;i++)
				{
					int index = i* m_numTag+m_vectFeatures[s1_Iter->second].outTag;
			 		ExpF[s1_Iter->second] += (*next_alpha)[index]*(*(betas[position]))[index]; 
					
				}
			}

			if(mCurPredicate.predType == State_Feature_2)
			{
				//pair<int,int> pValue = *s1_Iter;
				if(s1_Iter->first == event.BigramTag)
				{
					seq_logli += lambda[s1_Iter->second];
				}
				ExpF[s1_Iter->second] += (*next_alpha)[s1_Iter->first]*(*(betas[position]))[s1_Iter->first]; 
				
			}
			s1_Iter++;
		}	
	}

	for(int i  = 0;i<m_vectEdgeFeatures.size();i++)
	{
		int index_feature = m_vectEdgeFeatures[i];
		FEATURE & ftemp = m_vectFeatures[index_feature];
		if(ftemp.predType == Edge_Feature_1)
		{
			int index = ftemp.previousTag*m_numTag+ftemp.outTag;
			ExpF[index_feature] +=(*next_alpha)[index]* (*(betas[position]))[index];

			if(ftemp.outTag == event.outTag)
			{
				if((position == 0 && ftemp.previousTag == (m_numTag-1)) ||
				(position>0&& ftemp.previousTag == pre_event.outTag))
				{
					seq_logli += lambda[index_feature];
				}
			}
		}	
	
		if(ftemp.predType == Edge_Feature_2)
		{
			ExpF[index_feature] +=(*alpha)[ftemp.previousTag]*(*Vi)[ftemp.outTag]*SparseMatrix.Get(ftemp.previousTag,ftemp.outTag)* (*(betas[position]))[ftemp.outTag];
	
			if(ftemp.outTag == event.BigramTag && (position>0 && ftemp.previousTag == pre_event.BigramTag))
			{
				seq_logli += lambda[index_feature];
			}
		}
	}
}

void CPosCRF::Likelihood_Gradient_Unigram_Event(EVENT & event, EVENT & pre_event, int position,double& seq_logli )
{
	
	for(int i = 0;i<event.vectIndexPredicate.size();i++)
	{			
		PREDICATE & mCurPredicate = m_vectPredicates[event.vectIndexPredicate[i]]; 
 
		vector<pair<int,int> >::iterator s1_Iter = mCurPredicate.indexFeature.begin();
		while(s1_Iter != mCurPredicate.indexFeature.end())
		{	
			//if(mCurPredicate.predType == State_Feature_1)
			//{
				//pair<int,int> pValue = *s1_Iter;
				if(s1_Iter->first == event.outTag)
				{
					//gradlogli[pValue.second] +=1.0;
					seq_logli += lambda[s1_Iter->second];
				}
			 	ExpF[s1_Iter->second] += (*next_alpha)[s1_Iter->first] *  (*(betas[position]))[s1_Iter->first]; 
						
			//}
			s1_Iter++;
		}	
	}

	for(int i  = 0;i<m_vectEdgeFeatures.size();i++)
	{
		int index_feature = m_vectEdgeFeatures[i];
		FEATURE & ftemp = m_vectFeatures[index_feature];
		
		if(position>0)
		{
			if(ftemp.outTag == event.outTag && ftemp.previousTag == pre_event.outTag)
			{
				//gradlogli[index_feature]+=1.0;
				seq_logli += lambda[index_feature];
			}
		}

		ExpF[index_feature] +=(*alpha)[ftemp.previousTag]*(*Vi)[ftemp.outTag]* SparseMatrix.Get(ftemp.previousTag,ftemp.outTag)*(*(betas[position]))[ftemp.outTag];	
	}
}

void CPosCRF::Get_Mi_Vi_Unigram(EVENT & event, doublevector* Vi, int is_exp)
{
	//*Mi = 0.0;
	//SparseMatrix.Init();
    	*Vi = 0.0;

	for(int i = 0;i<event.vectIndexPredicate.size();i++)
	{			
		PREDICATE & mCurPredicate = m_vectPredicates[event.vectIndexPredicate[i]]; 
 		vector<pair<int,int> >::iterator   s1_Iter;
		
		s1_Iter = mCurPredicate.indexFeature.begin();
		while(s1_Iter != mCurPredicate.indexFeature.end())
		{	
			//pair<int,int> pValue = *s1_Iter;
			(*Vi)[s1_Iter->first] += lambda[s1_Iter->second];
			//cout<<pValue.first<<"lambda"<<lambda[pValue.second]<<endl;
			//cout<<pValue.first<<"(*Vi)"<<(*Vi)[pValue.first]<<endl;
			s1_Iter++;
		}
		
	}

/*	for(int i  = 0;i<m_vectEdgeFeatures.size();i++)
	{
		int index = m_vectEdgeFeatures[i];
		FEATURE & ftemp = m_vectFeatures[index];
		if(ftemp.predType == Edge_Feature_1)
		{
			SparseMatrix.Add(ftemp.previousTag, ftemp.outTag,lambda[index]);
		}
	}
	*/
	
	if (is_exp) 
	{
		for(int i = 0;i<m_numTag;i++)
		{ 
			
	    		(*Vi)[i] = exp((*Vi)[i]);
		}
	//	SparseMatrix.AllExp();
	}
}


void CPosCRF::Get_Mi_Vi_Bigram(EVENT & event, doublevector* Vi, int is_exp)
{
    	*Vi = 1.0;
	for(int i = 0;i<event.vectIndexPredicate.size();i++)
	{			
		PREDICATE & mCurPredicate = m_vectPredicates[event.vectIndexPredicate[i]]; 
 		vector<pair<int,int> >::iterator  s1_Iter;
		if(mCurPredicate.predType ==State_Feature_1)
		{
			s1_Iter = mCurPredicate.indexFeature.begin();
			while(s1_Iter != mCurPredicate.indexFeature.end())
			{	
				//pair<int,int> pValue = *s1_Iter;
				for(int i = 0;i<m_numTag;i++)
				{
					(*Vi)[i*m_numTag+s1_Iter->first] *= exp(lambda[s1_Iter->second]);
				}
				s1_Iter++;
			}
		}
		
		if(mCurPredicate.predType ==State_Feature_2)
		{
			s1_Iter = mCurPredicate.indexFeature.begin();
			while(s1_Iter != mCurPredicate.indexFeature.end())
			{	
				//pair<int,int> pValue = *s1_Iter;
				(*Vi)[s1_Iter->first] *= exp(lambda[s1_Iter->second]);
				s1_Iter++;
			}
		}
	}

	if (is_exp) 
	{
		//for(int i = 0;i<m_numBigramTag;i++)
		for(int i = 0;i<Edge_1_Index.size();i++)
		{
			int k = Edge_1_Index[i];
			(*Vi)[k] = (*Vi)[k]*(*Vi_Edge1)[k];
		}
    	}
}


double CPosCRF::Likelihood_Gradient_Bigram_Sentence(vector<EVENT>& vectEvent)
{
	*alpha = 1;
	int sen_len = vectEvent.size(); //sentence length;
	int betassize = betas.size();

	for(int i = 0;i<m_numFeatures;i++)
	{
		ExpF[i] = 0;
	}
	
	if (betassize < sen_len) {
		// allocate more beta vector		
		for (int i = 0; i < sen_len - betassize; i++) {
		betas.push_back(new doublevector(m_numBigramTag));
		}	    
	}

	int scalesize = scale.size();
	if (scalesize < sen_len) {
		// allocate more scale elements
		for (int i = 0; i < sen_len - scalesize; i++) {
			scale.push_back(1.0);
		}
	}

	scale[sen_len - 1] = m_numBigramTag;
	betas[sen_len - 1]->assign(1.0 / scale[sen_len - 1]);			

	//now I have a single sentence
	for(int i = sen_len-1;i>0;i--)
	{
		Get_Mi_Vi_Bigram(vectEvent[i], Vi, 1);
		
		*temp_vector = *(betas[i]);
		temp_vector->comp_mult(Vi);
		//mathlib::mult(m_numTag*m_numTag,betas[i-1],Mi,temp,0);
		mathlib::mult_bigram(m_numBigramTag,betas[i-1],SparseMatrix,temp_vector,0);
		
		scale[i-1] = betas[i-1]->sum();
		betas[i-1]->comp_mult(1.0/scale[i-1]);
	}

	
	double seq_logli = 0;
	for(int j = 0;j<vectEvent.size();j++)
	{
		Get_Mi_Vi_Bigram(vectEvent[j], Vi, 1);
		if(j>0)
		{
			//*temp_vector = *alpha;
			//mathlib::mult(m_numTag*m_numTag, next_alpha,Mi,temp,1);
			mathlib::mult_bigram(m_numBigramTag,next_alpha,SparseMatrix,alpha/*temp_vector*/,1);
			next_alpha->comp_mult(Vi);
		}
		else
		{
			*next_alpha = *Vi;
		}
		if(j>0)
		{
			 Likelihood_Gradient_Bigram_Event(vectEvent[j],vectEvent[j-1],j,seq_logli);
		}
		else
		{
			Likelihood_Gradient_Bigram_Event(vectEvent[j],vectEvent[j],j,seq_logli);
		}

		*alpha = *next_alpha;
		alpha->comp_mult(1.0 / scale[j]);
	}
	long double Zx = alpha->sum();
	seq_logli -= log(Zx);
	for(int k = 0;k<sen_len;k++)
	{
		seq_logli -= log(scale[k]);
	}

	for(int k = 0;k<m_numFeatures;k++)
	{
		gradlogli[k] -= ExpF[k]/Zx;
	}

	vectEvent.clear();
	return seq_logli;
}

double CPosCRF::Likelihood_Gradient_Unigram_Sentence(vector<EVENT>& vectEvent,int ni)
{
	*alpha = 1;
	int sen_len = vectEvent.size(); //sentence length;
	int betassize = betas.size();

	if (betassize < sen_len) {
	// allocate more beta vector to the most length sentence, It is dynamic vector, good idea.	
		for (int i = 0; i < sen_len - betassize; i++) {
		betas.push_back(new doublevector(m_numTag));
		}	 
	}

	int scalesize = scale.size();
	if (scalesize < sen_len) {
	// allocate more scale elements
		for (int i = 0; i < sen_len - scalesize; i++) {
			scale.push_back(1.0);
		}
	}

	scale[sen_len - 1] = m_numTag;
	betas[sen_len - 1]->assign(1.0 / scale[sen_len - 1]);			

	//now I have a single sentence
	for(int i = sen_len-1;i>0;i--)
	{
		
		Get_Mi_Vi_Unigram(vectEvent[i], Vi, 1);
		*temp_vector = *(betas[i]);
		temp_vector->comp_mult(Vi);
		//mathlib::mult(m_numTag*m_numTag,betas[i-1],Mi,temp,0);
		mathlib::mult_unigram(m_numTag,betas[i-1],SparseMatrix,temp_vector,0);
	
		scale[i-1] = betas[i-1]->sum();
		betas[i-1]->comp_mult(1.0/scale[i-1]);
	}
	if(g_debug == 1)
	{
		for(int zzz = 0;zzz<vectEvent.size();zzz++)
		{
			cerr<<IDToTag(vectEvent[zzz].outTag)<<endl;
		}
	}
	
	double seq_logli = 0;
	for(int j = 0;j<vectEvent.size();j++)
	{		
		Get_Mi_Vi_Unigram(vectEvent[j], Vi, 1);
		if(j>0)
		{
			*temp_vector = *alpha;
			//mathlib::mult(m_numTag*m_numTag, next_alpha,Mi,temp,1);
			mathlib::mult_unigram(m_numTag,next_alpha,SparseMatrix,temp_vector,1);
			if(next_alpha->sum()<0.0)
			{
				cerr<<"mult is wrong"<<endl;
			}
			if((*next_alpha)[990]*(*Vi)[990]<0.0)
			{
				cerr<<"(*next_alpha)[990]"<<(*next_alpha)[990]<<endl;
				cerr<<"(*Vi)[990]"<<(*Vi)[990]<<endl;
			}
			next_alpha->comp_mult(Vi);
			//if(next_alpha->sum()<0.0)
			//{
			//}
			
		}
		else
		{
			*next_alpha = *Vi;
		}

		if(j>0)
		{
			 Likelihood_Gradient_Unigram_Event(vectEvent[j],vectEvent[j-1],j,seq_logli);
		}
		else
		{
			Likelihood_Gradient_Unigram_Event(vectEvent[j],vectEvent[j],j,seq_logli);
		}

		*alpha = *next_alpha;
		alpha->comp_mult(1.0 / scale[j]);
		if(alpha->sum()<0.0)
		{
		
			cerr<<"I find here"<<endl;
			cerr <<"j="<<j<<endl;
			cerr <<"scale[j]="<<scale[j]<<endl;
			cerr <<"senlen ="<<vectEvent.size()<<endl;
			for(int zzz = 0;zzz<vectEvent.size();zzz++)
			{
				cerr<<IDToTag(vectEvent[zzz].outTag)<<endl;
			}
			for(int zzz = 0;zzz<m_numTag;zzz++)
			{
				cerr<<"alpha"<< zzz <<" "<<(*alpha)[zzz]<<endl;
			}
	
			for(int zzz = 0;zzz<m_numTag;zzz++)
			{
				cerr<<"Vi"<< zzz <<" "<<(*Vi)[zzz]<<endl;
			}
			cerr<<"let take a look"<<endl;
		
		}
	}

	
	double Zx = alpha->sum();
	seq_logli -= log(Zx);
	//if(ni == 117)
	{
	//	cout<<"Zx ="<<Zx<<"  "<<"log(Zx) ="<<log(Zx)<<endl;
	}

	for(int k = 0;k<sen_len;k++)
	{
		seq_logli -= log(scale[k]);
	}

	for(int k = 0;k<m_numFeatures;k++)
	{
		gradlogli[k] -= ExpF[k]/Zx;
		ExpF[k] = 0;
	}
		
	vectEvent.clear();
	return seq_logli;
}

void CPosCRF::Get_Mi_Unigram() 
{
	SparseMatrix.Init();
	for(int i  = 0;i<m_vectEdgeFeatures.size();i++)
	{
		int index = m_vectEdgeFeatures[i];
		FEATURE & ftemp = m_vectFeatures[index];
		SparseMatrix.Add(ftemp.previousTag, ftemp.outTag,lambda[index]);
		
	}
	SparseMatrix.AllExp();
}

void CPosCRF::Get_Mi_Bigram()
{
	SparseMatrix.Init();
	*Vi_Edge1 = 1.0;
	
	for(int i  = 0;i<m_vectEdgeFeatures.size();i++)
	{
		int index = m_vectEdgeFeatures[i];
		FEATURE & ftemp = m_vectFeatures[index];

	//	if(ftemp.predType == Edge_Feature_1)
	//	{
	//		(*Vi)[ftemp.previousTag*m_numTag +  ftemp.outTag]+= lambda[index];
	//	}
		if(ftemp.predType == Edge_Feature_2)
		{
			//Mi->get(ftemp.previousTag, ftemp.outTag)+=lambda[index];
			SparseMatrix.Add(ftemp.previousTag, ftemp.outTag,lambda[index]);
		}
		
		if(ftemp.predType == Edge_Feature_1)
		{
			(*Vi_Edge1)[ftemp.previousTag*m_numTag +  ftemp.outTag] *= exp(lambda[index]);
		}

	}

	SparseMatrix.AllExp();
}

double CPosCRF::Likelihood_Gradient_Whole(double * lambda, double * gradlogli,int num_iters)
{
	double logLikelihood= 0.0;
	if(g_sigma>0.0)
	{
		for (int i = 0; i < m_numFeatures; i++) 
		{
			logLikelihood -= (lambda[i] * lambda[i]) / (2 * g_sigma);
			gradlogli[i] = (-1 * lambda[i] / g_sigma) +m_feature_count[i];
    		}
	}
	else
	{
		for (int i = 0; i < m_numFeatures; i++) 
		{
			gradlogli[i] = m_feature_count[i];
    		}
	}
	if(g_Bigram)
	{
		Get_Mi_Bigram();
		SparseMatrix.ValueIndex(m_numBigramTag);
	}
	else
	{
		Get_Mi_Unigram();
		//SparseMatrix.BuildRowColIndex(m_numTag);
		SparseMatrix.ValueIndex();
	}
	
	EVENT mCurEvent;
	vector< EVENT > vectEvent;
	m_EventList.OpenEventList(OpenMode_Read);
	int sen_num = 0;
	//the CRF is based on sentences, I need to splite eventlist into sentences by EVENT_SPLIT tag.
	int num_sen  = 0;
	while (m_EventList.ReadEvent(mCurEvent) == true)
	{
 		if(mCurEvent.outTag != EVENT_SPLIT)
		{
			vectEvent.push_back(mCurEvent);
		}
		else
		{
			if(num_sen == g_lookSen)
			{
				//g_debug = 1;
			}
			else
			{
				g_debug = 0;
			}
			if(g_Bigram == true)
			{
				logLikelihood +=Likelihood_Gradient_Bigram_Sentence(vectEvent);	
			}
			else
			{
				logLikelihood += Likelihood_Gradient_Unigram_Sentence(vectEvent,num_iters);
			}
			num_sen ++;
			//cout<<"num_sen"<<num_sen<<endl;
			if(g_Bigram)
			{
				if(num_sen % 100 == 0)
				{
					cout<<".";
					cout.flush();
				}
				if(num_sen % 1000 == 0)
				{
					cout<<num_sen;
					cout.flush();
				}
			}
		}				
	}
	
	m_EventList.CloseEventList();

	//give some detail information about each Iteration
	cout<<endl;
	cout<<"Iteration: "<< num_iters<<endl;
	cout<<"Log-likelihood: "<<logLikelihood<<endl;
	cout<<"Norm(gradient vector): "<< norm(m_numFeatures, gradlogli)<<endl;
	cout<<"Norm(lambda vector): "<< norm(m_numFeatures, lambda)<<endl;
	//if(num_iters == 63)
	//{
	//	
	//}
	

	return logLikelihood;
}

// compute norm of a vector
double CPosCRF::norm(int len, double * vect) {
    double res = 0.0;
    for (int i = 0; i < len; i++) {
	//cout<<"i= "<<i<<" vect= "<<vect[i]<<endl;
	res += vect[i] * vect[i];
    }
    return sqrt(res);
}

void CPosCRF::LBFGS_Estimate(void)
{
	cout<<"now begining training...."<<endl;
	InitTrain();
	int num_iters = 0;
	double f = 0.0;
	int iflag = 0;
	int diagco = 0;
	int hessian = 7;
	int ws_size = m_vectFeatures.size() * (2 * hessian + 1) + 2 * hessian;
    	ws = new double[ws_size];
	double xtol = 1.0e-16; // machine precision
	double eps_for_convergence = 0.0001;	
	do{
		
		f = Likelihood_Gradient_Whole(lambda, gradlogli, num_iters + 1);
		f *= -1;
		for(int i = 0; i<m_numFeatures;i++)
		{
			gradlogli[i] *=-1;
		}

		lbfgs(&m_numFeatures, &hessian, lambda, &f, gradlogli, &diagco, diag, iprint, &eps_for_convergence, &xtol, ws, &iflag);
		

		if(iflag<0)
		{
			cout<<"iflag = "<<iflag<<endl;
			printf("LBFGS routine encounters an error......\n");
			break;
		}
		
		if(iflag==0)
		{
			printf("LBFGS finish successfully.......\n");
			break;
		}
		
		
		if ((num_iters + 1) % 20 == 0 && g_syn_flag == 1) 
		{
			for(int i = 0; i<m_numFeatures;i++)
			{
				m_vectFeatures[i].alpha = lambda[i];
			}
			SaveModel(num_iters + 1);

			g_right = 0;
			g_sum = 0;
			TagFile(g_testingFileName,num_iters + 1);
			m_numTag = LoadPosTag("model/"+APPLICATION+"/CRF_TagIDMap");
			m_numBigramTag = m_numTag*m_numTag;
			cerr<<"g_templateFileName="<<g_templateFileName;
			cerr<<" c="<<g_threshold;
			cerr<<" g="<<g_sigma;
			cerr<<" iter_num= "<<num_iters+1;
			cerr<<" "<<double(g_right*100)/g_sum<<endl;
			
		}
		num_iters++;
		
		vector<double> vetemp;
		for(int i = 0; i<m_numFeatures;i++)
		{
			vetemp.push_back(lambda[i]);
		}
		sort(vetemp.begin(),vetemp.end());
		for(int i = 0; i<10;i++)
		{
			cout.precision(16);
			cout<<vetemp[i]<<endl;
		}
		cout<<"++++++++++++++++++++++++++++++++++++++"<<endl;
	
		for(int i = vetemp.size()-1; i>vetemp.size()-10;i--)
		{
			cout.precision(16);
			cout<<vetemp[i]<<endl;
		}

		cout<<"run for "<<((double)(clock()-startClk))/CLOCKS_PER_SEC<<endl;

	}while(iflag !=0 && num_iters<g_iteration);

	vector<double> vetemp;
		for(int i = 0; i<m_numFeatures;i++)
		{
			vetemp.push_back(lambda[i]);
		}
		sort(vetemp.begin(),vetemp.end());
		for(int i = 0; i<10;i++)
		{
			cout.precision(16);
			cout<<vetemp[i]<<endl;
		}
		cout<<"++++++++++++++++++++++++++++++++++++++"<<endl;
	
		for(int i = vetemp.size()-1; i>vetemp.size()-10;i--)
		{
			cout.precision(16);
			cout<<vetemp[i]<<endl;
		}


	for(int i = 0; i<m_numFeatures;i++)
	{
		m_vectFeatures[i].alpha = lambda[i];
	}
	
	//this can be deleted in the near future.
	

	delete [] gradlogli;
	delete [] diag;
	delete [] lambda; 
	delete [] temp_lambda;
    	delete [] ExpF;
    	delete [] ws;
	delete [] iprint; 
	delete  Vi ;
	delete  alpha ;
    	delete  next_alpha ;
    	delete  temp_vector ;
}

int CPosCRF::GetPredictsFromFeatures(void)
{
	//create m_vectPredicate and m_hashPredicate
	cout<<"Collecting predicts from feature......"<<endl;
	vector<FEATURE>::iterator vIter;
	vector<FEATURE>::iterator vIterBegin = m_vectFeatures.begin();
	vector<FEATURE>::iterator vIterEnd = m_vectFeatures.end();

	int index = 0;
	
	for(vIter = vIterBegin;vIter!=vIterEnd;vIter++)
	{	
		if(vIter->predData != "") //for all edge features in CRF, the predData is empty, I don't need build Predicts from edge features,all edge features has been stored in m_vectEdgeFeatures
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
		}
		index++; 
	}	

	cout<<"the size of Predicates is "<<m_vectPredicates.size()<<endl;
	cout<<"Collecting predicts is ok"<<endl;

	return 0;
}

int CPosCRF::BigramTagToID(string tag)
{
	if(m_mapBigramPosID.find(tag) == m_mapBigramPosID.end( ))
	{
		return -1;
	}
	else
	{
		return m_mapBigramPosID[tag];
	}
}
string CPosCRF::IDToBigramTag(int ID)
{
	if(m_mapIDBigramPos.find(ID) == m_mapIDBigramPos.end( ))
	{
		return "";
	}
	else
	{
		return m_mapIDBigramPos[ID];
	}
}

int CPosCRF::FilterFeatures()
{
	cout << "Filtering Features ....." << endl;

	vector<FEATURE> mFilter;  
	vector<int> mFeatureCount;

	size_t mSize;
	mSize = m_vectFeatures.size();
	cout<<"before filtering, features count is "<<mSize<<endl;
	for ( int k = 0; k < mSize; k ++)
	{
		//don't filter edge features
		if (m_feature_count[k]> g_threshold || (m_vectFeatures[k].predType == Edge_Feature_1 ||  m_vectFeatures[k].predType == Edge_Feature_2))
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

	//m_model_expectation.resize(m_vectFeatures.size());
	//fill(m_model_expectation.begin(),m_model_expectation.end(),0.0f);
	

	//m_feature_count_log.resize(m_vectFeatures.size());
	//for(int i = 0;i<m_feature_count_log.size();i++)
	//{
	//	m_feature_count_log[i] = log(m_feature_count[i]);
	//}
	//m_feature_count and m_feature_count_log will be used in GIS and LBFGS. We only calculate them once and store in these two class variables. 
	cout<<"filtering features is ok"<<endl;
	return m_vectFeatures.size();

}

int CPosCRF::SaveModel(int AFileIndex)
{
	fstream f1;
	string mModel_FeatureName = File_FileName_AddIndex(m_fileNameModel, AFileIndex);
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

	return 0;


}

int CPosCRF::LoadModel(int AFileIndex)
{
	string mModel_FeatureName = File_FileName_AddIndex(m_fileNameModel, AFileIndex);


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

	m_vectEdgeFeatures.clear();
	for (int k = 0; k < m_vectFeatures.size(); k ++)
	{
		if(m_vectFeatures[k].predType == Edge_Feature_1 || m_vectFeatures[k].predType == Edge_Feature_2)
		{
			m_vectEdgeFeatures.push_back(k);
		}
	}

	

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
	
	return 0;

	
}

int CPosCRF::LoadDict(void)
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

///////////////////////////////////////////////////////////////////////////////////////////
//below is tagging part
int CPosCRF::UpdateTagMap(string fileNameOption)
{
	fstream fPosOptionsFile;
	fPosOptionsFile.open(fileNameOption.c_str(),ios::in);
	VERIFY_FILE(fPosOptionsFile);
	m_numTag--;
	do 
	{
		char line[10000];
		fPosOptionsFile.getline(line,10000);
		string sline;
		sline.assign(line);
		
		if(sline == ""||sline == "\n"){ continue; }

		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");
		string word = vectline[0];
		// replace the start tag in the end;
		if(word.substr(0,3) != SEN_SPLIT )
		{
			for(int i = 1;i<vectline.size();i++)
			{	
				if(TagToID(vectline[i]) < 0)
				{		
					// if there is only one choice which is unknown tag, I can't tag it correctly. so I need add this tag to the map too.
					pair<string,int> pairPosID;
					pair<int,string> pairPosTag;

					pairPosID.first = vectline[i];
					pairPosID.second = m_numTag;

					pairPosTag.first = m_numTag++;
					pairPosTag.second = vectline[i];

					m_mapPosID.insert(pairPosID);
					m_mapPosTag.insert(pairPosTag);
				}	
			}
		}
	}while(!fPosOptionsFile.eof());

	//add the last START_TAG to the list;
	pair<string,int> pairPosID;
	pair<int,string> pairPosTag;	
	pairPosID.first = START_TAG;
	pairPosID.second = m_numTag;

	pairPosTag.first = m_numTag++;
	pairPosTag.second = START_TAG;

	m_mapPosID.insert(pairPosID);
	m_mapPosTag.insert(pairPosTag);
	
	return m_mapPosTag.size();

	fPosOptionsFile.close();
}

int CPosCRF::TagFile(string strFileName, int AFileIndex)
{
	cout << " Begining Test......" << endl;
	string mPosFileName;
	if(g_Bigram)
	{
		mPosFileName = File_FileName_AddSuffix(strFileName, "_CRF_Bi"); 
	}
	else
	{
		mPosFileName = File_FileName_AddSuffix(strFileName, "_CRF_Uni"); 
	}
	string mstrNewFileNameWithIndex = File_FileName_AddIndex(mPosFileName, AFileIndex); 
	string fileNameOption = g_testingFileName+"_options";

	fstream fPosFile,fPosResultFile,fPosOptionsFile;
	fPosFile.open(strFileName.c_str(),ios::in);
	fPosResultFile.open(mstrNewFileNameWithIndex.c_str(),ios::out);
	fPosOptionsFile.open(fileNameOption.c_str(),ios::in);
	
	VERIFY_FILE(fPosOptionsFile);
	VERIFY_FILE(fPosFile);
	VERIFY_FILE(fPosResultFile);

	if(g_syn_flag == 0)
	{
		//if training and testing are performed at the same time. 
		//these data has been produced by training process, don't need to load them again.
		m_numTag = LoadPosTag("model/"+APPLICATION+"/CRF_TagIDMap");
		cout<< "number of Tag is "<<m_numTag<<endl;		
		//if(!g_flag_pos_options)
		//	LoadDict();
		LoadTemplate(g_templateFileName);
		LoadModel(0);
		GetPredictsFromFeatures();
	}


	if(g_flag_pos_options==2)
	{	
		m_trainNumTag = m_numTag;
		m_numTag = UpdateTagMap(fileNameOption);	
	}

	if(g_Bigram == true)
	{
		m_numBigramTag = m_numTag*m_numTag;
	//	GetBigramPosList();
	}

	if(g_Bigram){
		//only calculated once, maybe a little low effective, but I don't need improve it by now.
		Compute_Mi_Bigram();
	}
	else{
		Compute_Mi_Unigram();
	}
	
	//conduct Inititiate task;

	InitTagging();

	string strPosLine;
	int line = 0;
	char ch_num[20];

	string sSentence = "";
	string pos_options_line;
	int lineNum = 0;
	vector< EVENT > vectEvent;
	cout<<"Collect Events .........."<<endl;

	SAMPLE* wnd[WND_SIZE];
	for (int k = 0; k < WND_SIZE; k ++) {
		wnd[k] = 0;
	}

	int mNewEventCount = 0;   	
	SAMPLE mSample;
	m_vectSamples.clear();
	m_vectSamples.push_back(m_BeginSample);
	m_vectSamples.push_back(m_BeginSample);
	
	int sumTrainLine = 0;
	
	

	do 
	{
		sumTrainLine ++;
		if(sumTrainLine % 1000 == 0) { cout << "."; cout.flush();}
		if(sumTrainLine % 10000 == 0){ cout << sumTrainLine; cout.flush(); }

		char line[10000];
		fPosFile.getline(line,10000);
		string sline;
		sline.assign(line);

		if(g_flag_pos_options)
		{	
			fPosOptionsFile.getline(line,10000);
			pos_options_line.assign(line);
		}

		if(sline == ""||sline == "\n"){ continue; }

		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");
		
		mSample.word = vectline[0];
		/*if(mSample.word == "W@#AD20030408-48-2-7W@#")
		{
			g_debug = 1;
		}
		else
		{
			g_debug = 0;
		}*/
		
		if((mSample.word.substr(0,3) == SEN_SPLIT )&& m_vectSamples.size()>=2)
		{
			m_vectSamples.push_back(m_EndSample);
			m_vectSamples.push_back(m_EndSample);
			vectEvent.clear();
			for(int i = 0;i<m_vectSamples.size()-4;i++)
			{
				wnd[0] = &m_vectSamples[i];
				wnd[1] = &m_vectSamples[i+1];
				wnd[2] = &m_vectSamples[i+2];
				wnd[3] = &m_vectSamples[i+3];
				wnd[4] = &m_vectSamples[i+4];		
				{
					ExtractEventFromWnd(wnd,vectEvent,1);
				}
			}
			fPosResultFile<<TagSentence(vectEvent);
			fPosResultFile<<mSample.word<<endl;
			m_vectSamples.clear();
			m_vectSamples.push_back(m_BeginSample);
			m_vectSamples.push_back(m_BeginSample);
			map_pos_options.clear();
			map_pos_gold.clear();
			lineNum = 0;
			//break;
		}
		else
		{
			//if(vectline.size()>2)
			//{
			//	mSample.middle = vectline[vectline.size()-2];
			//}
			mSample.tag = vectline[vectline.size()-2];
			m_vectSamples.push_back(mSample);
			//got options from the optionfile
			if(g_flag_pos_options)
			{
				String_SeperateToList_WithTrim(pos_options_line,vectline," ");
				{
					string word_opt = vectline[0];
					if(word_opt != mSample.word)
					{
						cout<<"error in pos_options"<<endl;
						exit(2);
					}
					vector<int> p_o;
					for(int i = 1;i<vectline.size();i++)
					{	
						if(TagToID(vectline[i])>=0)
						{
							p_o.push_back(TagToID(vectline[i]));	
						}
						else
						{
							// if there is only one choice which is unknown tag, I can't tag it correctly. so I need add this tag to the map too.
							pair<string,int> pairPosID;
							pair<int,string> pairPosTag;

							pairPosID.first = vectline[i];
							pairPosID.second = m_numTag;

							pairPosTag.first = m_numTag++;
							pairPosTag.second = vectline[i];

							m_mapPosID.insert(pairPosID);
							m_mapPosTag.insert(pairPosTag);
							p_o.push_back(TagToID(vectline[i]));
						}
					}
					pair<int, vector<int> > Pair;
					Pair.first = lineNum;
					Pair.second = p_o;
					map_pos_options.insert(Pair);
		
					pair<int, int > Pair_gold;
					Pair_gold.first = lineNum;
					Pair_gold.second = TagToID(mSample.tag);
					map_pos_gold.insert(Pair_gold);
					lineNum++;
				}
			}
		}
		
	}while(!fPosFile.eof());

	if(fPosFile.eof() && m_vectSamples.size()>2)
	{
		m_vectSamples.push_back(m_EndSample);
		m_vectSamples.push_back(m_EndSample);
		vectEvent.clear();
		for(int i = 0;i<m_vectSamples.size()-4;i++)
		{
			wnd[0] = &m_vectSamples[i];
			wnd[1] = &m_vectSamples[i+1];
			wnd[2] = &m_vectSamples[i+2];
			wnd[3] = &m_vectSamples[i+3];
			wnd[4] = &m_vectSamples[i+4];		
			{
				ExtractEventFromWnd(wnd,vectEvent,1);
			}
		}
		fPosResultFile<<TagSentence(vectEvent);
	}
	
	
	
	fPosFile.close();
	fPosResultFile.close();
	if(g_flag_pos_options)
	{
		fPosOptionsFile.close();	
	}
	
	delete Vi_Tag;

	
	return line;
}



void CPosCRF::Compute_Mi_Unigram() {

	for(int i  = 0;i<m_vectEdgeFeatures.size();i++)
	{
		int index = m_vectEdgeFeatures[i];
		FEATURE & ftemp = m_vectFeatures[index];
		if(ftemp.predType == Edge_Feature_1)
		{
			SparseMatrix.AddToMap(ftemp.previousTag,ftemp.outTag,0.0);
			SparseMatrix.Add(ftemp.previousTag,ftemp.outTag,ftemp.alpha);
		}
	}

	//SparseMatrix.AllExp(false);
	SparseMatrix.AllExp(true);
}

int CPosCRF::ChangeBigramID(int oldID, int trainNum){
	int y = oldID%trainNum  ;
	int x = (oldID-y)/trainNum;
	int newID;
	if(x != trainNum -1)
	{
		newID = x*m_numTag+y;
	}
	else
	{
		//to deal with the last START_TAG, I always add it to the end of the list;
		newID = (m_numTag-1)*m_numTag+y;
	}
	return newID;
}
void CPosCRF::Compute_Mi_Bigram() {
	
	//*Mi = 0;

	for(int i  = 0;i<m_vectEdgeFeatures.size();i++)
	{
		int index = m_vectEdgeFeatures[i];
		FEATURE & ftemp = m_vectFeatures[index];

		if(ftemp.predType == Edge_Feature_1)
		{
			int col = ftemp.previousTag*m_numTag +  ftemp.outTag;
			for(int row = 0;row<m_numTag;row++)
			{
				//Mi->get(row,col) += ftemp.alpha;
				//SparseMatrix.Set(row,col,0.0);
				SparseMatrix_Tag.Set(row*m_numTag+ftemp.previousTag,col,0.0);
			}
		}
		if(ftemp.predType == Edge_Feature_2)
		{
			//Mi->get(ftemp.previousTag, ftemp.outTag)+=ftemp.alpha;
			//SparseMatrix.Set(ftemp.previousTag,ftemp.outTag,0.0);
			if(g_flag_pos_options == 2)
			{
				int x=  ChangeBigramID(ftemp.previousTag,m_trainNumTag);
				int y = ChangeBigramID(ftemp.outTag,m_trainNumTag);
				SparseMatrix_Tag.Set(x,y,0.0);
			}
			if(g_flag_pos_options == 0)
			{
				SparseMatrix_Tag.Set(ftemp.previousTag,ftemp.outTag,0.0);
			}
		}
	}
	
	//SparseMatrix.BuildRowColIndex(m_numBigramTag);
	//the code below look like a little werid.
	for(int i  = 0;i<m_vectEdgeFeatures.size();i++)
	{
		int index = m_vectEdgeFeatures[i];
		FEATURE & ftemp = m_vectFeatures[index];

		if(ftemp.predType == Edge_Feature_1)
		{
			int col = ftemp.previousTag*m_numTag +  ftemp.outTag;
			for(int row = 0;row<m_numTag;row++)
			{
				//Mi->get(row,col) += ftemp.alpha;
				//SparseMatrix.Set(row,col,0.0);
				SparseMatrix_Tag.Add(row*m_numTag+ftemp.previousTag,col,ftemp.alpha);
			}
		}
		if(ftemp.predType == Edge_Feature_2)
		{
			//Mi->get(ftemp.previousTag, ftemp.outTag)+=ftemp.alpha;
			//SparseMatrix.Set(ftemp.previousTag,ftemp.outTag,0.0);
			if(g_flag_pos_options == 2)
			{
				int x=  ChangeBigramID(ftemp.previousTag,m_trainNumTag);
				int y = ChangeBigramID(ftemp.outTag,m_trainNumTag);
				SparseMatrix_Tag.Add(x,y,ftemp.alpha);
			}
			if(g_flag_pos_options == 0)
			{
				SparseMatrix_Tag.Add(ftemp.previousTag,ftemp.outTag,ftemp.alpha);
			}
		}
	}

	SparseMatrix_Tag.AllExp(false);
	//SparseMatrix.ValueIndex();

	/*for (int i = 0; i < m_numBigramTag; i++) {
	    for (int j = 0; j < m_numBigramTag; j++) {
		if(i%m_numTag ==j/m_numTag)
		{
			//Mi->get(i, j) = exp(Mi->get(i, j));
			if(SparseMatrix.Exist(i,j))
			{
				SparseMatrix.Set(i, j, exp(SparseMatrix.Get(i,j)));
			}
			else
			{
				SparseMatrix.Set(i,j,1.0);
			}
		}
		else
		{
			//Mi->get(i, j) = 0.0;
			if(SparseMatrix.Exist(i,j))
			{
				SparseMatrix.Set(i, j, 0);
			}
			SparseMatrix.other = 0.0;
		}
	    }
	}

	SparseMatrix.ValueIndex();
	*/
}

void CPosCRF::ViterbiBigram(vector<EVENT>& vectEvent)
{
	int sen_len = vectEvent.size(); //sentence length;
	if(sen_len == 0)
	{
		return ;
	}
	
	int memorysize = memory.size();
	
	if(memorysize<sen_len)
	{
		for(int i = 0;i<sen_len - memorysize; i++)
		{
			memory.push_back(tempMem);
		}
	}

	int scalesize = scale.size();
	
	if(scalesize< sen_len)
	{
		for(int i = 0;i< sen_len-scalesize;i++)
		{
			scale.push_back(1.0);
		}
	}

	vector<int> v_0;
	vector<int> v_1;
	vector<int> vectPre_1Pos = map_pos_options[0];
	vector<int> vectPre_2Pos;
	vectPre_2Pos.clear();

	if(g_flag_pos_options==2)
	{		
		for(int index = 0; index < vectPre_1Pos.size(); index++)
		{
			int j = m_numTag*(m_numTag-1)+vectPre_1Pos[index];
			v_0.push_back(j);
		}
	}
	
	Compute_Vi_Bigram(vectEvent[0],Vi_Tag,1,0,v_0);
	//v_0.clear();
	
	if(g_flag_pos_options==0)
	{
		for(int j = 0;j<m_numBigramTag;j++)
		{
			memory[0][j].first = (*Vi)[j];
			memory[0][j].second = j;
		}
		
		scale[0] = sum(memory[0]);
		divide(memory[0],scale[0]);
	}

	if(g_flag_pos_options==2)
	{		
		for(int index = 0; index < vectPre_1Pos.size(); index++)
		{
			int j = m_numTag*(m_numTag-1)+vectPre_1Pos[index];
			memory[0][j].first = (*Vi_Tag)[j];
			memory[0][j].second = j;
			
		}
		for(int index = 0; index < vectPre_1Pos.size(); index++)
		{
			//int j = vectPre_1Pos[index];
			int j = m_numTag*(m_numTag-1)+vectPre_1Pos[index];
			scale[0] += memory[0][j].first;
			
		}
		for(int index = 0; index < vectPre_1Pos.size(); index++)
		{
			int j = m_numTag*(m_numTag-1)+vectPre_1Pos[index];
			memory[0][j].first /= scale[0];
		}
	}
	
	vector<int> vectCurPos;
	for(int i = 1;i<sen_len;i++)
	{
		

// 		// applying constraints 
// 		int num_cnts = popt->prevfixedintlabels.size();
// 		for (int cc = 0; cc < num_cnts; cc++) {
// 		int col = popt->prevfixedintlabels[cc][0];
// 		for (int row = 0; row < popt->num_labels; row++) {
// 			int in = 0;
// 			for (int count = 1; count < popt->prevfixedintlabels[cc].size(); count++) {
// 			if (row == popt->prevfixedintlabels[cc][count]) {
// 				in = 1;
// 			}
// 			}
// 			if (!in) {
// 			int index = row * popt->num_labels + col;
// 			(*Vi)[index] = 0;
// 			}
// 		}
// 		}
// 	
// 		num_cnts = popt->nextfixedintlabels.size();
// 		for (int cc = 0; cc < num_cnts; cc++) {
// 		int row = popt->nextfixedintlabels[cc][0];
// 		for (int col = 0; col < popt->num_labels; col++) {
// 			int in = 0;
// 			for (int count = 1; count < popt->nextfixedintlabels[cc].size(); count++) {
// 			if (col == popt->nextfixedintlabels[cc][count]) {
// 				in = 1;
// 			}
// 			}
// 			if (!in) {
// 			int index = row * popt->num_labels + col;
// 			(*Vi)[index] = 0;
// 			}
// 		}
// 		}
	
		// for all possible labels at the position "i"
		if(g_flag_pos_options==2)
		{
			if(i>=2)
			{
				vectPre_2Pos = map_pos_options[i-2];
			}
			vectPre_1Pos = map_pos_options[i-1];
			vectCurPos = map_pos_options[i];
			
			/*for (int index_1 = 0; index_1 < vectPre_1Pos.size(); index_1++) 
			{
				int pos_1 = vectPre_1Pos[index_1];
				if(i == 1)
				{
					int bi_1 = (m_numTag-1)*m_numTag+pos_1;	
					v_0.push_back(bi_1);	
				}
				if(i >=2 )
				{
					for(int index_0 = 0; index_0<vectPre_2Pos.size(); index_0++)
					{	
						int pos_0 = vectPre_2Pos[index_0];
						int bi_1 = pos_0*m_numTag + pos_1;				v_0.push_back(bi_1);	
						
					}
				}
			}*/


			for (int index_2 = 0; index_2 < vectCurPos.size(); index_2++) 
			{
				int pos_2 = vectCurPos[index_2];
				for(int index_1 = 0; index_1<vectPre_1Pos.size(); index_1++)
				{	
					int pos_1 = vectPre_1Pos[index_1];
					int bi_2 = pos_1*m_numTag + pos_2;
					v_1.push_back(bi_2);
				}
			}

			Compute_Vi_Bigram(vectEvent[i],Vi_Tag,1,i,v_1);
	
		
			for (int j = 0;  j< v_1.size(); j++) 
			{
				int bi_2 = v_1[j];
				memory[i][bi_2].first = 0.0;
				memory[i][bi_2].second = 0;
					
				for(int m = 0;m<v_0.size();m++)
				{
					int bi_1 = v_0[m];
					double tempval;
					if(SparseMatrix_Tag.Exist(bi_1,bi_2))
					{
						tempval = memory[i-1][bi_1].first * SparseMatrix_Tag.Tag_Get(bi_1,bi_2) * (*Vi_Tag)[bi_2];
					}
					else
					{
						tempval = memory[i-1][bi_1].first *0.0000001 * (*Vi_Tag)[bi_2];
					}
					if (tempval > memory[i][bi_2].first||m == 0) 
					{
						memory[i][bi_2].first = tempval;
						memory[i][bi_2].second = bi_1;
					}	
				}
			}
			
			for(int m = 0;m<v_1.size();m++)
			{
				int bi_2 = v_1[m];	
				scale[i] += memory[i][bi_2].first;
			}
			for(int m = 0;m<v_1.size();m++)
			{
				int bi_2 = v_1[m];	
				memory[i][bi_2].first /=scale[i];	
			}

			v_0=v_1;;
			v_1.clear();
		}

		if(g_flag_pos_options == 0)
		{
			for (int j = 0; j < m_numBigramTag; j++) 
			{
				memory[i][j].first = 0.0;
				memory[i][j].second = 0;
				
				// find the maximal value and its index and store them in memory
				// for later tracing back to find the best path
				for (int k = 0; k < m_numBigramTag; k++) 
				{		
					//double tempval = memory[i-1][k].first * Mi->mtrx[k][j] * (*Vi)[j];
					double tempval = memory[i-1][k].first * SparseMatrix_Tag.Tag_Get(k,j) * (*Vi)[j];
					if (tempval > memory[i][j].first) 
					{
					memory[i][j].first = tempval;
					memory[i][j].second = k;
					}  
				}
				if(i == 10&&j ==10)
				{
					cout<<i<<" "<<j<<" "<<memory[i][j].first<<endl;	    
				}
			}
				
			// scaling for memory at position "i"
			scale[i] = sum(memory[i]);
			divide(memory[i], scale[i]);
		}
	}

 	// viterbi backtrack to find the best path
	int max_idx;
	double max_value;
	for(int m = 0;m<v_0.size();m++)
	{	
		int tag_index = v_0[m];
		if(m ==0)
		{
			max_idx = tag_index;
			max_value = memory[sen_len - 1][tag_index].first;
		}
		else
		{
			if(memory[sen_len - 1][tag_index].first>max_value)
			{
				max_idx = tag_index;
				max_value = memory[sen_len - 1][tag_index].first;
			}
		}	 
	}

 	vectEvent[sen_len - 1].outTag = max_idx;
 	for (int i = sen_len - 2; i >= 0; i--) 
	{
 		vectEvent[i].outTag = memory[i + 1][max_idx].second;
 		max_idx = vectEvent[i].outTag;
 	}
// 	
 	// converting from second-order labels to first-order ones
 	for (int i = 0; i < sen_len; i++) 
	{
 		vectEvent[i].outTag = vectEvent[i].outTag% m_numTag;		
 	}
}


void CPosCRF::ViterbiUnigram(vector<EVENT>& vectEvent)
{
	int sen_len = vectEvent.size(); //sentence length;
	if(sen_len == 0)
	{
		return ;
	}
	
	int memorysize = memory.size();
	
	if(memorysize<sen_len)
	{
		for(int i = 0;i<sen_len - memorysize; i++)
		{
			memory.push_back(tempMem);
		}
	}

	int scalesize = scale.size();
	if(scalesize< sen_len)
	{
		for(int i = 0;i< sen_len-scalesize;i++)
		{
			scale.push_back(1.0);
		}
	}

	
	Compute_Vi_Unigram(vectEvent[0],Vi_Tag,1,0);

	if(g_flag_pos_options==0)
	{
		for(int j = 0;j<m_numTag;j++)
		{
			memory[0][j].first = (*Vi_Tag)[j];
			memory[0][j].second = j;
		}
	
		scale[0] = sum(memory[0]);
		divide(memory[0],scale[0]);
	}

	vector<int> vectPrePos = map_pos_options[0];
	if(g_flag_pos_options==2)
	{		
		for(int index = 0; index < vectPrePos.size(); index++)
		{
			int j = vectPrePos[index];
			memory[0][j].first = (*Vi_Tag)[j];
			memory[0][j].second = j;
		}
		for(int index = 0; index < vectPrePos.size(); index++)
		{
			int j = vectPrePos[index];
			scale[0] += memory[0][j].first;
		}
		for(int index = 0; index < vectPrePos.size(); index++)
		{
			int j = vectPrePos[index];
			memory[0][j].first /= scale[0];
		}
	}
	
	vector<int> vectCurPos;
	
	for(int i = 1;i<sen_len;i++)
	{
		Compute_Vi_Unigram(vectEvent[i],Vi_Tag,1,i);
		
		if(g_flag_pos_options==2)
		{
			vectCurPos = map_pos_options[i];
			int index;
			for (index = 0; index < vectCurPos.size(); index++) 
			{
				int j = vectCurPos[index];
				memory[i][j].first = 0.0;
				memory[i][j].second = 0;
			
				for(int m = 0;m<vectPrePos.size();m++)
				{
					int k = vectPrePos[m];
					double tempval ;
					if(SparseMatrix.Exist(k,j))
					{
						tempval = memory[i-1][k].first * SparseMatrix.Tag_Get(k,j) * (*Vi_Tag)[j];
					}
					else
					{
						tempval = memory[i-1][k].first *0.0000001 * (*Vi_Tag)[j];
					}
					
					if (tempval > memory[i][j].first||m == 0) 
					{
						memory[i][j].first = tempval;
						memory[i][j].second = k;
					}
				}
			}
			vectPrePos = map_pos_options[i];
			//if(i == 10&&j ==2)
			//{
			//	cout<<i<<" "<<j<<" "<<memory[i][j].first<<endl;	    
			//}
			// scaling for memory at position "i"
			for (index = 0; index < vectCurPos.size(); index++) 
			{
				scale[i] += memory[i][vectCurPos[index]].first;
			}
			for (index = 0; index < vectCurPos.size(); index++) 
			{
				memory[i][vectCurPos[index]].first /=scale[i];
			}
		}
			
		
// 		// applying constraints 
// 		int num_cnts = popt->prevfixedintlabels.size();
// 		for (int cc = 0; cc < num_cnts; cc++) {
// 		int col = popt->prevfixedintlabels[cc][0];
// 		for (int row = 0; row < popt->num_labels; row++) {
// 			int in = 0;
// 			for (int count = 1; count < popt->prevfixedintlabels[cc].size(); count++) {
// 			if (row == popt->prevfixedintlabels[cc][count]) {
// 				in = 1;
// 			}
// 			}
// 			if (!in) {
// 			int index = row * popt->num_labels + col;
// 			(*Vi)[index] = 0;
// 			}
// 		}
// 		}
// 	
// 		num_cnts = popt->nextfixedintlabels.size();
// 		for (int cc = 0; cc < num_cnts; cc++) {
// 		int row = popt->nextfixedintlabels[cc][0];
// 		for (int col = 0; col < popt->num_labels; col++) {
// 			int in = 0;
// 			for (int count = 1; count < popt->nextfixedintlabels[cc].size(); count++) {
// 			if (col == popt->nextfixedintlabels[cc][count]) {
// 				in = 1;
// 			}
// 			}
// 			if (!in) {
// 			int index = row * popt->num_labels + col;
// 			(*Vi)[index] = 0;
// 			}
// 		}
// 		}
	
		// for all possible labels at the position "i"
		if(g_flag_pos_options == 0)
		{
			for (int j = 0; j < m_numTag; j++) 
			{
				memory[i][j].first = 0.0;
				memory[i][j].second = 0;
				
				// find the maximal value and its index and store them in memory
				// for later tracing back to find the best path
				//use all options
				for (int k = 0; k < m_numTag; k++) 
				{		
					//double tempval = memory[i-1][k].first * Mi->mtrx[k][j] * (*Vi)[j];
					double tempval = memory[i-1][k].first * SparseMatrix.Tag_Get(k,j) * (*Vi)[j];
					if (tempval > memory[i][j].first) 
					{
						memory[i][j].first = tempval;
						memory[i][j].second = k;
					}  
				}
				if(i == 10&&j ==2)
				{
					cout<<i<<" "<<j<<" "<<memory[i][j].first<<endl;	    
				}
			}		
			// scaling for memory at position "i"
			scale[i] = sum(memory[i]);
			divide(memory[i], scale[i]);
		}
	}

 	// viterbi backtrack to find the best path
	int max_idx;
	if(g_flag_pos_options == 0)
	{
 		max_idx = find_max(memory[sen_len - 1]);
	}
	if(g_flag_pos_options == 2)
	{
		double max_value;
		for (int index = 0; index < vectPrePos.size(); index++) 
		{
			if(index == 0)
			{
				max_idx = vectPrePos[index];
				max_value = memory[sen_len - 1][max_idx].first;
			}
			else
			{
				
				if(memory[sen_len - 1][vectPrePos[index]].first>max_value)
				{
					max_idx = vectPrePos[index];
					max_value = memory[sen_len - 1][max_idx].first;
				}
			}
		}
	}
	vectEvent[sen_len - 1].outTag = max_idx;
 	for (int i = sen_len - 2; i >= 0; i--) 
	{
 		vectEvent[i].outTag = memory[i + 1][max_idx].second;
 		max_idx = vectEvent[i].outTag;
 	}
}


void CPosCRF::Compute_Vi_Bigram(EVENT & event, doublevector* Vi, int is_exp,int pos,vector<int> & options)
{

	*Vi = 0.0;
	for(int i = 0;i<event.vectIndexPredicate.size();i++)
	{			
		PREDICATE & mCurPredicate = m_vectPredicates[event.vectIndexPredicate[i]]; 
 		vector<pair<int,int> >::iterator  s1_Iter;
		if(mCurPredicate.predType ==State_Feature_1)
		{
			s1_Iter = mCurPredicate.indexFeature.begin();
			while(s1_Iter != mCurPredicate.indexFeature.end())
			{	
				//pair<int,int> pValue = *s1_Iter;
				for(int index = 0;index<options.size();index++)
				{
					int j = options[index];
					if(j%m_numTag == s1_Iter->first)
					{
						(*Vi)[j] += m_vectFeatures[s1_Iter->second].alpha;
					}
				}		
				//for(int i = 0;i<m_numTag;i++)
				//{
				//	(*Vi)[i*m_numTag+pValue.first] += m_vectFeatures[pValue.second].alpha;
				//}
				s1_Iter++;
			}
		}
		
		if(mCurPredicate.predType ==State_Feature_2)
		{
			s1_Iter = mCurPredicate.indexFeature.begin();
			while(s1_Iter != mCurPredicate.indexFeature.end())
			{	
				//pair<int,int> pValue = *s1_Iter;
				if(g_flag_pos_options == 2)
				{
					int newID = ChangeBigramID(s1_Iter->first,m_trainNumTag);
					(*Vi)[newID] += m_vectFeatures[s1_Iter->second].alpha;
				}

				if(g_flag_pos_options == 0)
				{
					(*Vi)[s1_Iter->first] += m_vectFeatures[s1_Iter->second].alpha;
				}
				s1_Iter++;
			}
		}
	}

	 // take exponential operator
    if (is_exp) {
	for(int index = 0;index<options.size();index++)
	{
		int j = options[index];
		(*Vi)[j] = exp((*Vi)[j]);
	}
/*	if (pos == 0) {
	    for (int j = 0; j < m_numBigramTag; j++) {
		if (j / m_numTag == (m_numTag-1)) {
		    (*Vi)[j] = exp((*Vi)[j]);
		} else {
		    (*Vi)[j] = 0;
		}
	    }
	
	} else {
	    for (int j = 0; j < m_numBigramTag; j++) {
		    (*Vi)[j] = exp((*Vi)[j]);
	    }	    	
	}*/
    }
}


void CPosCRF::Compute_Vi_Unigram(EVENT & event, doublevector* Vi, int is_exp,int pos)
{
	*Vi_Tag = 0.0;
	for(int i = 0;i<event.vectIndexPredicate.size();i++)
	{			
		PREDICATE & mCurPredicate = m_vectPredicates[event.vectIndexPredicate[i]]; 
 		vector<pair<int,int> >::iterator  s1_Iter;
		if(mCurPredicate.predType ==State_Feature_1)
		{
			s1_Iter = mCurPredicate.indexFeature.begin();
			while(s1_Iter != mCurPredicate.indexFeature.end())
			{	
				(*Vi_Tag)[s1_Iter->first] += m_vectFeatures[s1_Iter->second].alpha;
				s1_Iter++;
			}
		}
	}
	 // take exponential operator
    	if (is_exp)
	{
		for (int j = 0; j < m_numTag; j++) 
		{
		    (*Vi_Tag)[j] = exp((*Vi_Tag)[j]);
	    	}
    	}

	
}

string CPosCRF::OutputResult(vector<EVENT>& vectEvent)
{
	string strResult = "";
	for(int i = 0;i<vectEvent.size();i++)
	{
		strResult += m_vectSamples[i+2].word;
		strResult += " ";
		//if(m_vectSamples[i+2].middle !="")
		//{
		//	strResult += m_vectSamples[i+2].middle;
		//	strResult += " ";
		//}
		if(vectEvent[i].outTag == map_pos_gold[i])
		{
				g_right++;
		}
		strResult += IDToTag(vectEvent[i].outTag);
		strResult += "\n";
		g_sum ++;
	}	
	//strResult += SEN_SPLIT;
	//strResult += "\n";
	//cout << strResult;
	return strResult;
}

string CPosCRF::TagSentence(vector<EVENT>& vectEvent)
{
	if(g_Bigram == true)
	{
		ViterbiBigram(vectEvent);
	}
	else
	{
		ViterbiUnigram(vectEvent);
	}
	return OutputResult(vectEvent);	
}

void CPosCRF::MakeSentenceEvent(fstream & fPosFile,vector<EVENT> &vectEvent)
{
	cout<<"Collect Events .........."<<endl;
	SAMPLE* wnd[WND_SIZE];
	for (int k = 0; k < WND_SIZE; k ++)
	{
		wnd[k] = 0;
	}

	int mNewEventCount = 0;   	
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
		fPosFile.getline(line,1000);
		string sline;
		sline.assign(line);
		if(sline == ""||sline == "\n")
		{
			continue;
		}
		vector<string> vectline;
		String_SeperateToList_WithTrim(sline,vectline," ");
		
		mSample.word = vectline[0];
		
		if((mSample.word.substr(0,3) == SEN_SPLIT )&& m_vectSamples.size()>2)
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
					ExtractEventFromWnd(wnd,vectEvent,1);
				}
			}
			//fPosResultFile<<TagSentence(vectEvent);
			//fPosResultFile<<SEN_SPLIT<<endl;
			break;
		}
		else
		{
			mSample.tag = vectline[vectline.size()-1];
			m_vectSamples.push_back(mSample);
		}
		
	}while(!fPosFile.eof());

	if(fPosFile.eof() && m_vectSamples.size()>2)
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
				ExtractEventFromWnd(wnd,vectEvent,1);
			}
		}
		//fPosResultFile<<TagSentence(vectEvent);
	}
	
	return ;
}

void CPosCRF::InitTagging()
{
	if(g_Bigram)
	{
		tempMem.resize(m_numBigramTag);
		for(int i = 0;i<m_numBigramTag;i++)
		{
			tempMem.push_back(pair<double,int>(0.0,-1));
		}
		//Mi =  new doublematrix(m_numBigramTag,m_numBigramTag);
		Vi_Tag = new doublevector(m_numBigramTag);
	}
	else
	{
		tempMem.resize(m_numTag);
		for(int i = 0;i<m_numTag;i++)
		{
			tempMem[i] = pair<double,int>(0.0,-1);
		}
	//	Mi =  new doublematrix(m_numTag,m_numTag);
		Vi_Tag = new doublevector(m_numTag);
	}


		
	prevfixedintlabels.clear();
	nextfixedintlabels.clear();
	
	vector<int> labels;	
	int len = prevfixedstrlabels.size();
	for (int i = 0; i < len; i++) {
		labels.clear();
		
		int ID = TagToID(prevfixedstrlabels[i][0]);
		if (ID != -1) {
		labels.push_back(ID);
		} else {
		continue;
		}
		
		for (int j = 1; j < prevfixedstrlabels[i].size(); j++) {
		int ID = TagToID(prevfixedstrlabels[i][j]);
		if (ID != -1) {
			labels.push_back(ID);
		}
		}
		
		if (labels.size() <= 1) {
		continue;
		}
		
		prevfixedintlabels.push_back(labels);
	}
	
	len = nextfixedstrlabels.size();
	for (int i = 0; i < len; i++) {
		labels.clear();
		
		int ID = TagToID(nextfixedstrlabels[i][0]);
		if (ID != -1) {
		labels.push_back(ID);
		} else {
		continue;
		}
		
		for (int j = 1; j < nextfixedstrlabels[i].size(); j++) {
		int ID = TagToID(nextfixedstrlabels[i][j]);
		if (ID != -1) {
			labels.push_back(ID);
		}
		}
		
		if (labels.size() <= 1) {
		continue;
		}
		nextfixedintlabels.push_back(labels);
	}
}


void CPosCRF::MakeSentenceOption(fstream & fPosOptionsFile)
{

	if(!g_flag_pos_options)
	{
		return ;
	}
	char chline[10000];
	string pos_options_line;
	fPosOptionsFile.getline(chline,10000);
	pos_options_line.assign(chline);
	
	map_pos_options.clear();
	map_pos_gold.clear();

	int lineNum = 0;

	if(pos_options_line.substr(0,3) == "W@#")
	{
		return ;
	}
	else
	{
		vector<string> vectline;
		String_SeperateToList_WithTrim(pos_options_line,vectline," ");	
		string word_opt = vectline[0];
		vector<int> p_o;
		for(int i = 1;i<vectline.size();i++)
		{	
			if(TagToID(vectline[i])>=0)
			{
				p_o.push_back(TagToID(vectline[i]));	
			}
			else
			{
				pair<string,int> pairPosID;
				pair<int,string> pairPosTag;
				pairPosID.first = vectline[i];
				pairPosID.second = m_numTag;
				pairPosTag.first = m_numTag++;
				pairPosTag.second = vectline[i];
				m_mapPosID.insert(pairPosID);
				m_mapPosTag.insert(pairPosTag);
				p_o.push_back(TagToID(vectline[i]));
			}
		}
		pair<int, vector<int> > Pair;
		Pair.first = lineNum;
		Pair.second = p_o;
		map_pos_options.insert(Pair);
		
		
	}
	lineNum++;
}


// this is used by viterbi search
double CPosCRF::sum(vector<pair<double, int> > & vect) {
    double res = 0.0;
    
    for (int i = 0; i < vect.size(); i++) {
	res += vect[i].first;
    }
    
    // if the sum in (-1, 1), then set it to 1
    if (res < 1 && res > -1) {
	res = 1;
    }
    
    return res;
}

// this is necessary for scaling
double CPosCRF::divide(vector<pair<double, int> > & vect, double val) {
    for (int i = 0; i < vect.size(); i++) {
	vect[i].first /= val;
    }
}

// this is called once in the viterbi search to trace back the best path
int CPosCRF::find_max(vector<pair<double, int> > & vect) {
    int max_idx = 0;
    double max_val = -1.0;
    
    for (int i = 0; i < vect.size(); i++) {
	if (vect[i].first > max_val) {
	    max_val = vect[i].first;
	    max_idx = i;
	}
    }
    
    return max_idx;    
}







