#include "StateNode.h"

#include <list>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <math.h>
#include <map>
#include "StrHashTrait.h"
#include <ext/hash_set>
#include <ext/hash_map>
#include "global.h"

using namespace std;
using namespace __gnu_cxx;



extern hash_map<string,double,StrHashTrait<string> > g_hashProb;

extern double transitionProbability(vector<string> tagSequence, 
			     vector<string> contextSequence,
			     vector<string> words,
                             int transitionType,int index = 0);

extern double ME_Probability(vector<string> tagSequence, 
			     vector<string> contextSequence,
				int index);


double g_addLogProbabilities(double value1, double value2)
  // add two (-100) log probabilities: corresponds to adding 
  // two normal probabilities
{
	if(g_hmm == 0)
	{
		double 
		a = value1,
		b = value2, 
		c;
		
		if(a<b){
		c = a-log(1+exp(a-b));
		}else{
		c = b-log(1+exp(b-a));
		}
		//c = a+log(1+exp(b-a));
		return(c);
	}

	if(g_hmm == 1)
	{
		double 
		a = value1/double(100),
		b = value2/double(100), 
		c;
		
		if(a<b){
		c = a-log(1+exp(a-b));
		}else{
		c = b-log(1+exp(b-a));
		}
		
		return(100*c);
	}
}



STATENODE::STATENODE(vector<string> history, vector<string> contextHistory,
		     vector<string> observedWords, string tag, string context)
  :
  history(history),
  observedWords(observedWords),
  contextHistory(contextHistory),
  tag(tag),
  r_reach(0),
  context(context),
  probabilityDefined(2),
  probability(2),
  max_probability(0),
  flag(false)
{}

STATENODE::~STATENODE()
{}

void STATENODE::setProbability(double p_probability, int direction)
  // set either forward or backward probability of current state
{
  probability[direction]        = p_probability;
  probabilityDefined[direction] = 1;
}

double STATENODE::createTransition(list<STATENODE>::iterator targetState,
				   int transitionType,int index,string & wl1,string  &w0,string& wr1)
  // create a transition object in the current state for transition from this
  // state to targetState; this includes computing the transition probability
{
  // compute transition probability:

  // get associated tag sequence
  vector<string> tagSequence = history;
  tagSequence.push_back(targetState->getTagName());

  // get associated context sequence
  vector<string> contextSequence = contextHistory;
  contextSequence.push_back(targetState->getContextName());

  // transitionType == 1: ordinary
  //                == 2: only possible transition, assign 0


  	double transitionProb;
	double contextProb = 0;
	if(g_hmm == 1)
	{
		transitionProb = transitionProbability(tagSequence,
					 contextSequence,
					 targetState->getObservedWords(),
					 transitionType,index);
	}


	//method 1;
	if(g_context == 1 && g_hmm == 0)
	{
		double contextProb1 =   transitionProbability(tagSequence,
					 contextSequence,
					 targetState->getObservedWords(),
					 transitionType,index);
					 
		if(contextSequence[1] == "pre")
			contextSequence[1] = "post";
		else if(contextSequence[1] == "post")
			contextSequence[1] = "pre";
		double contextProb2 = transitionProbability(tagSequence,
					 contextSequence,
					 targetState->getObservedWords(),
					 transitionType);
		
		//  if(contextProb1>1340 &&contextProb2>1340)
		  //if(true)
		 // {
		 // 	contextProb = contextProb1/100; 
		//  }
		//  else
		  {	 
		  	contextProb1/=-100;
			contextProb1 = exp(contextProb1);
				
			contextProb2/=-100;
			contextProb2 = exp(contextProb2);
			
			contextProb = contextProb1/(contextProb1+contextProb2);  
			contextProb = -log(contextProb);
  		
  		}
	}

	//method 2
	/*if(g_hmm == 0)
	{
		char buffer[20];
		string key, temp;
		sprintf(buffer, "%d",index);
		key = buffer;
		key +="+";
		if(wl1 =="<DUMMY_WORD>")
			wl1 = START_WORD;
		
		key +=wl1;
		key +="+";
		if(w0 =="<DUMMY_WORD>")
			w0 = END_WORD;
		key +=w0;
		key +="+";
		if(wr1 =="<DUMMY_WORD>")
			wr1 = END_WORD;
		key +=wr1;
			
		for(int zzz = 1;zzz<tagSequence.size();zzz++)
		{
			key +="+";
			if(tagSequence[zzz] =="xxx_sentence_start")
				key += "<DUMMY_CONTEXT>";
			else if(tagSequence[zzz] =="xxx_sentence_end")
				key += "<DUMMY_CONTEXT>";
			else
				key += tagSequence[zzz];
		}
		key +="+";

		if(contextSequence[0] =="<DUMMY_CONTEXT>")
			key += START_TAG;
		else
			key +=contextSequence[0];
		
		key +="+";
		if(contextSequence[1] =="<DUMMY_CONTEXT>")
			key += END_TAG;
		else
			key +=contextSequence[1];
		
		//key +=contextSequence[1]; 
		if(g_hashContextProb.find(key)!=g_hashContextProb.end())  //如果找到
		{
			double prob = g_hashContextProb[key];
			contextProb = -log(prob);
			
		}
		else
		{
			//only occur in the end of sentence. 
			cout<<" I didn't find this string ~!@#$%^&*"<<key<<endl;
			exit(3);
		}
		
	}*/

	if(g_hmm == 0)
	{
		char buffer[20];
		string key, temp;
		sprintf(buffer, "%d",index);
		key = buffer;
		
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
		if(g_hashProb.find(key)!=g_hashProb.end())  //如果找到
		{
			double prob = g_hashProb[key];
			transitionProb = -log(prob);
			transitionProb += contextProb;
		}
		else
		{
			//only occur in the end of sentence. 
			cout<<" I didn't find this string ~!@#$%^&*"<<key<<endl;
			transitionProb	= 0;
			exit(2);
		}
	}

	
  if(transitionProb < 0){
    if(transitionProb < WEIRD_PROB_TOLERANCE){
      cerr << "TAGGER ERROR: weird transitionProb: " << transitionProb << endl;
      print();
    }
    transitionProb = 0;
  }

  // add transition object to state s
  TRANSITION transition(targetState,transitionProb);
  transitions.push_back(transition);

  return(transitionProb);
}

void STATENODE::addToComputation(list<STATENODE>::iterator sourceState,double transitionProb,int direction)
  // add transition probability to forward (or backward) probability computed
  // so far and stored in sourceState; store result in current state
{
  double newProbability = sourceState->getProbability(direction) + transitionProb;
  
  if(probabilityDefined[direction]){
    // add probability to existing value
    probability[direction] = g_addLogProbabilities(probability[direction],newProbability);
  }else{
    // just use new probability
    probability[direction] = newProbability;
  }

  probabilityDefined[direction] = 1;
}

double STATENODE::getForwardBackwardValue()
  // compute fw/bw value as sum of fw and bw value 
  // (sum, since values are log probabilities)
{
  if(r_reachable()) {
    
    if(probabilityDefined[DIRECTION_FORWARD] && 
       probabilityDefined[DIRECTION_BACKWARD]) {
      return(probability[DIRECTION_FORWARD]+probability[DIRECTION_BACKWARD]);
    } else {
    // this should never happen; assuming probabilities are always set through
    // the TAGNODE function "setAllStatesTo()" before this function is called
      cerr << "TAGGER ERROR: FW or BW probability undefined in state:" << endl;
      print();
     // tagger_close();
      exit(1);
    } 
  } else {
    return 1000000; 
  }
}

void STATENODE::print()
  // show forward and backward probabilities
{
  cout << " state:";
  for(vector<string>::iterator vi=history.begin() ; vi!=history.end() ; ++vi){
    cout << " " << *vi;
  }
  cout << " fw: " << probability[DIRECTION_FORWARD]
       << " bw: " << probability[DIRECTION_BACKWARD]  
       << " " << (r_reach ? "" : "not-ending") << endl;
}

void STATENODE::printHistory()
  // show history; for debugging
{
  for(vector<string>::iterator vi=history.begin() ; vi!=history.end() ; ++vi){
    cout << *vi << " ";
  }
}