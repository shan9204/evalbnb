/*
Copyright 2015 David Bartok, Zoltan Adam Mann

This file is part of VMAllocation.

VMAllocation is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VMAllocation is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VMAllocation. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BNBPARAMS_H
#define BNBPARAMS_H

#include <string>
#include <iostream>

#include "AllocatorParams.h"

enum SortType
{
	NONE,
	LEXICOGRAPHIC,
	MAXIMUM,
	SUM
};

struct BnBParams : public AllocatorParams
{
	bool failFirst;

	bool intelligentBound;

	SortType PMSortMethod;
	SortType VMSortMethod;
	bool initialPMFirst;
	bool symmetryBreaking; // causes the loss of optimality

	double boundThreshold; // bound also when (cost >= bestSoFar * boundThreshold), makes sense when between 0 and 1
};

static SortType stringToSortType(const std::string& toConvert)
{
	if (toConvert == "NONE")
	{
		return NONE;
	}
	else if (toConvert == "LEXICOGRAPHIC")
	{
		return LEXICOGRAPHIC;
	}
	else if (toConvert == "MAXIMUM")
	{
		return MAXIMUM;
	}
	else if (toConvert == "SUM")
	{
		return SUM;
	}
	else
	{
		std::cout << "WARNING: Invalid Sort Type. Defaulting to NONE." << std::endl;
		return NONE;
	}
}

#endif