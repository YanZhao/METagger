#include "transition.h"
#include <list>

using namespace std;


TRANSITION::TRANSITION(list<STATENODE>::iterator targetState,double transitionProb)
  :
  targetState(targetState),
  transitionProbability(transitionProb)
{}

TRANSITION::~TRANSITION()
{}
