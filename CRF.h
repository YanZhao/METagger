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

#pragma once
#include "LogLinear.h"
#include "global.h"
#include <string>
#include "Events.h"
#include "doublevector.h"
#include "doublematrix.h"
#include "mathlib.h"
#include "SparseMatrix.h"

using namespace std;
#ifdef linux
using namespace __gnu_cxx;
#endif 

typedef enum CRF_PredType
{
	State_Feature_1 = 1,
	State_Feature_2 = 2,
	Edge_Feature_1 = 3,
	Edge_Feature_2 = 4,
}CRF_PRED_TYPE;

class CPosCRF: public CLogLinear
{
public:
	
	CPosCRF(void);
	~CPosCRF(void);
	int Train(string& fileName);
	int TagFile(string strFileName, int AFileIndex = 0);

protected:
	void MakeSentenceOption(fstream & fPosOptionsFile);
	string TagSentence(vector<EVENT>& vectEvent);
	void MakeSentenceEvent(fstream & fPosFile,vector<EVENT> &vectEvent);
	int SaveModel(int AFileIndex);
	void Compute_Mi_Bigram();
	int GetBigramPosList();
	int ReadSamplesAndCollectFeature(string& fileName);
	void ExtractFeatureFromWnd(SAMPLE * wnd[]);
	int GetPredictsFromFeatures(void);
	template<class T>
	bool GetFeatureFromTemplate(T &ft, SAMPLE * wnd[], string & templateStr);
	virtual void LookupAndUpdateFeature(FEATURE &AFeature);
	int CollectEvents(string& fileName);
	void ExtractEventFromWnd(SAMPLE * wnd[],vector<EVENT> &vectEvent ,bool flag =0);
	int IndexOfPredicate(PREDICATE &APre_Type);  
	void LBFGS_Estimate(void);
	void InitTrain();
	double Likelihood_Gradient_Whole(double * lambda, double * gradlogli,int num_iters);
	void Get_Mi_Vi_Bigram(EVENT & event, doublevector* Vi, int is_exp);
	void Get_Mi_Vi_Unigram(EVENT & event, doublevector* Vi, int is_exp);
	void Likelihood_Gradient_Bigram_Event(EVENT & event,EVENT & pre_event, int position,double& seq_logli);
	void Likelihood_Gradient_Unigram_Event(EVENT & event, EVENT & pre_event, int position,double& seq_logli );

	void SaveFeaturesToTxt(string AFileName);
	int FilterFeatures();
	double Likelihood_Gradient_Bigram_Sentence(vector<EVENT>& vectEvent);
	double Likelihood_Gradient_Unigram_Sentence(vector<EVENT>& vectEvent,int ni);
	void ViterbiBigram(vector<EVENT>& vectEvent);
	void Compute_Vi_Bigram(EVENT & event,  doublevector* Vi, int is_exp,int pos,vector<int> & options);
	int ChangeBigramID(int oldID, int trainNum);

	int BigramTagToID(string tag);
	void GetEdgeFeature();
	string IDToBigramTag(int ID);
	double norm(int len, double * vect);
	void Get_Mi_Unigram();
	void Get_Mi_Bigram();

	int LoadModel(int AFileIndex);
	string OutputResult(vector<EVENT>& vectEvent);
	
	int LoadDict(void);
	void InitTagging();
	void ViterbiUnigram(vector<EVENT>& vectEvent);
	void Compute_Mi_Unigram();
	void Compute_Vi_Unigram(EVENT & event, doublevector* Vi, int is_exp,int pos);

	static double sum(vector<pair<double, int> > & vect);
    	
    	static double divide(vector<pair<double, int> > & vect, double val);
    	
    	static int find_max(vector<pair<double, int> > & vect);
	int UpdateTagMap(string fileNameOption);
    
private:
	vector<int> m_vectEdgeFeatures;
	vector<int> Edge_1_Index;
	CEventList m_EventList; 
	int m_numTag;
	int m_numBigramTag;
	int m_numFeatures;
	string m_fileNameModel;	
	map<int, vector<int> > map_pos_options;

	map<string, int> m_mapBigramPosID;
	map<int, string> m_mapIDBigramPos;
	CSparseMatrix SparseMatrix;
	CSparseMatrix SparseMatrix_Tag;

	double * lambda;
    	double * temp_lambda;
    	int is_logging;
	int m_trainNumTag;
    
    	double * gradlogli;	// log-likelihood vector gradient
    	double * diag;	// for optimization (used by L-BFGS)
    
    	doublematrix * Mi;	// for edge features (a small modification from published papers)
    	doublevector * Vi;	// for state features
	doublevector * Vi_Edge1; // for bigram edge1 feature
	doublevector * Vi_Tag;
    	doublevector * alpha, * next_alpha;	// forward variable
    	vector<doublevector *> betas;	// backward variables
    	doublevector * temp_vector;	// temporary vector used during computing
	vector<double> scale, rlogscale;


	vector<pair<double, int> > tempMem;
    	vector<vector<pair<double, int> > > memory;	// storing viterbi information

    
    	double * ExpF;	// feature expectation (according to the model)
    	double * ws;	// memory workspace used by L-BFGS
	int * iprint;
	
	// constraints for Viterbi (a particular label follows particular labels)
   	vector<vector<string> > prevfixedstrlabels;
    	vector<vector<int> >  prevfixedintlabels;

    	// constraints for Viterbi (a particular label is followed by particular labels)
    	vector<vector<string> > nextfixedstrlabels;
    	vector<vector<int> >  nextfixedintlabels;

};