//////////////////////////////////////////////////////////////
//
//  author : JiangWei
//  Date : 2004 02 22
//  Function:利用标准C++重写赵岩使用C#写的评测程序
//
//////////////////////////////////////////////////////////////

#ifndef _JEVALUATE__H_
#define _JEVALUATE__H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <queue>

#ifdef linux
#include <ext/hash_map>
#include <ext/hash_set>
#else
#include <hash_map>
#include <hash_set>
#endif

#include "JBaseProc.h"
#include "StrHashTrait.h"

using namespace std;
#ifdef linux
using namespace __gnu_cxx;
#endif 

double JEvaluate_Pos(string ARightPosFileName, string ACheckPosFileName, string AResultFileNameSuffix = "");  


#endif