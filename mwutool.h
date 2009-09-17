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
#ifndef MWUTOOL_H
#define MWUTOOL_H

/**
	@author Yan Zhao <yzhao@holde>
*/
#include <string>
using namespace std;

class MWUTool{
public:
   	static int ht_mwu(string t1);
	static int legal_transition(string t1, string t2) ;
	static int ht_mwu_no(string s);
	static int legal_transition_no(string t1, string t2) ;
	static bool IsNotMWUBegingTag(string t1);
	static int legal_transition_simp_no(string t1, string t2) ;

};

#endif
