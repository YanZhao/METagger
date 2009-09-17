#ifndef _TRANSITION_H_
#define _TRANSITION_H_


#include <list>
#include "StateNode.h"

using namespace std;

class STATENODE;

class TRANSITION
{
 public:

  TRANSITION(list<STATENODE>::iterator targetState,double transitionProb);

  ~TRANSITION();

  list<STATENODE>::iterator getTargetState()
    {return targetState;}

  double getTransitionProbability()
    {return transitionProbability;}

 private:

  list<STATENODE>::iterator
    targetState;

  double
    transitionProbability;
};

#endif