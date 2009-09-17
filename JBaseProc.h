
#ifndef __J_W_JBaseProc__
#define __J_W_JBaseProc__

#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

string Trim(string AStr);

void String_SeperateToList(string AFMTStr, vector<string>& AResultList, string ASeperator = "\t");
void String_SeperateAppendToList(string AFMTStr, vector<string>& AResultList, string ASeperator);
void String_SeperateToList_WithTrim(string AFMTStr, vector<string>& AResultList, string ASeperator = "\t");

void Zhao_String_SeperateToList_WithTrim(string wstrIn, vector<string>& vecOut, string wstrTag);  

bool File_Exist(string AFileName);  

inline string JIntToStr(int AInteger)
{
	char mBuf[200];
	sprintf(mBuf,"%d", AInteger);
	return mBuf;
}

inline int JStrToInt(string &AValue)
{
	return atoi(AValue.c_str());
}

inline double JStrToDouble(string &AValue)
{
	return atof(AValue.c_str());
}


string File_GetFilePath(string AFullFileName);  

string File_GetFileExt(string AFileName);  

string File_GetFileNameWithoutExt(string AFileName);  

string File_GetFileName(string AFullFileName);  

string Path_AddBackSlash(string APath);  

string File_FileName_AddSuffix(string AFileName,string ASuffix);  

string File_FileName_AddIndex(string AFileName,int AIndex);  
inline bool IsSentenceEnd(string word)
{
	if(word == "."||word == "?"||word == "!")
		return true;
	return false;
	
}


#endif