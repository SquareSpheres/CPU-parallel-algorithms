#include "stdafx.h"
#include <iostream>
#include <vector>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <omp.h>
#include "Utils.h"
#include "ShiloachesVishkinOmp.h"
#include "ShiloachVishkinAcc.h"


using namespace std;
using namespace boost;



/// <summary>
/// Sequential connected component algorithm using DFS. Running time is O(V + E). This function simply uses a out-of-the-box algorithm
/// found in the Boost graph library.
/// </summary>
/// <param name="graph">The graph.</param>
/// <returns></returns>
vector<int> BoostConnectedComponent(adjacency_list<vecS, vecS, undirectedS> &graph)
{
	std::vector<int> component(num_vertices(graph));
	int num = connected_components(graph, &component[0]);
	return component;
}


int main()
{


	typedef vector<std::pair<int, int>> StdGraph;
	typedef adjacency_list<vecS, vecS, undirectedS> BoostGraph;


	vector<int> baseLineBoostGraph;
	vector<int> svOmp;
	vector<int> svOmpUpdt;
	


	do
	{

		// 4 to 1 == 4000000, 0.000002
		// 2 to 1 == 5000000, 0.0000008
		BoostGraph boostGraph = GenerateRandomGraphBoost(4000000, 0.000002);
		StdGraph stdGraph;
		StdGraph stdGraphBi;



		const std::pair<int, int> numVertEdg = FromBoostToStdGraph(boostGraph, &stdGraph);
		const std::pair<int, int> numVertEdgUpdt = FromBoostToStdGraph(boostGraph, &stdGraphBi);


		/*	for (auto it = stdGraph.begin(); it != stdGraph.end(); ++it)
			{
				cout << it->first << "," << it->second << endl;
			}*/

		const int numVerticesBoost = num_vertices(boostGraph);
		const int numEdgesBoost = num_edges(boostGraph);

		const int numVerticesStd = numVertEdg.first;
		const int numEdgesStd = numVertEdg.second;

		const int numVerticesStdUpdt = numVertEdgUpdt.first;
		const int numEdgesStdUpdt = numVertEdgUpdt.second;


		cout << "numVerti STD  = " << numVerticesStd << endl;
		cout << "numEdges STD  = " << numEdgesStd << endl;

		cout << "numVerti BOOST = " << numVerticesBoost << endl;
		cout << "numEdges BOOST = " << numEdgesBoost << endl;


		const double boostTime = omp_get_wtime();
		baseLineBoostGraph = BoostConnectedComponent(boostGraph);
		const double boostTimeEnd = omp_get_wtime() - boostTime;
		
		const double parTime = omp_get_wtime();
		svOmp = ShiloachVishkinOmp(&stdGraph[0], numVerticesStd, numEdgesStd);
		const double parTimeEnd = omp_get_wtime() - parTime;

		const double parUpdTime = omp_get_wtime();
		svOmpUpdt = ShiloachVishkinUpdtOmp(&stdGraphBi[0], numVerticesStdUpdt, numEdgesStdUpdt);
		const double parUpdTimeEnd = omp_get_wtime() - parUpdTime;

		cout << "boost time			= " << boostTimeEnd << endl;
		cout << "paral time			= " << parTimeEnd << endl;
		cout << "paralIpdt time			= " << parUpdTimeEnd << endl;



	} while (baseLineBoostGraph == svOmp && svOmp == svOmpUpdt);



	return 0;
}

