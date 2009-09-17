/*
 * Copyright (C) 2004 - 2005 by
 *     Hieu Xuan Phan & Minh Le Nguyen {hieuxuan, nguyenml}@jaist.ac.jp
 *     Graduate School of Information Science,
 *     Japan Advanced Institute of Science and Technology (JAIST)
 *
 * mathlib.cpp - this file is part of FlexCRFs.
 *
 * Begin:	Dec. 15, 2004
 * Last change:	Sep. 09, 2005
 *
 * FlexCRFs is a free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * FlexCRFs is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FlexCRFs; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include "mathlib.h"
#include "global.h"
extern int g_debug;

void mathlib::mult(int size, doublevector * x, doublematrix * A, doublevector * y, 
		    int is_transposed) {
    // is_transposed = 0 (false): 	x = A * y
    // is_transposed = 1 (true):	x^t = y^t * A^t
    // in which x^t, y^t, and A^t are the transposed vectors and matrix
    // of x, y, and A, respectively 
    
    int i, j;
    
    if (!is_transposed) {
	// for beta
	// x = A * y
	for (i = 0; i < size; i++) {
	    x->vect[i] = 0;
	    for (j = 0; j < size; j++) {
		x->vect[i] += A->mtrx[i][j] * y->vect[j];
	    }
	}
	
    } else {
	// for alpha
	// x^t = y^t * A^t
	for (i = 0; i < size; i++) {
	    x->vect[i] = 0;
	    for (j = 0; j < size; j++) {
		x->vect[i] += y->vect[j] * A->mtrx[j][i];
	    }
	}
    }
}

void mathlib::mult_unigram(int size, doublevector * x, CSparseMatrix &SparseMatrix, doublevector * y, 
		    int is_transposed) {
    // is_transposed = 0 (false): 	x = A * y
    // is_transposed = 1 (true):	x^t = y^t * A^t
    // in which x^t, y^t, and A^t are the transposed vectors and matrix
    // of x, y, and A, respectively 

	
	double sum = 0.0;
		
	for (int i = 0; i < size; i++)
	{
	
		sum += y->vect[i];
	}
	if (!is_transposed) {
		for( int i = 0;i<size;i++)
		{
			x->vect[i] = sum;
			double sum_1 = 0.0;
			double sum_2 = 0.0;
			for(int j = 0;j<SparseMatrix.row_index[i].size();j++)
			{	
				sum_1 += y->vect[SparseMatrix.row_index[i][j]] * (SparseMatrix.Get(i,SparseMatrix.row_index[i][j]));
				sum_2 += y->vect[SparseMatrix.row_index[i][j]];
				if(sum-sum_2<0.0)
				{
					cout<<"sum"<<sum<<endl;
					for(int j = 0;j<size;j++)
					{
						cerr<<j<<" "<<y->vect[j]<<endl;
					}
					for(int j = 0;j<SparseMatrix.row_index[i].size();j++)
					{
						double sum_temp = 0.0;
						cerr<<"i j "<<i<<" "<<j<<endl;
						cerr<<"SparseMatrix.row_index[i][j]="<<SparseMatrix.row_index[i][j]<<endl;
						cerr<<"y->vect[SparseMatrix.row_index[i][j]]="<<y->vect[SparseMatrix.row_index[i][j]]<<endl;
						sum_temp += y->vect[SparseMatrix.row_index[i][j]];
						cerr<<"sum_temp"<<sum_temp<<endl;
					}		
				}
			}
			x->vect[i] -=sum_2;
			x->vect[i] +=sum_1;

/*			for(int j = 0;j<SparseMatrix.row_index[i].size();j++)
			{
				if(x->vect[i] - y->vect[SparseMatrix.row_index[i][j]]<0.0)
				{
					cerr<<"origan one"<<x->vect[i]<<endl;
					cerr<<"after one" <<x->vect[i] - y->vect[SparseMatrix.row_index[i][j]]<<endl;
					g_debug = 1;
				}
				x->vect[i] -= y->vect[SparseMatrix.row_index[i][j]];	
				if(g_debug ==1)
				{
					cerr<<"sum"<<sum<<endl;
					for(int j = 0;j<SparseMatrix.row_index[i].size();j++)
					{	
						double sum_temp = sum;
						sum_temp += y->vect[SparseMatrix.row_index[i][j]] * (SparseMatrix.Get(i,SparseMatrix.row_index[i][j]));
						cerr<<"i j "<<i<<" "<<j<<endl;
						cerr<<"SparseMatrix.row_index[i][j]="<<SparseMatrix.row_index[i][j]<<endl;
						cerr<<"y->vect[SparseMatrix.row_index[i][j]]="<<y->vect[SparseMatrix.row_index[i][j]]<<endl;
						cerr<<"SparseMatrix.Get(i,SparseMatrix.row_index[i][j])="<<SparseMatrix.Get(i,SparseMatrix.row_index[i][j])<<endl;
						cerr<<"sum_temp"<<sum_temp<<endl;
					}
					cerr<<"*******************************"<<endl;
					g_debug = 0;
				}
			}*/		
		}
	} else {
		for (int i = 0; i < size; i++)
		{
			x->vect[i] = sum;
			double sum_1 = 0.0;
			double sum_2 = 0.0;	
			for(int j = 0;j<SparseMatrix.col_index[i].size();j++)
			{
				sum_1 += y->vect[SparseMatrix.col_index[i][j]] * (SparseMatrix.Get(SparseMatrix.col_index[i][j],i));
				sum_2 += y->vect[SparseMatrix.col_index[i][j]];
				if(sum-sum_2<0.0)
				{
					cout<<"sum"<<sum<<endl;
					for(int j = 0;j<size;j++)
					{
						cerr<<j<<" "<<y->vect[j]<<endl;
					}
					for(int j = 0;j<SparseMatrix.col_index[i].size();j++)
					{
						double sum_temp = 0.0;
						cerr<<"i j "<<i<<" "<<j<<endl;
						cerr<<"SparseMatrix.col_index[i][j]="<<SparseMatrix.col_index[i][j]<<endl;
						cerr<<"y->vect[SparseMatrix.col_index[i][j]]="<<y->vect[SparseMatrix.col_index[i][j]]<<endl;
						sum_temp += y->vect[SparseMatrix.col_index[i][j]];
						cerr<<"sum_temp"<<sum_temp<<endl;
					}		
				}
			}
			x->vect[i] -= sum_2;
			x->vect[i] += sum_1;
			
	
			/*for(int j = 0;j<SparseMatrix.col_index[i].size();j++)
			{
				if(x->vect[i] - y->vect[SparseMatrix.col_index[i][j]]<0.0)
				{
					cerr<<"origan one"<<x->vect[i]<<endl;
					cerr<<"after one" <<x->vect[i] - y->vect[SparseMatrix.col_index[i][j]]<<endl;
					g_debug = 1;
				}
				x->vect[i] -= y->vect[SparseMatrix.col_index[i][j]];
				if(g_debug ==1)
				{
					cerr<<"sum"<<sum<<endl;
					for(int j = 0;j<SparseMatrix.col_index[i].size();j++)
					{	
						double sum_temp = sum;
						sum_temp += y->vect[SparseMatrix.col_index[i][j]] * (SparseMatrix.Get(SparseMatrix.col_index[i][j],i));
						cerr<<"i j "<<i<<" "<<j<<endl;
						cerr<<"SparseMatrix.col_index[i][j]="<<SparseMatrix.col_index[i][j]<<endl;
						cerr<<"y->vect[SparseMatrix.col_index[i][j]]="<<y->vect[SparseMatrix.col_index[i][j]]<<endl;
						cerr<<"SparseMatrix.Get(SparseMatrix.col_index[i][j],i)="<<SparseMatrix.Get(SparseMatrix.col_index[i][j],i)<<endl;
						cerr<<"sum_temp"<<sum_temp<<endl;
					}
					cerr<<"*******************************"<<endl;
					g_debug = 0;	
				}	
			}*/	
		}
	}
}


void mathlib::mult_bigram(int size, doublevector * x, CSparseMatrix &SparseMatrix, doublevector * y, int is_transposed) {
    // is_transposed = 0 (false): 	x = A * y
    // is_transposed = 1 (true):	x^t = y^t * A^t
    // in which x^t, y^t, and A^t are the transposed vectors and matrix
    // of x, y, and A, respectively 


	double sum = 0.0;
	
	for (int i = 0; i < size; i++)
	{
		sum += y->vect[i];
	}
	for (int i = 0; i < size; i++)
	{
		x->vect[i] = sum;
	}
	
	if (!is_transposed) {	
		for (int i = 0; i < SparseMatrix.row.size(); i++)
		{
			int k = SparseMatrix.row[i];
			
			double sum_1 = 0.0;
			double sum_2 = 0.0;
			for(int j = 0;j<SparseMatrix.row_index[k].size();j++)
			{
				sum_1 += y->vect[SparseMatrix.row_index[k][j]] * (SparseMatrix.row_value[k][j]);
				sum_2 += y->vect[SparseMatrix.row_index[k][j]];
				
			}
			x->vect[k] -=sum_2;
			x->vect[k] += sum_1;		
		}
		
		
	} else {
		for (int i = 0; i < SparseMatrix.col.size(); i++)
		{
			int k = SparseMatrix.col[i];
	
			double sum_1 = 0.0;
			double sum_2 = 0.0;
			for(int j = 0;j<SparseMatrix.col_index[k].size();j++)
			{
				sum_1 += y->vect[SparseMatrix.col_index[k][j]] * (SparseMatrix.col_value[k][j]);
				sum_2 += y->vect[SparseMatrix.col_index[k][j]];
			}
			x->vect[k] -= sum_2;
			x->vect[k] += sum_1;	
		}
	}
}



