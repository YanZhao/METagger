#ifndef _STATENODE_H_
#define _STATENODE_H_

#include "transition.h"
#include <vector>
#include <string>
using namespace std;
class TRANSITION;

const double WEIRD_PROB_TOLERANCE   = -1.0e-10;
const int    DIRECTION_FORWARD      = 0;
const int    DIRECTION_BACKWARD     = 1;

class STATENODE
{
 public:

  //STATENODE(vector<string> history,
//	    vector<string> observedWords,string tag);

	STATENODE(vector<string> history,vector<string> contextHistory,
	    vector<string> observedWords,string tag,string context);

  ~STATENODE();

  STATENODE(const STATENODE& obj) 
    {*this = obj;}
  
  double getProbability(int direction) 
    {return probability[direction];}
  
  vector<TRANSITION> getTransitions()
    {return transitions;}
  
  vector<string> getHistory() 
    {return history;}

  vector<string> getContextHistory() 
    {return contextHistory;}
  
  vector<string> getObservedWords()  
    {return observedWords;}

  string getTagName()
    {return tag;}

  string getContextName()
    {return context;}

  void mark_r_reachable()
    { r_reach=1;}

  int r_reachable() 
    { return(r_reach);}
  
  double getForwardBackwardValue();
  void   setProbability(double p_probability, int direction);	 
  void   addToComputation(list<STATENODE>::iterator sourceState,
			double transitionProb,int direction);
  double createTransition(list<STATENODE>::iterator targetState,
			  int transitionType,int index,string & wl1,string  &w0,string& wr1);
  void   print();
  void   printHistory();

 public:
    double max_probability;
    list<STATENODE>::iterator  previousState;
    bool flag;
   
  
 private:
  
  vector<string>
    history,
    observedWords,
    contextHistory;
   

  string
    tag,
    context;
   
 
  vector<int>
    probabilityDefined;
  
  vector<double>
    probability;

  int r_reach;
  
  double
    forwardBackwardProbability;
  
  vector<TRANSITION>
    transitions;
};

#endif
