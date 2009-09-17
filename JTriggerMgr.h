

#ifndef _JTRIGGERMGR__UNIT_
#define _JTRIGGERMGR__UNIT_
#include "global.h"


#include <map>
#include <math.h>
#include <algorithm>
#include "JBaseProc.h"
#include <iostream>
#include "PosMaxEn.h"

using namespace std;

typedef struct tag_Trigger_Text_Feature  
{
	string B;               
	int outTag;              
	vector<string> vect_A;   
	double alpha;           
	bool operator==(const tag_Trigger_Text_Feature &right) const;
    bool operator!=(const tag_Trigger_Text_Feature &right) const;
    bool operator<(const tag_Trigger_Text_Feature &right) const;
    bool operator>(const tag_Trigger_Text_Feature &right) const;
    bool operator<=(const tag_Trigger_Text_Feature &right) const;
    bool operator>=(const tag_Trigger_Text_Feature &right) const;
}TRIGGER_TEXT_FEATURE;

class CJTriggerTxtFeature
{
public:
	 CJTriggerTxtFeature(string ATxtFileName); 
	 ~CJTriggerTxtFeature();

public:
	void clear();  

	//±£´æÌØÕ÷
	void AddTriggerFeature(FEATURE &AFeature);
	bool SaveToFile();     
	
	bool LoadFromFile();   
	void CopyTriggerFeature(POS_PRED_TYPE APredType, vector<FEATURE> & AToFeatureVector);  

	void SetTxtFileName(string ATxtFileName);

private:
	string m_TxtFileName;   
	vector<TRIGGER_TEXT_FEATURE> m_vectTxtFeature;

private:
	bool JLoadFromFile(string AFileName);
	bool JSaveToFile(string AFileName);
	void SortTxtFeature();    
	void MergerTxtFeature();  
	void JChangeFeatureToTriggerFeature(FEATURE &AFeature, TRIGGER_TEXT_FEATURE & AResultFeature);

};

inline bool tag_Trigger_Text_Feature::operator==(const tag_Trigger_Text_Feature &right) const
{
	if ((B == right.B) && (outTag == right.outTag) && (alpha == right.alpha))  
		return true;                                                      
	else
		return false;
}

inline bool tag_Trigger_Text_Feature::operator!=(const tag_Trigger_Text_Feature &right) const
{
	if (!((*this) == right))
		return true;
	else
		return false;
}


inline bool tag_Trigger_Text_Feature::operator<(const tag_Trigger_Text_Feature &right) const
{
	if (B < right.B) return true;
	if (B > right.B) return false;

	if (outTag < right.outTag) return true;
	if (outTag > right.outTag) return false;

	if (alpha < right.alpha) return true;

	return false;
}

inline bool tag_Trigger_Text_Feature::operator>(const tag_Trigger_Text_Feature &right) const
{
	if (B > right.B) return true;
	if (B < right.B) return false;

	if (outTag > right.outTag) return true;
	if (outTag < right.outTag) return false;

	if (alpha > right.alpha) return true;

	return false;
}

inline bool tag_Trigger_Text_Feature::operator<=(const tag_Trigger_Text_Feature &right) const
{
	const tag_Trigger_Text_Feature & mLeft = *this;
	if ((mLeft < right) || (mLeft == right)) return true;
	return false;
}

inline bool tag_Trigger_Text_Feature::operator>=(const tag_Trigger_Text_Feature &right) const
{
	const tag_Trigger_Text_Feature & mLeft = *this;
	if ((mLeft > right) || (mLeft == right)) return true;
	return false;
}

////////////////////////////////////////
#endif

