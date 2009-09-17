#pragma once

#include <vector>

using namespace std;

typedef struct tag_Sequence
{
	vector<int> tagSequence;
	double score;

	bool operator==(const tag_Sequence &right) const;
    bool operator!=(const tag_Sequence &right) const;
    bool operator<(const tag_Sequence &right) const;
    bool operator>(const tag_Sequence &right) const;
    bool operator<=(const tag_Sequence &right) const;
    bool operator>=(const tag_Sequence &right) const;
	
}SEQUENCE;

inline bool tag_Sequence::operator==(const tag_Sequence &right) const
{
	if (score == right.score)
		return true;
	else
		return false;
}

inline bool tag_Sequence::operator!=(const tag_Sequence &right) const
{
	if (score != right.score)
		return true;
	else
		return false;
}


inline bool tag_Sequence::operator<(const tag_Sequence &right) const
{
	if (score < right.score)
		return true;
	else
		return false;
}

inline bool tag_Sequence::operator>(const tag_Sequence &right) const
{
	if (score > right.score)
		return true;
	else
		return false;
}

inline bool tag_Sequence::operator<=(const tag_Sequence &right) const
{
	if (score <= right.score)
		return true;
	else
		return false;
}

inline bool tag_Sequence::operator>=(const tag_Sequence &right) const
{
	if (score >= right.score)
		return true;
	else
		return false;
}