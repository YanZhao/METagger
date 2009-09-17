
//#include <stdafx.h>
#include "JBaseProc.h"
#include <stdexcept>


void String_SeperateToList(string AFMTStr, vector<string>& AResultList, string ASeperator)
{
  int mPos;
  size_t mLen;
       

  AResultList.clear();
  if (ASeperator == "")
  {
	  AResultList.push_back(AFMTStr);
	  return ;
  }

  mLen = ASeperator.length();
  mPos = AFMTStr.find(ASeperator); 
  while (mPos >= 0)
  {
	  AResultList.push_back(AFMTStr.substr(0, mPos));
	  AFMTStr.replace(0, mPos + mLen, "");
	  mPos = AFMTStr.find(ASeperator);
  }
  AResultList.push_back(AFMTStr);
}

void String_SeperateAppendToList(string AFMTStr, vector<string>& AResultList, string ASeperator)
{
  int mPos;
  size_t mLen;
         

  //AResultList.clear();
  if (ASeperator == "")
  {
	  AResultList.push_back(AFMTStr);
	  return ;
  }

  mLen = ASeperator.length();
  mPos = AFMTStr.find(ASeperator); 
  while (mPos >= 0)
  {
	  AResultList.push_back(AFMTStr.substr(0, mPos));
	  AFMTStr.replace(0, mPos + mLen, "");
	  mPos = AFMTStr.find(ASeperator);
  }
  AResultList.push_back(AFMTStr);
}

void String_SeperateToList_WithTrim(string AFMTStr, vector<string>& AResultList, string ASeperator)
{
	int mPos;
	size_t mLen;
			
	AResultList.clear();

	if (ASeperator == "")
	{
		AResultList.push_back(AFMTStr);
		return ;
	}

	mLen = ASeperator.length();

	AFMTStr = Trim(AFMTStr);
	mPos = AFMTStr.find(ASeperator); 
	string mT;
	while (mPos >= 0)
	{
		mT = Trim(AFMTStr.substr(0, mPos));
		if (mT != "")  AResultList.push_back(mT);

		AFMTStr.replace(0, mPos + mLen, "");
		AFMTStr = Trim(AFMTStr);
		if (AFMTStr == "") {break;}
		mPos = AFMTStr.find(ASeperator);
	}
	if (AFMTStr != "")
		AResultList.push_back(AFMTStr);
}

void Zhao_String_SeperateToList_WithTrim(string wstrIn, vector<string>& vecOut, string wstrTag)
{
	if(wstrIn.length() ==0)
		return ;
	size_t strLength = wstrIn.length();
	if(0x20 != wstrIn[strLength-1]|| 0x20 != wstrIn[strLength-2])
	{
		throw runtime_error("the end of line has not two spaces");
	}
	static const basic_string <wchar_t>::size_type npos = -1;
	basic_string <wchar_t>::size_type indexBeg, indexEnd;
	indexBeg = 0;
	string wstrTemp;
	int nSum = 0;
	while(true)
	{
		indexEnd = wstrIn.find_first_of(wstrTag,indexBeg);
		if(indexEnd == npos)
		{
			break;
		}

		if(0x20 != wstrIn[indexEnd]|| 0x20 != wstrIn[indexEnd+1])
		{
			throw runtime_error("the line has s single space, need you to check it");
		}

		wstrTemp = wstrIn.substr(indexBeg,(indexEnd-indexBeg));		
		indexBeg = indexEnd+wstrTag.length();
		vecOut.push_back(wstrTemp);
		nSum++;
	}
}

string Trim(string AStr)
{    
	size_t mLen = AStr.length();
	if (mLen == 0) return "";
	unsigned int mbegin, mend ;

	char mch;
	for (mbegin = 0 ; mbegin < mLen ; mbegin ++)  //
	{
		mch = AStr[mbegin];
		if ((mch != ' ') && (mch != '\n') 
		   && (mch != '\r') && (mch != '\t') && (mch != '\v')) break;
	}
    if (mbegin == mLen) return "";  

	for (mend = mLen - 1 ; mend >= 0 ; mend --)  //
	{
		mch = AStr[mend];
		if ((mch != ' ') && (mch != '\n') 
		   && (mch != '\r') && (mch != '\t') && (mch != '\v')) break;
	}

	if (mend < 0) return "";  

	return AStr.substr(mbegin, mend - mbegin + 1);
}

bool File_Exist(string AFileName)  
{
	FILE * fp;
	fp = fopen(AFileName.c_str(), "r");
	if (fp == NULL)
	{
		return false;
	}
	fclose(fp);
	return true;
}

string File_GetFilePath(string AFullFileName)  
{
	int mIndex;

    for (mIndex = AFullFileName.length() - 1; mIndex >=0 ; mIndex --)
	{
		if ((AFullFileName[mIndex] == '\\') || (AFullFileName[mIndex] == ':'))
		{
			break;
		}
	}   
	
	if (mIndex < 0)
	{
		return "";
	}
	else
	{
		return AFullFileName.substr(0, mIndex + 1);   
	}
}

string File_GetFileExt(string AFileName)  
{
	int mIndex;
	
    for (mIndex = AFileName.length() - 1; mIndex >=0 ; mIndex --)
	{
		if ((AFileName[mIndex] == '.') || (AFileName[mIndex] == '\\') || (AFileName[mIndex] == ':'))
		{
			break;
		}
	}   

	if (mIndex < 0 || mIndex == 0)
	{
		return "";
	}
	else if (AFileName[mIndex] == '.')
	{
		return AFileName.substr(mIndex + 1, AFileName.length() - mIndex - 1);
	}
	else
	{
		return "";
	}
}

string File_GetFileNameWithoutExt(string AFileName)  
{
	
	string mFileName = File_GetFileName(AFileName);  
	
	int mIndex;
	
    for (mIndex = mFileName.length() - 1; mIndex >=0 ; mIndex --)
	{
		if (mFileName[mIndex] == '.')
		{
			break;
		}
	}   

	if (mIndex < 0) 
	{
		return mFileName;
	}
	else
	{
		return mFileName.substr(0, mIndex);
	}
}

string File_GetFileName(string AFullFileName)  
{
	int mIndex;
	
    for (mIndex = AFullFileName.length() - 1; mIndex >=0 ; mIndex --)
	{
		if ((AFullFileName[mIndex] == '\\') || (AFullFileName[mIndex] == ':'))
		{
			break;
		}
	}   
	
	if (mIndex < 0)
	{
		return AFullFileName;
	}
	else
	{
		return AFullFileName.substr(mIndex + 1, AFullFileName.length() - mIndex - 1);   
	}
}

string Path_AddBackSlash(string APath)  
{
	if (APath == "") return "";
	if (APath[APath.length() - 1] != '\\') return APath + '\\';
	return APath;
}

string File_FileName_AddSuffix(string AFileName,string ASuffix)  
{
	string mPath = File_GetFilePath(AFileName);
	string mFileNameWithoutExt = File_GetFileNameWithoutExt(AFileName);
	string mExt = File_GetFileExt(AFileName);

	string mIndexFileName = mPath + mFileNameWithoutExt + ASuffix;
	if (mExt != "")  
	{
		mIndexFileName += '.';
		mIndexFileName += mExt;
	}
	return mIndexFileName;

}

string File_FileName_AddIndex(string AFileName,int AIndex)  
{
	string mSuffix = "_";
	mSuffix += JIntToStr(AIndex);
	return File_FileName_AddSuffix(AFileName, mSuffix);
}

