//#include <stdafx.h>
#include "JEvaluate.h"
#include <fstream>
#include "HashFunction.h"

double JEvaluate_Pos(string ARightPosFileName, string ACheckPosFileName, string AResultFileNameSuffix)  
{
	
	if (File_Exist(ARightPosFileName) == false)
	{
		cout << "不存在正确词性标注的文件：" << ARightPosFileName << endl;
		return -1;
	}
	if (File_Exist(ACheckPosFileName) == false)
	{
		cout << "不存在需要检测词性标注的文件：" << ACheckPosFileName << endl;
		return -1;
	}

	string rightPosFileName = ARightPosFileName;
	string checkPosFileName = ACheckPosFileName;

	string mPath = File_GetFilePath(ACheckPosFileName);  

	//3、设置评测生成文件名称
	string comparePosFileName = mPath + "PosCompare.txt";            
	string comparePosFileNameWord = mPath + "PosErrorTotal.txt";      
	string mEvaluateFileName = mPath + "PosEvaluate.txt";            

	
	if (AResultFileNameSuffix != "")
	{
		comparePosFileName = File_FileName_AddSuffix(comparePosFileName, AResultFileNameSuffix);
		comparePosFileNameWord = File_FileName_AddSuffix(comparePosFileNameWord, AResultFileNameSuffix);
		mEvaluateFileName = File_FileName_AddSuffix(mEvaluateFileName, AResultFileNameSuffix);
	}

	fstream rightPosFile, checkPosFile;
	rightPosFile.open(rightPosFileName.c_str(), ios_base::in);
	checkPosFile.open(checkPosFileName.c_str(), ios_base::in);

	FILE * comparePosFile , * comparePosFileWord, * mEvaluateFile; 
	comparePosFile = fopen(comparePosFileName.c_str(), "w");
	comparePosFileWord = fopen(comparePosFileNameWord.c_str(), "w");
	mEvaluateFile = fopen(mEvaluateFileName.c_str(), "w");
	

	hash_map<string,int,StrHashTrait<string> > mHashWordError;
	vector<string> vectRightWord, vectCheckWord;

	string rightPos;
	string checkPos;
	int mSumWord = 0;
	int mErrorWord = 0;

	const string ct_Separator = "  ";

	int mCompareRowCount = 0;  
	while (( !rightPosFile.eof()) && (!checkPosFile.eof()))
	{
		
		if (getline(rightPosFile, rightPos) == NULL) break;
		if (getline(checkPosFile, checkPos) == NULL) break;

		
		mCompareRowCount ++;  //

		
		vectRightWord.clear();
		String_SeperateToList_WithTrim(rightPos, vectRightWord, ct_Separator);


		
		mSumWord += vectRightWord.size();

		
		if (rightPos != checkPos)  
		{
			
			fprintf(comparePosFile, "%s\n", rightPos.c_str());
			fprintf(comparePosFile, "%s\n", checkPos.c_str());
			fprintf(comparePosFile, "\n");

			
			vectCheckWord.clear();
			String_SeperateToList_WithTrim(checkPos, vectCheckWord, ct_Separator);

			
			for (int k = 0 ; k < vectRightWord.size(); k ++)
			{
				string & mRightWordPos = vectRightWord[k];
				string & mCheckWordPos = vectCheckWord[k];
				if (mRightWordPos != mCheckWordPos)
				{
					
					mErrorWord ++;
				
					int mIndex = mRightWordPos.find("/", 0);
					string mWord = mRightWordPos.substr(0, mIndex);  
							
					string rightPosTag = mRightWordPos.substr( mIndex + 1 , mRightWordPos.length() - mIndex - 1 );
					string errorPosTag = mCheckWordPos.substr( mIndex + 1 , mCheckWordPos.length() - mIndex - 1 );
					string key = mWord + "*" + rightPosTag + "*" + errorPosTag;

					
					
					if (mHashWordError.find(key) != mHashWordError.end())   
					{
						mHashWordError[key] = mHashWordError[key] + 1;
					}
					else  
					{
						mHashWordError.insert(pair<string, int>(key, 1));
					}
				}

			}

		}
	}

	hash_map<string,int,StrHashTrait<string> > ::iterator mIter, mHashBegin, mHashEnd;
	mHashBegin = mHashWordError.begin();
	mHashEnd = mHashWordError.end(); 

	for (mIter = mHashBegin ; mIter != mHashEnd; mIter ++)
	{
		const string &mKey = mIter -> first;
		int &mValue = mIter -> second;
		fprintf(comparePosFileWord, "%s  %d\n", mKey.c_str(), mValue);
	}



	fprintf(mEvaluateFile, "正确率 = %16.15f\n" , (1 - ((double)mErrorWord / mSumWord)));
	fprintf(mEvaluateFile, "总词数 = %d\n" , mSumWord);
	fprintf(mEvaluateFile, "错误标注词数 = %d\n" , mErrorWord);
	fprintf(mEvaluateFile, "文件行数 = %d\n" , mCompareRowCount);

	
	rightPosFile.close();
	checkPosFile.close();

	fclose(comparePosFile);
	fclose(comparePosFileWord);
	fclose(mEvaluateFile);

	
	return (1 - ((double)mErrorWord / mSumWord));


}
