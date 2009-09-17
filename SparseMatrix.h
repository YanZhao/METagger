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
#pragma once

#include <map>
#include <iostream>
#include <vector>

/////////////////////////////////////////////////////////
//here, we store some basic data structure, such as feature, predicate, and event. it's good starting 
//point for you to navigate the whole project. 

using namespace std;

class CSparseMatrix
{
public:
	map< pair<int, int>, double> matrix;
	vector< vector< double > > value;
	double other;
	bool value_index;
	vector< vector<int> > row_index;
	vector< vector<int> > col_index;

	vector< vector<double> > row_value;
	vector< vector<double> > col_value;
	vector<int> row;
	vector<int> col;
	
public:
	CSparseMatrix(void);
	void AllExp(bool flag = true);
	double Get(int i,int j);
	void Set(int i, int j,double value);
	void Add(int i, int j,double value);
	void AddToMap(int i, int j,double value);
	bool Exist(int i,int j);
	int GetSize();
	void Init(void);
	vector<int> RowIndex(int row);
	vector<int> ColIndex(int col);
	void BuildRowColIndex(int num_tag);
	void ValueIndex(int num_tag = 0);
	double Tag_Get(int i,int j);
	double Get_Col(int i,int j);
	double Get_Row(int i,int j);

};

