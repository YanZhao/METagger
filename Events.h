#pragma once

#include "global.h"
#include <vector>
#include <cstdio>
#include <stdio.h>

using namespace std;

class CEventList  
{
public:
	void SetEventOperatorMode(EVENT_OPERATOR_MODE AOperatorMode); 

	bool OpenEventList(EVENT_LIST_OPEN_MODE AOpenMode);  
	bool ReadEvent(EVENT &AEvent);    
	                                 
	bool WriteEvent(EVENT &AEvent);   
	int TotalEventCount();   
	void CloseEventList();

public:
	bool LoadEventToMemory();   
	bool SaveMemoryToFile();    
	void ClearMemory();         

public:
	CEventList(EVENT_OPERATOR_MODE AOperatorMode = OperatorMode_Memory);  
	~CEventList();

private:
	vector<EVENT> m_vectEvents;  
	FILE * fpEvent;  
	FILE * fpEvent_1;

private:
	int m_CurMemoryEventIndex;  
	int m_CurFileEventIndex;    

	int m_EventCount;      

	bool m_Active;         
	EVENT_OPERATOR_MODE m_OperatorMode;  
	EVENT_LIST_OPEN_MODE m_OpenMode;

public: 
	bool LoadEventsFromFile(string AEventFileName, vector<EVENT> &AEventVector);  
	bool SaveEventsToFile(vector<EVENT> &AEventVector, string AEventFileName);  

private:
	string m_EventFileName;
	string m_EventFileName_1;
	bool JReadFileEvent(FILE * AEventFile, EVENT &AEvent);
	bool JWriteFileEvent(FILE * AEventFile, EVENT & AEvent);

public:  
	bool PrintToTxtFile(string ATxtFileName);  
    
};


