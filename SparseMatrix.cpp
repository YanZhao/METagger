/***************************************************************************
 *   Copyright (C) 2008 by Yan Zhao   *
 *   yzhao@holde   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "SparseMatrix.h"
#include <math.h>
#include <algorithm>
#include "global.h"

CSparseMatrix::CSparseMatrix(void)
{
	value_index = false;
	other = 0.0;
}

void CSparseMatrix::Init(void)
{
	map< pair<int, int>, double>::iterator it1 = matrix.begin();
	while(it1 != matrix.end())
	{
		it1->second = 0.0;
		it1++;
	}
	other = 0.0;
}

bool CSparseMatrix::Exist(int i,int j)
{
	pair<int,int> pairIndex;
	pairIndex.first = i;
	pairIndex.second = j;
	if(matrix.find(pairIndex) != matrix.end())
	{
		return true;
	}
	else
	{
		return false;
	}


}

/*void CSparseMatrix::SequenceExp(int num_tag)
{
	map< pair<int, int>, double>::iterator itr;
	itr = matrix.begin();
	while(itr !=matrix.end())
	{
		pair<int,int> temp = itr->first;
		if(temp.first%num_tag = temp.second/num_tag)
		{
			itr->second = exp(itr->second);
		}
		itr++;
	}
	other = 0.0;
	
}*/

vector<int> CSparseMatrix::ColIndex(int col)
{
	return col_index[col];
}

vector<int> CSparseMatrix::RowIndex(int row)
{
	return row_index[row];
}

void CSparseMatrix::BuildRowColIndex(int num_tag)
{
	row_index.resize(num_tag);
	col_index.resize(num_tag);

	row_value.resize(num_tag);
	col_value.resize(num_tag);
	
	map< pair<int, int>, double>::iterator itr;
	itr = matrix.begin();
	while(itr !=matrix.end())
	{
		pair<int,int> temp = itr->first;
		row_index[temp.first].push_back(temp.second);
		row_value[temp.first].push_back(itr->second);

		col_index[temp.second].push_back(temp.first);
		col_value[temp.second].push_back(itr->second);
		itr++;
	}

	row.clear();
	col.clear();
	for(int i = 0;i<num_tag;i++)
	{
		if(!row_index[i].empty())
		{
			row.push_back(i);
		}
		if(!col_index[i].empty())
		{
			col.push_back(i);
		}
	}
	
	if(!g_Bigram)
	{
		value.resize(num_tag);
		for(int i = 0;i<num_tag;i++)
		{
			int max = 0;
			if(row_index[i].size()>0)
			{
				max = *max_element(row_index[i].begin(),row_index[i].end());
				value[i].resize(max+1,1.0);
			}
		}
	}
}
void CSparseMatrix::ValueIndex(int num_tag)
{
	if(g_Bigram)
	{
		for(int i = 0;i<num_tag;i++)
		{
			row_value[i].clear();
			col_value[i].clear();
		}
		map< pair<int, int>, double>::iterator itr;
		itr = matrix.begin();
		while(itr !=matrix.end())
		{
			pair<int,int> temp = itr->first;
			row_value[temp.first].push_back(itr->second);
			col_value[temp.second].push_back(itr->second);
			itr++;
		}
	}
	else
	{
		map< pair<int, int>, double>::iterator itr;
		itr = matrix.begin();
		while(itr !=matrix.end())
		{
			pair<int,int> temp = itr->first;
			value[temp.first][temp.second] = itr->second;
			itr++;
		}
		value_index = true;
	}
}

void CSparseMatrix::AllExp(bool flag)
{
	map< pair<int, int>, double>::iterator itr;
	itr = matrix.begin();
	while(itr !=matrix.end())
	{
		itr->second = exp(itr->second);
		itr++;
	}
	if(flag == true) 
	{
		other = 1.0;
	}
}

double CSparseMatrix::Tag_Get(int i,int j)
{
	//return value[i][j];

	pair<int,int> pairIndex;
	pairIndex.first = i;
	pairIndex.second = j;
	if(matrix.find(pairIndex) != matrix.end())
	{
		return matrix[pairIndex];
	}
	else
	{
		return other;
	}
}
void CSparseMatrix::Add(int i, int j,double value)
{
	pair<int,int> pairIndex;
	pairIndex.first = i;
	pairIndex.second = j;
	
	matrix[pairIndex] += value;
	
}

void CSparseMatrix::AddToMap(int i, int j,double value)
{
	pair<int,int> pairIndex;
	pairIndex.first = i;
	pairIndex.second = j;
	if(matrix.find(pairIndex) != matrix.end())
	{
		matrix[pairIndex] = value;
	}
	else
	{
		pair< pair<int,int>,double> temp;
		temp.first = pairIndex;
		temp.second = value;
		matrix.insert(temp);
	}
}


void CSparseMatrix::Set(int i, int j,double value)
{
	pair<int,int> pairIndex;
	pairIndex.first = i;
	pairIndex.second = j;

	matrix[pairIndex] = value;
	
}
double CSparseMatrix::Get(int i,int j)
{
	if(value_index)
	{
		return value[i][j];
	}
	else
	{
		pair<int,int> pairIndex;
		pairIndex.first = i;
		pairIndex.second = j;
		if(matrix.find(pairIndex) != matrix.end())
		{
			return matrix[pairIndex];
		}
		else
		{
			return other;
		}
	}
}
double CSparseMatrix::Get_Col(int i,int j)
{
	return col_value[j][i];
}
double CSparseMatrix::Get_Row(int i,int j)
{
	return row_value[i][j];
}

int CSparseMatrix::GetSize()
{
	return matrix.size();
}
