#include "Events.h"
#include <iostream>
#include <string>

#include <stdlib.h>
using namespace std;


//const string CEventList::m_EventFileName = "/scratch/EventFile.bin";
CEventList::CEventList(EVENT_OPERATOR_MODE AOperatorMode)
{
	m_Active = false;
	m_EventCount = -1;       
	m_CurMemoryEventIndex = -1;   
	m_CurFileEventIndex = -1;    

	m_EventFileName = "/data/yzhao/model_mwu/"+APPLICATION+"/EventFile.bin";
	m_EventFileName_1 = "/data/yzhao/model_mwu/"+APPLICATION+"/EventFile_1.bin";

	m_OperatorMode = AOperatorMode;
	fpEvent = NULL;
	fpEvent_1 = NULL;
}

CEventList::~CEventList()
{
	if (m_Active == true)
	{
		CloseEventList();
	}
}



bool CEventList::OpenEventList(EVENT_LIST_OPEN_MODE AOpenMode)  
{
	if (m_Active == true)
	{
		CloseEventList();
	}
	m_OpenMode = AOpenMode;
	if (AOpenMode == OpenMode_Read)  
	{
		if (m_OperatorMode == OperatorMode_File) 
		{
			m_CurFileEventIndex = 0;
			if (fpEvent != NULL) fclose(fpEvent);
			fpEvent = fopen(m_EventFileName.c_str(), "rb");
			if (fpEvent == NULL)
			{
				cout << "File does not exist:" << m_EventFileName << endl;
				return false;
			}
			if (fpEvent_1 != NULL) fclose(fpEvent_1);
			fpEvent_1 = fopen(m_EventFileName_1.c_str(), "rb");
			if (fpEvent_1 == NULL)
			{
				cout << "File does not exist:" << m_EventFileName_1 << endl;
				return false;
			}
		}
		else 
		{	
            m_CurMemoryEventIndex = 0;
		}  //
	}
	else   
	{
		m_EventCount = 0;		
		if (m_OperatorMode == OperatorMode_File) 
		{
			m_CurFileEventIndex = 0;
			if (fpEvent != NULL) fclose(fpEvent);
			fpEvent = fopen(m_EventFileName.c_str(), "wb");
			if (fpEvent == NULL)
			{
				cout << "Can not create File:" << m_EventFileName << endl;
				return false;
			}
			if (fpEvent_1 != NULL) fclose(fpEvent_1);
			fpEvent_1 = fopen(m_EventFileName_1.c_str(), "wb");
			if (fpEvent_1 == NULL)
			{
				cout << "Can not create File:" << m_EventFileName_1 << endl;
				return false;
			}
		}
		else 
		{	
            m_CurMemoryEventIndex = 0;
			m_vectEvents.clear();     
		}  
	}
	m_Active = true;
	return true;

}

void CEventList::SetEventOperatorMode(EVENT_OPERATOR_MODE AOperatorMode)  
{         
	if (AOperatorMode == m_OperatorMode) return; 
	m_OperatorMode = AOperatorMode;   
	if (m_Active == false) return;  

	if (fpEvent != NULL)  
	{
		fclose(fpEvent);
		fpEvent = NULL;
	}

 
	if (m_OperatorMode == OperatorMode_Memory)  
	{
		if (m_OpenMode == OpenMode_Read) 
		{
			
			m_vectEvents.clear();
			if (LoadEventsFromFile(m_EventFileName, m_vectEvents) == false)
			{
				cout << "Can not open and read Event File:" << m_EventFileName << endl;
                throw("Event Operator Mode Changed Failure");
			}
			
			m_CurMemoryEventIndex = m_CurFileEventIndex;
		}
		else    
		{
			
			m_vectEvents.clear();
			if (LoadEventsFromFile(m_EventFileName, m_vectEvents) == false)
			{
				cout << "Can not open and read Event File:" << m_EventFileName << endl;
                throw("Event Operator Mode Changed Failure");
			}
		
			m_CurMemoryEventIndex = m_CurFileEventIndex;
		}		        
	}
	else   
	{
		if (m_OpenMode == OpenMode_Read)
		{
			
			if (SaveEventsToFile(m_vectEvents, m_EventFileName) == false)
			{
				cout << "Can not create and write Event File:" << m_EventFileName << endl;
                throw("Event Operator Mode Changed Failure");
			}
			int mEventIndex = m_CurMemoryEventIndex;  
			
			if (mEventIndex < m_vectEvents.size())
			{
				OpenEventList(OpenMode_Read);
				EVENT mT;
				for (int k = 0; k < mEventIndex; k ++)  
				{
					JReadFileEvent(fpEvent, mT);
				}
			}
			
			m_CurFileEventIndex = mEventIndex;        
		}
		else   
		{
			
			int mEventIndex = m_CurMemoryEventIndex;
			
			OpenEventList(OpenMode_Write);
			for (int k= 0; k < m_vectEvents.size(); k ++)
			{
				JWriteFileEvent(fpEvent,m_vectEvents[k]);
			}
		
			m_CurFileEventIndex = mEventIndex;       
		}
	}
}

bool CEventList::ReadEvent(EVENT &AEvent)    
{
	if (m_Active == false)
	{
		throw ("Please Open List First!");
	}
	if (m_OpenMode != OpenMode_Read)
	{
		throw ("Open Mode Is Not Read");
	}

	if (m_OperatorMode == OperatorMode_File) 
	{
		if(m_CurFileEventIndex<g_event_split)
		{
			if (JReadFileEvent(fpEvent, AEvent) == false) {return false;}
			m_CurFileEventIndex ++;   
		}
		else
		{
			if (JReadFileEvent(fpEvent_1, AEvent) == false) {return false;}
			m_CurFileEventIndex ++;   
		}
	}
	else 
	{
		if (m_CurMemoryEventIndex < m_vectEvents.size())
		{
			AEvent = m_vectEvents[m_CurMemoryEventIndex];  
			m_CurMemoryEventIndex ++; 
		}
		else { return false; }
	}
	

	return true;

}
	                            
bool CEventList::WriteEvent(EVENT &AEvent)  
{
	if (m_Active == false)
	{
		throw ("Please Open List First!");
	}
	if (m_OpenMode != OpenMode_Write)
	{
		throw ("Open Mode Is Not Write");
	}
	
	if (m_OperatorMode == OperatorMode_File) 
	{
		if(m_CurFileEventIndex<g_event_split)
		{
        		if (JWriteFileEvent(fpEvent, AEvent) == false) {	return false;	}
				m_CurFileEventIndex ++;  
		}
		else
		{
			if (JWriteFileEvent(fpEvent_1, AEvent) == false) {	return false;	}
				m_CurFileEventIndex ++;   
		}
	}
	else 
	{
		m_vectEvents.push_back(AEvent);
		m_CurMemoryEventIndex ++;  
	}
	
	if (m_EventCount != -1)	m_EventCount ++;
	return true;
}

int CEventList::TotalEventCount()  
{
	if (m_EventCount >= 0) return m_EventCount;   
	if (m_Active == false) 
	{
		return -1;
	}
	if (m_OperatorMode == OperatorMode_Memory)   
	{
		m_EventCount = m_vectEvents.size();
	}
	else  
	{
		
		long mFilePos = ftell(fpEvent);
		int mCurIndex = m_CurFileEventIndex;
	
		EVENT mE;
		while (ReadEvent(mE) == true)
		{
		}
		m_EventCount = m_CurFileEventIndex;  
		fseek(fpEvent, mFilePos, SEEK_SET);
		m_CurFileEventIndex = mCurIndex;
	}

	return m_EventCount;
}

void CEventList::CloseEventList()
{

	m_CurMemoryEventIndex = -1;
	m_CurFileEventIndex = -1;
	if (m_OperatorMode == OperatorMode_File)
	{
		if (fpEvent != NULL)
		{
            		fclose(fpEvent);
			fpEvent = NULL;
		}

		if (fpEvent_1 != NULL)
		{
            		fclose(fpEvent_1);
			fpEvent_1 = NULL;
		}
	}
	else
	{
	}
	m_Active = false;
}

bool CEventList::LoadEventsFromFile(string AEventFileName, vector<EVENT> &AEventVector)  
{
	FILE * fp;
	fp = fopen(AEventFileName.c_str(), "rb");
	if (fp == NULL)
	{
		cout << "Event File does not Exist:" << AEventFileName << endl;
		return false;
	}
	EVENT mT;
    while (JReadFileEvent(fp, mT) == true)
	{
		AEventVector.push_back(mT);
	}
	fclose(fp);
	return true;
}

bool CEventList::SaveEventsToFile(vector<EVENT> &AEventVector, string AEventFileName)  
{
	FILE * fp;
	fp = fopen(AEventFileName.c_str(), "wb");
	if (fp == NULL)
	{
		cout << "Can not Create Event File:" << AEventFileName << endl;
		return false;
	}
	for (int k = 0 ; k < AEventVector.size(); k ++)
	{
		JWriteFileEvent(fp, AEventVector[k]);
	}
	fclose(fp);
	return true;
}



bool CEventList::PrintToTxtFile(string ATxtFileName)  
{
	if (m_OperatorMode == OperatorMode_File) 
	{
		if (m_Active == true)
		{
			cout << "Event File Mode is actived, Can not print to text file" << endl;
			return false;
		}
        
	}
	
	FILE * fp;
	fp = fopen(ATxtFileName.c_str(), "w");
	if (fp == NULL)
	{
		cout << "cannot create Events out file : " << ATxtFileName << endl;
		return false;
	}

	/////////////////
	if (m_OperatorMode == OperatorMode_Memory)
	{
		for (unsigned int k = 0 ; k < m_vectEvents.size(); k ++)
		{
			EVENT & mT = m_vectEvents[k];
			fprintf(fp, "%d\t%d\n", mT.outTag, mT.count);
			unsigned int t;
			for (t = 0 ; t < mT.vectIndexPredicate.size(); t ++)
			{
				fprintf(fp, "\t%d", mT.vectIndexPredicate[t]);
			}
			fprintf(fp, "\n");
		}
	}
	else
	{
		FILE * mFile;
		mFile = fopen(m_EventFileName.c_str(), "rb");
		EVENT mT;
		while (JReadFileEvent(mFile, mT) == true)
		{
			fprintf(fp, "%d\t%d\n", mT.outTag, mT.count);
			unsigned int t;
			for (t = 0 ; t < mT.vectIndexPredicate.size(); t ++)
			{
				fprintf(fp, "\t%d", mT.vectIndexPredicate[t]);
			}
			fprintf(fp, "\n");
		}
		fclose(mFile);
	}
	/////////////////
	fclose(fp);

	return true;
}

bool CEventList::LoadEventToMemory()  
{
	return LoadEventsFromFile(m_EventFileName, m_vectEvents);
}

bool CEventList::SaveMemoryToFile()    
{
	return SaveEventsToFile( m_vectEvents, m_EventFileName);
}

void CEventList::ClearMemory()        
{
	m_vectEvents.clear();
}

bool CEventList::JReadFileEvent(FILE * AEventFile, EVENT &AEvent)
{
	if (fread(&AEvent.outTag, sizeof(AEvent.outTag), 1, AEventFile) <= 0) return false;
	fread(&AEvent.count, sizeof(AEvent.count),1, AEventFile);

	int mPredicateLen;
	fread(&mPredicateLen, sizeof(mPredicateLen), 1, AEventFile);

	int mIndex;
	AEvent.vectIndexPredicate.clear();
	for (int k = 0; k < mPredicateLen; k ++)
	{
		fread(&mIndex, sizeof(mIndex), 1, AEventFile);
		AEvent.vectIndexPredicate.push_back(mIndex);
	}
	return true;
}

bool CEventList::JWriteFileEvent(FILE * AEventFile, EVENT & AEvent)
{
	fwrite(&AEvent.outTag, sizeof(AEvent.outTag), 1, AEventFile);
	fwrite(&AEvent.count, sizeof(AEvent.count),1, AEventFile);

	int mPredicateLen = AEvent.vectIndexPredicate.size();
	fwrite(&mPredicateLen, sizeof(mPredicateLen), 1, AEventFile);

	for (int k = 0; k < mPredicateLen; k ++)
	{
		fwrite(&AEvent.vectIndexPredicate[k], sizeof(AEvent.vectIndexPredicate[k]), 1 , AEventFile);
	}
	return true;
}

