#include "TagNode.h"
#include <list>
#include <iostream>

using namespace std;


const string DUMMY_WORD             = "<DUMMY_WORD>";
//const string DUMMY_CONTEXT          = "<DUMMY_CONTEXT>";
const string DUMMY_TAG              = "<DUMMY_TAG>"; // used for unknown words in baseline method

extern double g_addLogProbabilities(double value1, double value2);

TAGNODE::TAGNODE(string tagName,vector<string> sentence,int wordPos,int tagSpan,int tagID)
  :
  tagName(tagName),
  tagID(tagID),
  wordPos(wordPos),
  tagSpan(tagSpan),
  deleted(0), 
  best(0),
  l_reach(0),
  forwardBackwardProbability(0)
{
  // store observedWords for this tag in this tagNode
  for(int i=wordPos ; i<wordPos+tagSpan ; ++i){
    if(i > -1){
      observedWords.push_back(sentence[i]);
    }else{
      observedWords.push_back(DUMMY_WORD);
    }
  }
}

TAGNODE::~TAGNODE()
{}

void TAGNODE::setAllStatesTo(double value, int direction)
  // initialize all states' forward or backward probabilities to value
{
  for(list<STATENODE>::iterator li=associatedStates.begin() ; li!=associatedStates.end() ; ++li){
    li->setProbability(value,direction);
  }
}

void TAGNODE::setAllStatesReach()
{
  for(list<STATENODE>::iterator li=associatedStates.begin() ; li!=associatedStates.end() ; ++li){
    li->mark_r_reachable();
  }
}

list<STATENODE>::iterator TAGNODE::addNewState(list<STATENODE>::iterator sourceState)
  // add a state to the current tagNode, using source state for history data
{
  // tag history:
  vector<string> newHistory(sourceState->getHistory());                 



  // add current tag to end
  newHistory.push_back(tagName);  
  // remove oldest tag
  newHistory.erase(newHistory.begin()); 


vector<string> newContextHistory(sourceState->getContextHistory());                 
  // add current context to end
  newContextHistory.push_back(contextName);  
  // remove oldest tag
  newContextHistory.erase(newContextHistory.begin()); 

  

  for(list<STATENODE>::iterator li=associatedStates.begin() ; li!=associatedStates.end() ; ++li){
    // if state with same histories already exists
    if((newHistory == li->getHistory()) ){
      // return iterator to existing state
      return(li);
    }
  }


  STATENODE state(newHistory,newContextHistory,observedWords,tagName,contextName);
  
  // otherwise, add new state and return iterator
  associatedStates.push_front(state);
  return(associatedStates.begin());

}

/*void TAGNODE::addDummyState(vector<string> history)
  // add a statenode without doing the computations of the addState function
{
  STATENODE newState(history,observedWords,tagName);
  associatedStates.push_back(newState);
}*/

void TAGNODE::addDummyState(vector<string> history,vector<string> contextHistory)
  // add a statenode without doing the computations of the addState function
{
  STATENODE newState(history,contextHistory,observedWords,tagName,contextName);
  associatedStates.push_back(newState);
}


void TAGNODE::computeForwardBackwardValue()
  // Compute and combine fw/bw values of all states belonging to this tagNode.
  // Since regular probabilities would have to be added (thus representing 
  // probability of reaching one of these states) the log probabilities have 
  // to be added using the function addLogProbabilities.
{

  int r_reachable=1;
  for(list<STATENODE>::iterator s=associatedStates.begin() ; s!=associatedStates.end() ; ++s){
    if(s == associatedStates.begin()){
      forwardBackwardProbability = s->getForwardBackwardValue();
    } else {
      forwardBackwardProbability = g_addLogProbabilities(forwardBackwardProbability, s->getForwardBackwardValue());
    }
    r_reachable=(r_reachable && s->r_reachable());
  }
  if (!r_reachable) {
    markAsDeleted();
  }
}

void TAGNODE::print()
{

  string observedWord = *observedWords.begin();
  for(vector<string>::iterator vi=observedWords.begin()+1 ; vi!=observedWords.end() ; ++vi){
    observedWord += (" " + *vi);
  }
  
  cout << observedWord << " " << tagName << " "  
       << wordPos << "-"
       << wordPos+tagSpan << " " << forwardBackwardProbability
       << " " << (deleted ? "deleted" : "")
       << " " << (l_reach ? "" : "not-reachable")  << endl;
  
  for(list<STATENODE>::iterator li=associatedStates.begin() ; li!=associatedStates.end() ; ++li){
    li->print();
  }
}
