#ifndef _TAGNODE_H_
#define _TAGNODE_H_


#include <string>
#include <vector>
#include "StateNode.h"
using namespace std;


class TAGNODE
{
 public:
  
  TAGNODE(string tagName,vector<string> allWords,int wordPos,int tagSpan,
	  int tagID);

  ~TAGNODE();

  TAGNODE(const TAGNODE& obj) 
    {*this = obj;}
  
  int operator<(TAGNODE const &other) const
    {return(forwardBackwardProbability < other.forwardBackwardProbability);}
  
  int betterDifferenceThan(TAGNODE const &other)
    {return(differenceWithBest < other.differenceWithBest);}
  
  int getWordPos()
    {return wordPos;}
  
  int getTagSpan() 
    {return tagSpan;}
  
  int getTagID()
    {return tagID;}
  
  string getTagName() 
    {return tagName;}

  void setTagName(string newName)
    {tagName = newName;}

  vector<string> getObservedWords()  
    {return observedWords;}

   void setContextName(string newName)
    {contextName = newName;}

   string getContextName()
    {return contextName;}


  
  double getDifferenceWithBest()
    {return differenceWithBest;}
  
  list<STATENODE> *getAssociatedStates()
    {return &associatedStates;}
  
  void setForwardBackwardProbability(double value)
    {forwardBackwardProbability = value;}
  
  double getForwardBackwardProbability()
    {return(forwardBackwardProbability);}
  
  void markAsDeleted()
    {deleted = 1;}
  
  int markedAsDeleted()
    {return(deleted || !l_reach);}
  
  void markAsBest()
    {best = 1;}
  
  int markedAsBest()
    {return(best);}

  void computeDifferenceWithBest(double best)
    {differenceWithBest = forwardBackwardProbability - best;}
  
  list<STATENODE>::iterator addNewState(list<STATENODE>::iterator sourceState);
  //void addDummyState(vector<string> history);
  void addDummyState(vector<string> history, vector<string> contextHistory);
  void setAllStatesTo(double value, int direction);
  void setAllStatesReach();
  void computeForwardBackwardValue();
  void print();

  void mark_l_reachable()
    {l_reach=1;}
  
  int l_reachable()
    {return l_reach;}

 private:
  
  string 
    tagName,
    contextName;  
  
  int    
    tagID,
    wordPos,
    tagSpan,
    deleted,
    l_reach,
    best;
  
  double
    forwardBackwardProbability,
    differenceWithBest;
  
  vector<string> 
    observedWords;
  
  list<STATENODE>
    associatedStates;     
};

class TAGNODE_PTR_CMP
{
 public: 
  int operator() (TAGNODE *p1, TAGNODE *p2) 
    {return((*p1).betterDifferenceThan(*p2));}
};

#endif