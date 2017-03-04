#include <algorithm>
#include "GreedyAllocator.h"


GreedyAllocator::GreedyAllocator(AllocationProblem pr, std::shared_ptr<AllocatorParams> pa, std::ofstream& l)
	:m_problem(pr), m_log(l)
{
	m_numVMs = m_problem.VMs.size();
	m_numPMs = m_problem.PMs.size();
	m_dimension = m_problem.VMs[0].demand.size(); // only works if all VMs have the same number of dimensions
	m_bestCost=-1;
	m_numPMsOn=-1;
	m_numMigrations=-1;
	m_params = *pa;
	m_numMaxMigrations = m_numPMs / m_params.maxMigrationsRatio;

	for(auto& pm : m_problem.PMs)
	{
		m_pms.push_back(&pm);
		std::vector<int> load;
		for(int k=0;k<m_dimension;k++)
			load.push_back(0);
		m_load_of_pms[&pm]=load;
	}
	for(auto& vm : m_problem.VMs)
	{
		PM * pm=&m_problem.PMs[vm.initialID];
		m_vms_of_pms[pm].insert(&vm);
		for(int k=0;k<m_dimension;k++)
			m_load_of_pms[pm][k]+=vm.demand[k];
	}
}


bool GreedyAllocator::pm_less_guazz(PM *a, PM *b)
{
	//first criterion: power state
	if(m_vms_of_pms[a].empty() && !(m_vms_of_pms[b].empty()))
		return false;
	if(m_vms_of_pms[b].empty() && !(m_vms_of_pms[a].empty()))
		return true;
	//second criterion: CPU load
	if(m_load_of_pms[a][0]<m_load_of_pms[b][0])
		return false;
	if(m_load_of_pms[a][0]>m_load_of_pms[b][0])
		return true;
	//third criterion: idle power consumption -> doesn't matter here because we don't differentiate between PMs' power consumption characteristics
	return false;
}


bool GreedyAllocator::pm_less_numvm(PM *a, PM *b)
{
	return m_vms_of_pms[a].size()<m_vms_of_pms[b].size();
}


/*
bool GreedyAllocator::vm_less(VM *a, VM *b)
{
	return a->demand[0]>b->demand[0];
}
*/


PM * GreedyAllocator::find_pm_for_vm(VM * vm)
{
	PM * result=NULL;
	//std::sort(m_pms.begin(),m_pms.end(),pm_less_guazz);
	using namespace std::placeholders;
	std::sort(m_pms.begin(),m_pms.end(),std::bind(&GreedyAllocator::pm_less_guazz, this, _1, _2));
	std::vector<PM*>::iterator it_pm;
	for(it_pm=m_pms.begin();it_pm!=m_pms.end();it_pm++)
	{
		PM * pm=*it_pm;
		bool fit=true;
		for(int k=0;k<m_dimension;k++)
			if(m_load_of_pms[pm][k]+vm->demand[k]>pm->capacity[k])
				fit=false;
		if(fit)
		{
			result=pm;
			break;
		}
	}
	return result;
}


void GreedyAllocator::migrate(VM * vm, PM * pm1, PM * pm2)
{
	m_vms_of_pms[pm1].erase(vm);
	m_vms_of_pms[pm2].insert(vm);
	for(int k=0;k<m_dimension;k++)
	{
		m_load_of_pms[pm1][k]-=vm->demand[k];
		m_load_of_pms[pm2][k]+=vm->demand[k];
	}
	m_numMigrations++;
}


void GreedyAllocator::solve()
{
	//std::vector<PM*> pms(m_problem.PMs);
	//std::sort(pms.begin(),pms.end(),pm_less);
	//std::vector<VM*> vms(m_problem.VMs);
	//std::sort(vms.begin(),vms.end(),vm_less);
	//std::vector<VM*> vms_to_migrate;
	m_numMigrations=0;
	//relieve overloaded hosts
	for(auto& pm : m_problem.PMs)
	{
		bool changed;
		do
		{
			changed=false;
			bool overloaded=false;
			for(int k=0;k<m_dimension;k++)
			{
				if(m_load_of_pms[&pm][k]>pm.capacity[k])
					overloaded=true;
			}
			if(overloaded)
			{
				std::set<VM*>::iterator it_vm=m_vms_of_pms[&pm].begin();
				VM * vm=*it_vm;
				PM * pm2=find_pm_for_vm(vm);
				if(pm2!=NULL && m_numMigrations<m_numMaxMigrations)
				{
					migrate(vm,&pm,pm2);
					changed=true;
				}
			}
		} while(changed && m_numMigrations<m_numMaxMigrations);
	}
	//consolidate
	//std::sort(m_pms.begin(),m_pms.end(),pm_less_numvm);
	using namespace std::placeholders;
	std::sort(m_pms.begin(),m_pms.end(),std::bind(&GreedyAllocator::pm_less_numvm, this, _1, _2));
	std::vector<PM*>::iterator it_pm=m_pms.begin();
	while(it_pm!=m_pms.end() && m_numMigrations<m_numMaxMigrations)
	{
		PM * pm=*it_pm;
		it_pm++;
		std::set<VM*> vms_to_migrate=m_vms_of_pms[pm];
		if((signed)vms_to_migrate.size()>m_numMaxMigrations-m_numMigrations)
			break;
		std::set<VM*>::iterator it_vm;
		for(it_vm=vms_to_migrate.begin();it_vm!=vms_to_migrate.end();it_vm++)
		{
			VM * vm=*it_vm;
			PM * pm2=find_pm_for_vm(vm);
			if(pm2!=NULL)
				migrate(vm,pm,pm2);
			else
				break;
		}
	}
	//calculate number of PMs that are on
	m_numPMsOn=0;
	for(it_pm=m_pms.begin();it_pm!=m_pms.end();it_pm++)
	{
		PM * pm=*it_pm;
		if(m_vms_of_pms[pm].size()>0)
			m_numPMsOn++;
	}
	m_bestCost=COEFF_NR_OF_ACTIVE_HOSTS*m_numPMsOn+COEFF_NR_OF_MIGRATIONS*m_numMigrations;
	//update m_bestAllocation
	for(auto pm : m_pms)
	{
		std::set<VM*> vms=m_vms_of_pms[pm];
		for(auto vm : vms)
		{
			m_bestAllocation[vm]=pm;
		}
	}
}


double GreedyAllocator::getBestCost()
{
	return m_bestCost;
}


const AllocationMapType& GreedyAllocator::getBestAllocation()
{
	return m_bestAllocation;
}


int GreedyAllocator::getActiveHosts()
{
	return m_numPMsOn;
}


int GreedyAllocator::getMigrations()
{
	return m_numMigrations;
}


double GreedyAllocator::getLowerBound()
{
	return 0;
}

