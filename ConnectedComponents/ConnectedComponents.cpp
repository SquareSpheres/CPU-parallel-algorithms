
#include "stdafx.h"
#include <iostream>
#include <vector>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <omp.h>
#include <unordered_map>
#include "Utils.h"

using namespace std;
using namespace boost;



vector<int> BoostConnectedComponent(adjacency_list<vecS, vecS, undirectedS> &graph)
{
	std::vector<int> component(num_vertices(graph));
	int num = connected_components(graph, &component[0]);
	return component;
}



vector<int> ParallelConnectedComponentRetarded(std::pair<int, int> *graph, const int numVertices, const int numEdges)
{

	std::vector<int> componentV(numVertices);

#pragma omp parallel for
	for (int i = 0; i < numVertices; i++)
	{
		componentV[i] = i;
	}


	bool hasGrafted = true;


	while (hasGrafted)
	{
		hasGrafted = false;

#pragma omp parallel for
		for (int i = 0; i < numEdges; ++i)
		{
			const int from = graph[i].first;
			const int to = graph[i].second;

			if (componentV[from] > componentV[to])
			{
				componentV[componentV[from]] = componentV[to];
				hasGrafted = true;
			}
			else if (componentV[from] < componentV[to])
			{
				componentV[componentV[to]] = componentV[from];
				hasGrafted = true;
			}

		}

#pragma omp parallel for
		for (int i = 0; i < numVertices; i++)
		{
			while (componentV[i] != componentV[componentV[i]])
			{
				componentV[i] = componentV[componentV[i]];
			}
		}
	}

	// post processing. Inlcude this in runtime?
	std::unordered_map<int, int> uniqueComp;
	int count = 0;

	for (int i = 0; i < numVertices; i++)
	{
		int value = componentV[i];
		if (uniqueComp.find(value) == uniqueComp.end())
		{
			uniqueComp.insert({ value, count++ });
		}
		componentV[i] = uniqueComp[componentV[i]];
	}


	return componentV;

}


vector<int> ParallelConnectedComponent(std::pair<int, int> *graph, const int numVertices, const int numEdges)
{

	std::vector<int> component(numVertices);

#pragma omp parallel for
	for (int i = 0; i < numVertices; i++)
	{
		component[i] = i;
	}

	bool hasGrafted = true;

	while (hasGrafted)
	{
		hasGrafted = false;

#pragma omp parallel
		{


#pragma omp for
			for (int i = 0; i < numEdges; ++i)
			{
				const int fromVertex = graph[i].first;
				const int toVertex = graph[i].second;

				const int fromComponent = component[fromVertex];
				const int toComponent = component[toVertex];

				if ((fromComponent < toComponent) && (toComponent == component[toComponent]))
				{
					hasGrafted = true;
					component[toComponent] = fromComponent;
				}
			}

#pragma omp for
			for (int i = 0; i < numVertices; ++i)
			{
				while (component[i] != component[component[i]])
				{
					component[i] = component[component[i]];
				}
			}
		}

	}

	// post processing. Inlcude this in runtime?
	std::unordered_map<int, int> uniqueComp;
	int count = 0;

	for (int i = 0; i < numVertices; i++)
	{
		int value = component[i];
		if (uniqueComp.find(value) == uniqueComp.end())
		{
			uniqueComp.insert({ value, count++ });
		}

		component[i] = uniqueComp[component[i]];
	}

	return component;
}



int main()
{


	typedef vector<std::pair<int, int>> StdGraph;
	typedef vector<std::pair<int, int>> StdGraphBi;
	typedef adjacency_list<vecS, vecS, undirectedS> BoostGraph;


	


	vector<int> co1;
	vector<int> co2;
	vector<int> co3;


	do
	{
		BoostGraph boostGraph = GenerateRandomGraphBoost(1000000, 0.000015);
		StdGraph stdGraph;
		StdGraphBi stdGraphBi;


		
		const std::pair<int, int> numVertEdgBi = FromBoostToStdGraphBi(boostGraph, &stdGraphBi);
		const std::pair<int, int> numVertEdg = FromBoostToStdGraph(boostGraph, &stdGraph);
		



		/*	for (auto it = stdGraph.begin(); it != stdGraph.end(); ++it)
			{
				cout << it->first << "," << it->second << endl;
			}*/

		const int numVerticesBoost = num_vertices(boostGraph);
		const int numEdgesBoost = num_edges(boostGraph);

		const int numVerticesStdBi = numVertEdgBi.first;
		const int numEdgesStdBi = numVertEdgBi.second;

		const int numVerticesStd = numVertEdg.first;
		const int numEdgesStd = numVertEdg.second;


		cout << "numVerti STD  = " << numVerticesStd << endl;
		cout << "numEdges STD  = " << numEdgesStd << endl;

		cout << "numVerti STDBi = " << numVerticesStdBi << endl;
		cout << "numEdges STDBi = " << numEdgesStdBi << endl;

		cout << "numVerti BOOST = " << numVerticesBoost << endl;
		cout << "numEdges BOOST = " << numEdgesBoost << endl;


		const double boostTime = omp_get_wtime();
		co1 = BoostConnectedComponent(boostGraph);
		const double boostTimeEnd = omp_get_wtime() - boostTime;

		const double parRetardTime = omp_get_wtime();
		co2 = ParallelConnectedComponentRetarded(&stdGraph[0], numVerticesStd, numEdgesStd);
		const double parRetardTimeEnd = omp_get_wtime() - parRetardTime;

		const double parTime = omp_get_wtime();
		co3 = ParallelConnectedComponent(&stdGraphBi[0], numVerticesStdBi, numEdgesStdBi);
		const double parTimeEnd = omp_get_wtime() - parTime;

		cout << "boost time = " << boostTimeEnd << endl;
		cout << "retar time = " << parRetardTimeEnd << endl;
		cout << "paral time = " << parTimeEnd << endl;



	} while (co1 == co2 && co1 == co3 && co2 == co3);

	cout << "boost = ";
	PrintVector<int>(co1);
	cout << "seque = ";
	PrintVector<int>(co2);
	cout << "paral = ";
	PrintVector<int>(co3);


	return 0;
}

