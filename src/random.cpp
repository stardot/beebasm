/*************************************************************************************************/
/**
	random.cpp

        Simple Lehmer random number generator; used instead of the native C library's generator 
        so RANDOMIZE+RND() gives consistent results on all platforms. The constants used are those
        for C++11's minstd_rand. See https://en.wikipedia.org/wiki/Lehmer_random_number_generator

	Copyright (C) Rich Talbot-Watkins 2007 - 2012
	Copyright (C) Steven Flintham 2016

	This file is part of BeebAsm.

	BeebAsm is free software: you can redistribute it and/or modify it under the terms of the GNU
	General Public License as published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	BeebAsm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with BeebAsm, as
	COPYING.txt.  If not, see <http://www.gnu.org/licenses/>.
*/
/*************************************************************************************************/

#include "random.h"

static unsigned long state = 19670512;

static unsigned long modulus = BEEBASM_RAND_MODULUS;

void beebasm_srand(unsigned long seed)
{
        state = seed % modulus;
        if ( state == 0 )
        {
                state = 1;
        }
}

unsigned long beebasm_rand()
{
        state = ( BEEBASM_RAND_MULTIPLIER * state ) % modulus;
        // It's always true that 1 <= state <= (modulus - 1), so we return state - 1 to make
        // 0 a possible value. BEEBASM_RAND_MAX is modulus - 2, so we have 0 <= return value <=
        // BEEBASM_RAND_MAX as required for compatibility with the interface of rand().
        return state - 1;
}
