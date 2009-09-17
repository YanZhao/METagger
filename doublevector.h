/*
 * Copyright (C) 2004 - 2005 by
 *     Hieu Xuan Phan & Minh Le Nguyen {hieuxuan, nguyenml}@jaist.ac.jp
 *     Graduate School of Information Science,
 *     Japan Advanced Institute of Science and Technology (JAIST)
 *
 * doublevector.h - this file is part of FlexCRFs.
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

#ifndef _DOUBLEVECTOR_H
#define _DOUBLEVECTOR_H

#include <iostream>

using namespace std;

class doublevector {
public:
    double * vect;	// vector content
    int len;		// vector length	
    
    // default constructor
    doublevector() {
	len = 0; 
	vect = NULL;
    }
    
    // constructor with length
    doublevector(int len);
    
    // constructor with length and content
    doublevector(int len, double * vect);
    
    // copy constructor
    doublevector(doublevector & dv);
    
    // destructor
    ~doublevector() {
	if (vect) {
	    delete vect;
	}
    }
    
    // the size of vector
    int size();
    
    // overloading assignment operator
    void operator=(double val);
    
    // overloading assignment operator
    void operator=(doublevector & dv);
    
    // assign the same value to all vector elements
    void assign(double val);
    
    // assign values for all elements from another doublevector
    void assign(doublevector & dv);
    
    // reference to an element of index "idx"
    double & operator[](int idx);
    
    // sum of all vector elements
    double sum();
    
    // component multiplication
    void comp_mult(double val);
    
    // component multiplication
    void comp_mult(doublevector * dv);
};

#endif
