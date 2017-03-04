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

#ifndef VMALLOCATOR_H
#define VMALLOCATOR_H

#include <functional>
#include <unordered_map>
#include <stdexcept>

#include "AllocationProblem.h"

#define COEFF_NR_OF_ACTIVE_HOSTS 10
#define COEFF_NR_OF_MIGRATIONS 1

using AllocationMapType = std::unordered_map <VM*, PM*>;

class VMAllocator
{
public:
	// destructor
	virtual ~VMAllocator(){};

	// solves the allocation problem
	virtual void solve() = 0;

	// returns the cost of the best allocation found, or -1 when no allocation was found
	virtual double getBestCost() = 0;

	// returns the best allocation found
	virtual const AllocationMapType& getBestAllocation()=0;

	// return the number of active hosts in the best allocation
	virtual int getActiveHosts()=0;

	// return the number of migrations in the best allocation
	virtual int getMigrations()=0;

	// return lower bound
	virtual double getLowerBound()=0;
};

#endif
