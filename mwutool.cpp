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
#include "mwutool.h"
#include <cstdlib>
#include <iostream>

using namespace std;

//bool IsMWUTag()
//{
//}
bool MWUTool::IsNotMWUBegingTag(string t1)
{
  int t1_begin=1; 
  int t1_end=1;
  string t1_tag(t1);
  
  int p,q;
  string tmp;

  p=t1.find("/",0);
  if (p != string::npos) {
    tmp=t1.substr(0,p);
    t1_begin=atoi(tmp.c_str());
    q=t1.find("-",p);
    if (q != string::npos) {
      tmp=t1.substr(p+1,q-p);
      t1_end=atoi(tmp.c_str());
      t1_tag=t1.substr(q+1,t1.length()-q);
    } else {
      t1_begin=1;
    }
  }

  if(t1_begin>1 && t1_end>1)
    return true;
   return false;
}


int MWUTool::legal_transition_no(string t1, string t2) 
{
  // return codes:
  // 0: not a legal transition
  // 1: legal non-mwu transition
  // 2: obliged mwu transition i/j-T --> i+1/j-T
  // NOTE: unexpected results are expected for tags which have the shape
  //  xxxx/xxx-xxxx but which are not mwu tags.

  int t1_begin=1;
  int t1_end=1;
  int t2_begin=1;
  int t2_end=1;
  string t1_tag(t1);
  string t2_tag(t2);

  int p,q;
  string tmp;

//BE- -->0
//CO- -->1
//ED- -->2
//Other- -->4  
int t1_flag = -1;
int t2_flag = -1;

t1_flag = ht_mwu_no(t1);
t2_flag = ht_mwu_no(t2);

if(t1_flag !=4)
{
    q=t1.find("-",0);
    if (q != string::npos) {
      t1_tag=t1.substr(q+1,t1.length()-q);
  }
	else
	{
		cout<<"check function g_legal_transition"<<endl;
		exit(1);
	}

}

if(t2_flag !=4)
{
    q=t2.find("-",0);
    if (q != string::npos) {
      t2_tag=t2.substr(q+1,t2.length()-q);
    } else {
 		cout<<"check function g_legal_transition"<<endl;
		exit(1);
    }
}

  int mwu_transition = ( t1_tag == t2_tag &&
			 ((t1_flag ==0&& t2_flag==1)|| (t2_flag == 2 &&t1_flag ==0) ||
				(t1_flag ==1&& t2_flag==1)
				|| (t1_flag ==1&& t2_flag==2)
				)
			 );


  if (t1_flag ==0||t1_flag ==1|| t2_flag==1||t2_flag==2) {
    if(mwu_transition)
      return 2;
    else
      return 0;
  }

  return 1;
}

int MWUTool::ht_mwu_no(string s)
{
int q;
 q=s.find("BE-",0);
    if (q != string::npos) {
	return 0;
}

 q=s.find("CO-",0);
    if (q != string::npos) {
	return 1;
}


 q=s.find("EN-",0);
    if (q != string::npos) {
	return 2;
}

return 4;
}

int MWUTool::legal_transition(string t1, string t2) 
{
  // return codes:
  // 0: not a legal transition
  // 1: legal non-mwu transition
  // 2: obliged mwu transition i/j-T --> i+1/j-T
  // NOTE: unexpected results are expected for tags which have the shape
  //  xxxx/xxx-xxxx but which are not mwu tags.

  int t1_begin=1; 
  int t1_end=1;
  int t2_begin=1;
  int t2_end=1;
  string t1_tag(t1);
  string t2_tag(t2);

  int p,q;
  string tmp;

  p=t1.find("/",0);
  if (p != string::npos) {
    tmp=t1.substr(0,p);
    t1_begin=atoi(tmp.c_str());
    q=t1.find("-",p);
    if (q != string::npos) {
      tmp=t1.substr(p+1,q-p);
      t1_end=atoi(tmp.c_str());
      t1_tag=t1.substr(q+1,t1.length()-q);
    } else {
      t1_begin=1;
    }
  }

  p=t2.find("/",0);
  if (p != string::npos) {
    tmp=t2.substr(0,p);
    t2_begin=atoi(tmp.c_str());
    q=t2.find("-",p);
    if (q != string::npos) {
      tmp=t2.substr(p+1,q-p);
      t2_end=atoi(tmp.c_str());
      t2_tag=t2.substr(q+1,t2.length()-q);
    } else {
      t2_begin=1;
    }
  }

  int mwu_transition = ( t1_tag == t2_tag &&
			 t2_begin == t1_begin + 1 &&
			 t1_end == t2_end 
			 );


  if (t1_begin < t1_end || t2_begin > 1) {
    if(mwu_transition)
      return 2;
    else
      return 0;
  }

  return 1;
}

int MWUTool::legal_transition_simp_no(string t1, string t2) 
{
  // return codes:
  // 0: not a legal transition
  // 1: legal non-mwu transition
  // 2: obliged mwu transition i/j-T --> i+1/j-T
  // NOTE: unexpected results are expected for tags which have the shape
  //  xxxx/xxx-xxxx but which are not mwu tags.

  int t1_begin=1;
  int t1_end=1;
  int t2_begin=1;
  int t2_end=1;
  string t1_tag(t1);
  string t2_tag(t2);

  int p,q;
  string tmp;

//BE- -->0
//CO- -->1
//Other- -->4  
int t1_flag = -1;
int t2_flag = -1;

t1_flag = ht_mwu_no(t1);
t2_flag = ht_mwu_no(t2);

if(t1_flag !=4)
{
    q=t1.find("-",0);
    if (q != string::npos) {
      t1_tag=t1.substr(q+1,t1.length()-q);
  }
	else
	{
		cout<<"check function g_legal_transition"<<endl;
		exit(1);
	}

}

if(t2_flag !=4)
{
    q=t2.find("-",0);
    if (q != string::npos) {
      t2_tag=t2.substr(q+1,t2.length()-q);
    } else {
 		cout<<"check function g_legal_transition"<<endl;
		exit(1);
    }
}

  int mwu_transition = ( t1_tag == t2_tag &&
			 ((t1_flag ==0&& t2_flag==1)
				|| (t1_flag ==1&& t2_flag==1)
				));

  if(t1_flag == 0)
	{
		if(mwu_transition)
      			return 2;
    		else
      			return 0;
	}

  if (t2_flag == 1 ) {
    if(mwu_transition)
      return 1;
    else
      return 0;
  }

  return 1;
}


int MWUTool::ht_mwu(string t1) 
{
  // return codes:
  // 0: not a legal transition
  // 1: legal non-mwu transition
  // 2: obliged mwu transition i/j-T --> i+1/j-T
  // NOTE: unexpected results are expected for tags which have the shape
  //  xxxx/xxx-xxxx but which are not mwu tags.

  int t1_begin=1;
  int t1_end=1;
  string t1_tag(t1);
  

  int p,q;
  string tmp;

  p=t1.find("/",0);
  if(p != string::npos)
 {
    tmp=t1.substr(0,p);
    t1_begin=atoi(tmp.c_str());
    q=t1.find("-",p);
    if (q != string::npos) {
      tmp=t1.substr(p+1,q-p);
      t1_end=atoi(tmp.c_str());
    if((t1_begin == 1))
    {
	return t1_end;
    }
    else if(t1_begin <t1_end)
    {
	return 1;
    }
    else
    {
	return 0;
    }
    }
 }

  return -1;
}




