#pragma once

#include "global.h"
#include "doublevector.h"
#include "doublematrix.h"


class Viterbi {


public:
	Viterbi();
    	~Viterbi();
	
    	doublematrix * Mi;
    	doublevector * Vi;
};
