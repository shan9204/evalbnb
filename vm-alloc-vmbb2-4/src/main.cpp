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

/*
The VMAllocation project is currently under development, this version
is a research prototype intended only for internal usage.
*/

/*
Test configurations can be set up in the this file.
Result files are created in the .\logs folder.
*/

#include <cstdio>
#include <iostream>
#include <vector>
#include <fstream>
#include <climits>
#include <memory>

#include "BnBAllocator.h"
#include "ILPAllocator.h"
#include "AllocationProblem.h"
#include "ProblemGenerator.h"
#include "Timer.h"
#include "AllocatorParams.h"
#include "Utils.h"
#include "ConfigParser.h"
#include "GreedyAllocator.h"

using std::cout;
using std::vector;
using std::ofstream;
using std::endl;

int main()
{
	std::string timeString = currentDateTime();
	#ifdef WIN32
		ofstream log("logs\\Log_" + timeString + ".txt");
	#else
		ofstream log("logs/Log_" + timeString + ".txt");
	#endif
	if (!log.good())
	{
		cout << "Cannot create log files. Maybe the .\\logs folder doesn't exist?" << endl;
		getchar();
		return 1;
	}

	Timer t;

	ConfigParser parser("config.txt");
	parser.parse();
	std::unique_ptr<ProblemGenerator> generator = parser.getGenerator();
	ParamsPtrVectorType paramsList = parser.getParamsList();

	// initialize result file
	#ifdef WIN32
		ofstream output("logs\\Output_" + timeString + ".csv");
	#else
		ofstream output("logs/Output_" + timeString + ".csv");
	#endif

	//problem data
	output << "Input problem";
	output << "; ";

	bool showDetailedCost = parser.getShowDetailedCost();
	for (unsigned i = 0; i < paramsList.size(); i++) // columns for runtimes
		output << paramsList[i]->name << ": time; ";
	for (unsigned i = 0; i < paramsList.size(); i++) // columns for other data
	{
		output << paramsList[i]->name << ": lower bound; ";
		output << paramsList[i]->name << ": cost; ";
		if(showDetailedCost)
		{
			output << paramsList[i]->name << ": PMs on; ";
			output << paramsList[i]->name << ": migrations;";
		}
	}

	output << endl;

	ConfigParser::Steps vmSteps = parser.getVMs();
	ConfigParser::Steps pmSteps = parser.getPMs();
	int numVMs = vmSteps.from;
	int numPMs = pmSteps.from;
	while (numVMs <= vmSteps.to && numPMs <= pmSteps.to)
	{
		// run tests
		cout << "VMs: " << numVMs << " PMs: " << numPMs << ", Running " << parser.getNumTests() << " test(s) with " << paramsList.size() << " parameter setups each..." << endl;

		generator->setNumVMsNumPMs(numVMs, numPMs); // finalizing generator
		for (int i = 0; i < parser.getNumTests(); i++) // run for all instances
		{
			cout << "Instance " << i << ":" << endl;

			AllocationProblem problem = generator->generate_ff();

			// logging problem data
			#ifdef VERBOSE_BASIC	
				log << "Instance " << i << ":" << endl;

				int dimension = problem.VMs[0].demand.size();
				log << std::endl << "PMs:\t";
				for (auto pm : problem.PMs)
				{
					log << "[";
					for (int i = 0; i < dimension; i++)
					{
						log << pm.capacity[i];
						if (i != dimension - 1)
							log << " ";
					}

					log << "] ";
				}
				log << std::endl << "VMs:\t";
				for (auto vm : problem.VMs)
				{
					log << "[";
					for (int i = 0; i < dimension; i++)
					{
						log << vm.demand[i];
						if (i != dimension - 1)
							log << " ";
					}

					log << "] ";
				}
				log << std::endl << "init:\t";
				for (auto vm : problem.VMs)
				{
					log << vm.id << "->" << vm.initialID << " ";
				}
				log << std::endl << std::endl;
			#endif

			vector<double> solutions; // costs
			vector<int> activeHosts;
			vector<int> migrations;
			vector<double> lowerBounds;

			output << numVMs << " VMs, " << numPMs << " PMs";
			output << "; ";
			for (unsigned i = 0; i < paramsList.size(); i++) // run current instance for all configurations
			{
				cout << "\t" << paramsList[i]->name << "..." << std::flush;

				#ifdef VERBOSE_BASIC	
					log << "Parameter configuration: " << paramsList[i]->name << std::endl << std::endl;
				#endif

				std::shared_ptr<VMAllocator> vmAllocator;
				if (paramsList[i]->allocatorType == BnB)
				{
					vmAllocator = std::make_shared<BnBAllocator>(problem, paramsList[i], log);
				}
				else if (paramsList[i]->allocatorType == ILP)
				{
					vmAllocator = std::make_shared<ILPAllocator>(problem, paramsList[i], log);
				}
				else if (paramsList[i]->allocatorType == Greedy)
				{
					vmAllocator = std::make_shared<GreedyAllocator>(problem, paramsList[i], log);
				}
				double loBo=vmAllocator->getLowerBound();
				lowerBounds.push_back(loBo);
				t.start();
				vmAllocator->solve();
				double elapsed = t.getElapsedTime();
				cout << " DONE!" << endl;
				output << elapsed;
				output << "; ";
				double opt = vmAllocator->getBestCost();
				solutions.push_back(opt);
				if (showDetailedCost)
				{
					activeHosts.push_back(vmAllocator->getActiveHosts());
					migrations.push_back(vmAllocator->getMigrations());
				}
				#ifdef VERBOSE_BASIC			
					log << "Solution = " << opt << endl;
					log << "------------------" << endl;
				#endif
			}

			for (unsigned i = 0; i < paramsList.size(); i++)
			{
				output << lowerBounds[i];
				output << "; ";
				output << solutions[i];
				output << "; ";
				if (showDetailedCost)
				{
					output << activeHosts[i];
					output << "; ";
					output << migrations[i];
					output << "; ";
				}
			}

			output << endl;
			#ifdef VERBOSE_BASIC			
				log << "===== End of instance =====" << endl;
			#endif
		}

		output << endl;
		#ifdef VERBOSE_BASIC			
			log << "===== End of simulation for this size =====" << endl;
		#endif
		numVMs += vmSteps.step;
		numPMs += pmSteps.step;
	}

	output.close();
	log.close();
	cout << "(Finished.)" << endl;
}
