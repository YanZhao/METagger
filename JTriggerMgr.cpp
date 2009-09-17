//#include <stdafx.h>
#include "JTriggerMgr.h"
#include "PosMaxEn.h"
#include <fstream>

CJTriggerTxtFeature::CJTriggerTxtFeature(string ATxtFileName)
{
	m_TxtFileName = ATxtFileName;
}

CJTriggerTxtFeature::~CJTriggerTxtFeature()
{
}

void CJTriggerTxtFeature::clear()  //清空数据
{
	m_vectTxtFeature.clear();
}

void CJTriggerTxtFeature::AddTriggerFeature(FEATURE &AFeature)
{
	TRIGGER_TEXT_FEATURE mTemp;
	JChangeFeatureToTriggerFeature(AFeature, mTemp);  //将特征转换为文本特征
	m_vectTxtFeature.push_back(mTemp);
}

bool CJTriggerTxtFeature::SaveToFile()      //保存触发器特征到文件
{
	return JSaveToFile(m_TxtFileName);
}

bool CJTriggerTxtFeature::LoadFromFile()    //从文件中装载触发器特征
{
	clear();   //清空原来的数据
	return JLoadFromFile(m_TxtFileName);
}

void CJTriggerTxtFeature::CopyTriggerFeature(POS_PRED_TYPE APredType, vector<FEATURE> & AToFeatureVector)  //拷贝当前所有触发器Feature追加到AToFeatureVector末尾
{
	FEATURE mF;
	mF.predType = APredType;


	for (int k = 0; k < m_vectTxtFeature.size(); k ++)
	{
		TRIGGER_TEXT_FEATURE & mCurTF = m_vectTxtFeature[k];
		//
//		mF.predType  = pos_trigger;
		mF.alpha = mCurTF.alpha;
		mF.outTag = mCurTF.outTag;
		for (int mI = 0; mI < mCurTF.vect_A.size(); mI ++)
		{
			mF.predData = mCurTF.vect_A[mI] + "+" + mCurTF.B;
			AToFeatureVector.push_back(mF);
		}
	}
}
bool CJTriggerTxtFeature::JLoadFromFile(string AFileName)
{
	if (File_Exist(AFileName) == false)
	{
		cout << "Can not open File:" << AFileName << endl;
		return false;
	}
	fstream fp;
	fp.open(AFileName.c_str(), ios_base::in);
	vector<string> mFenJie;
	string mRowLine;

    TRIGGER_TEXT_FEATURE mTemp;
	mTemp.B = "";
	mTemp.outTag = -1;
	
	while (!fp.eof())
	{
		mRowLine = "";
        getline(fp, mRowLine);
		//处理当前行数据
		if (mRowLine != "")  //增加当前行数据到特征向量中
		{
			//分解数据
			String_SeperateToList(mRowLine, mFenJie, "\t");

			//装载数据到内存
			if (mFenJie[0] != "")  //此时代表为对应的被触发字串 ，和输出标记
			{
				if (mFenJie.size() != 2)
				{
					cout << "Error: Trigger Feature Data Format:" << mRowLine << endl;
					return false;
				}
				mTemp.B = mFenJie[0];
				mTemp.outTag = JStrToInt(mFenJie[1]);
			}
			else  //此时第一个为空，代表触发者，以及权重信息
			{
				if (mFenJie.size() < 3)
				{
					cout << "Error: Trigger Feature Data Format:" << mRowLine << endl;
					return false;
				}
				if (mTemp.outTag == -1) 
				{
					cout << "Error: First Row Is not The B and outTag" << endl;
					return false;
				}

				mTemp.vect_A.clear();

				for (int k = 1; k < mFenJie.size() - 1 ; k ++)
				{
					string &A = mFenJie[k];
					mTemp.vect_A.push_back(A);  //增加A
				}
				mTemp.alpha = JStrToDouble(mFenJie[mFenJie.size() - 1]);

				//追加到内存中去
				m_vectTxtFeature.push_back(mTemp);
			}

		}
	}
	fp.close();
	return true;
}

bool CJTriggerTxtFeature::JSaveToFile(string AFileName)
{
	FILE * fp;
	fp = fopen(AFileName.c_str(), "w");
	if (fp == NULL)
	{
		cout << "Can not Create File:" << AFileName << endl;
		return false;
	}

	//第一步，排序已有的特征
	SortTxtFeature();
	//第二步、进行特征合并
	MergerTxtFeature();
	//第三步、进行特征保存
	string mPreB = "";
	int mPreTag = -1;
	for (int k = 0; k < m_vectTxtFeature.size(); k ++)  
	{
		//因为排序了，所以对于相邻的一致需要合并
		TRIGGER_TEXT_FEATURE & mCurFeature = m_vectTxtFeature[k];
		if (( mCurFeature.B == mPreB) && (mCurFeature.outTag == mPreTag)) //需要合并
		{
			//保存A
			for (int mI =0; mI < mCurFeature.vect_A.size(); mI ++)
			{
				fprintf(fp, "\t%s", mCurFeature.vect_A[mI].c_str());
			}
			//保存alpha
			fprintf(fp, "\t%f\n", mCurFeature.alpha);
		}
		else  //如果不相同，不必要合并
		{
			//保存特征输出
			mPreB = mCurFeature.B;
			mPreTag = mCurFeature.outTag;
			fprintf(fp, "%s\t%d\n", mPreB.c_str(), mPreTag);

			//保存A
			for (int mI =0; mI < mCurFeature.vect_A.size(); mI ++)
			{
				fprintf(fp, "\t%s", mCurFeature.vect_A[mI].c_str());
			}
			//保存alpha
			fprintf(fp, "\t%f\n", mCurFeature.alpha);
		}
	}

	fclose(fp);
	return true;
}

void CJTriggerTxtFeature::SortTxtFeature()     //进行排序
{
	sort(m_vectTxtFeature.begin(),m_vectTxtFeature.end());
}

void CJTriggerTxtFeature::MergerTxtFeature()   //进行特征合并
{
	//进行相同特征的合并。假设已经进行排序了，所以，只需要对于相邻连续可以合并的触发信息
	vector<TRIGGER_TEXT_FEATURE> mTemp;   //作为临时排序中转
	//首先保存第一个
	if (m_vectTxtFeature.size() > 0)
	{
		mTemp.push_back(m_vectTxtFeature[0]);
	}
	//后面的判断，如果合前面的一致，进行合并
	for (int k = 1; k < m_vectTxtFeature.size(); k ++)  
	{
		//因为排序了，所以对于相邻的一致需要合并
		TRIGGER_TEXT_FEATURE & mCurFeature = m_vectTxtFeature[k];
        TRIGGER_TEXT_FEATURE & mPreFeature = mTemp[mTemp.size() - 1];
		if ( mCurFeature == mPreFeature)  //如果相等就可以合并
		{
			for (int mI = 0; mI < mCurFeature.vect_A.size(); mI ++)
			{
				mPreFeature.vect_A.push_back(mCurFeature.vect_A[mI]);
			}
		}
		else  //如果不相同，不必要合并
		{
			mTemp.push_back(mCurFeature);
		}
	}

	m_vectTxtFeature.clear();
	//还原
	m_vectTxtFeature = mTemp;

}

void CJTriggerTxtFeature::JChangeFeatureToTriggerFeature(FEATURE &AFeature, TRIGGER_TEXT_FEATURE & AResultFeature)
{
	string mA_B = AFeature.predData;  //提取出来数据
	int mIndex = mA_B.find("+", 0);
	string mA = mA_B.substr(0, mIndex);                                     //提取'+'前面的字串
	AResultFeature.B = mA_B.substr(mIndex + 1, mA_B.size() - mIndex - 1);   //提取'+'后面的字串
	// I change feature structure, so I need change it later 
	//AResultFeature.outTag = AFeature.outTag;
	AResultFeature.vect_A.push_back(mA);
	AResultFeature.alpha = AFeature.alpha;
}

void CJTriggerTxtFeature::SetTxtFileName(string ATxtFileName)
{
	m_TxtFileName = ATxtFileName;
}

