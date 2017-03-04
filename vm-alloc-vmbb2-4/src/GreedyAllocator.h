#ifndef GREEDYALLOCATOR_H
#define GREEDYALLOCATOR_H

#include <set>
#include <map>
#include <memory>
#include "VMAllocator.h"
#include "AllocatorParams.h"

class GreedyAllocator: public VMAllocator
{
private:
	AllocationProblem m_problem; // the allocation problem
	AllocatorParams m_params; // algorithm parameters
	std::ofstream& m_log; // output log file
	int m_dimension; // dimension of resources
	int m_numVMs; // number of Virtual Machines
	int m_numPMs; // number of Physical Machines
	double m_bestCost;
	AllocationMapType m_bestAllocation;
	int m_numMaxMigrations;
	int m_numMigrations;
	int m_numPMsOn;
	std::vector<PM*> m_pms;
	std::map<PM*,std::set<VM*>> m_vms_of_pms;
	std::map<PM*,std::vector<int>> m_load_of_pms;
	bool pm_less_guazz(PM *a, PM *b);
	bool pm_less_numvm(PM *a, PM *b);
	PM * find_pm_for_vm(VM * vm);
	void migrate(VM * vm, PM * pm1, PM * pm2);
public:
	GreedyAllocator(AllocationProblem pr, std::shared_ptr<AllocatorParams> pa, std::ofstream& l);
	void solve() final override;
	double getBestCost() final override;
	const AllocationMapType& getBestAllocation() final override;
	int getActiveHosts() final override;
	int getMigrations() final override;
	double getLowerBound() final override;
};

#endif /* GREEDYALLOCATOR_H */
