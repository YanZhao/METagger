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

void CJTriggerTxtFeature::clear()  //�������
{
	m_vectTxtFeature.clear();
}

void CJTriggerTxtFeature::AddTriggerFeature(FEATURE &AFeature)
{
	TRIGGER_TEXT_FEATURE mTemp;
	JChangeFeatureToTriggerFeature(AFeature, mTemp);  //������ת��Ϊ�ı�����
	m_vectTxtFeature.push_back(mTemp);
}

bool CJTriggerTxtFeature::SaveToFile()      //���津�����������ļ�
{
	return JSaveToFile(m_TxtFileName);
}

bool CJTriggerTxtFeature::LoadFromFile()    //���ļ���װ�ش���������
{
	clear();   //���ԭ��������
	return JLoadFromFile(m_TxtFileName);
}

void CJTriggerTxtFeature::CopyTriggerFeature(POS_PRED_TYPE APredType, vector<FEATURE> & AToFeatureVector)  //������ǰ���д�����Feature׷�ӵ�AToFeatureVectorĩβ
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
		//����ǰ������
		if (mRowLine != "")  //���ӵ�ǰ�����ݵ�����������
		{
			//�ֽ�����
			String_SeperateToList(mRowLine, mFenJie, "\t");

			//װ�����ݵ��ڴ�
			if (mFenJie[0] != "")  //��ʱ����Ϊ��Ӧ�ı������ִ� ����������
			{
				if (mFenJie.size() != 2)
				{
					cout << "Error: Trigger Feature Data Format:" << mRowLine << endl;
					return false;
				}
				mTemp.B = mFenJie[0];
				mTemp.outTag = JStrToInt(mFenJie[1]);
			}
			else  //��ʱ��һ��Ϊ�գ��������ߣ��Լ�Ȩ����Ϣ
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
					mTemp.vect_A.push_back(A);  //����A
				}
				mTemp.alpha = JStrToDouble(mFenJie[mFenJie.size() - 1]);

				//׷�ӵ��ڴ���ȥ
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

	//��һ�����������е�����
	SortTxtFeature();
	//�ڶ��������������ϲ�
	MergerTxtFeature();
	//��������������������
	string mPreB = "";
	int mPreTag = -1;
	for (int k = 0; k < m_vectTxtFeature.size(); k ++)  
	{
		//��Ϊ�����ˣ����Զ������ڵ�һ����Ҫ�ϲ�
		TRIGGER_TEXT_FEATURE & mCurFeature = m_vectTxtFeature[k];
		if (( mCurFeature.B == mPreB) && (mCurFeature.outTag == mPreTag)) //��Ҫ�ϲ�
		{
			//����A
			for (int mI =0; mI < mCurFeature.vect_A.size(); mI ++)
			{
				fprintf(fp, "\t%s", mCurFeature.vect_A[mI].c_str());
			}
			//����alpha
			fprintf(fp, "\t%f\n", mCurFeature.alpha);
		}
		else  //�������ͬ������Ҫ�ϲ�
		{
			//�����������
			mPreB = mCurFeature.B;
			mPreTag = mCurFeature.outTag;
			fprintf(fp, "%s\t%d\n", mPreB.c_str(), mPreTag);

			//����A
			for (int mI =0; mI < mCurFeature.vect_A.size(); mI ++)
			{
				fprintf(fp, "\t%s", mCurFeature.vect_A[mI].c_str());
			}
			//����alpha
			fprintf(fp, "\t%f\n", mCurFeature.alpha);
		}
	}

	fclose(fp);
	return true;
}

void CJTriggerTxtFeature::SortTxtFeature()     //��������
{
	sort(m_vectTxtFeature.begin(),m_vectTxtFeature.end());
}

void CJTriggerTxtFeature::MergerTxtFeature()   //���������ϲ�
{
	//������ͬ�����ĺϲ��������Ѿ����������ˣ����ԣ�ֻ��Ҫ���������������Ժϲ��Ĵ�����Ϣ
	vector<TRIGGER_TEXT_FEATURE> mTemp;   //��Ϊ��ʱ������ת
	//���ȱ����һ��
	if (m_vectTxtFeature.size() > 0)
	{
		mTemp.push_back(m_vectTxtFeature[0]);
	}
	//������жϣ������ǰ���һ�£����кϲ�
	for (int k = 1; k < m_vectTxtFeature.size(); k ++)  
	{
		//��Ϊ�����ˣ����Զ������ڵ�һ����Ҫ�ϲ�
		TRIGGER_TEXT_FEATURE & mCurFeature = m_vectTxtFeature[k];
        TRIGGER_TEXT_FEATURE & mPreFeature = mTemp[mTemp.size() - 1];
		if ( mCurFeature == mPreFeature)  //�����ȾͿ��Ժϲ�
		{
			for (int mI = 0; mI < mCurFeature.vect_A.size(); mI ++)
			{
				mPreFeature.vect_A.push_back(mCurFeature.vect_A[mI]);
			}
		}
		else  //�������ͬ������Ҫ�ϲ�
		{
			mTemp.push_back(mCurFeature);
		}
	}

	m_vectTxtFeature.clear();
	//��ԭ
	m_vectTxtFeature = mTemp;

}

void CJTriggerTxtFeature::JChangeFeatureToTriggerFeature(FEATURE &AFeature, TRIGGER_TEXT_FEATURE & AResultFeature)
{
	string mA_B = AFeature.predData;  //��ȡ��������
	int mIndex = mA_B.find("+", 0);
	string mA = mA_B.substr(0, mIndex);                                     //��ȡ'+'ǰ����ִ�
	AResultFeature.B = mA_B.substr(mIndex + 1, mA_B.size() - mIndex - 1);   //��ȡ'+'������ִ�
	// I change feature structure, so I need change it later 
	//AResultFeature.outTag = AFeature.outTag;
	AResultFeature.vect_A.push_back(mA);
	AResultFeature.alpha = AFeature.alpha;
}

void CJTriggerTxtFeature::SetTxtFileName(string ATxtFileName)
{
	m_TxtFileName = ATxtFileName;
}

