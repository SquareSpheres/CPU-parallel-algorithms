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


/// <summary>
/// Parallel Shiloaches-vishkin algorithm. The graph is an array of std::pair<int,int>, where each pair represents a edge from a vertex to antoher.
/// E.g. std::pair{0,3} represents an edge from vertex 0 to vertex 3. The algorithm returns a vector with size |numVertices|, where each index 
/// represents what component that vertex belongs to. E.g. component[7] will give you vertex 7s component.
/// </summary>
/// <param name="graph">An array std::pair</param>
/// <param name="numVertices">The number vertices.</param>
/// <param name="numEdges">The number edges.</param>
/// <returns></returns>
vector<int> ShiloachVishkin(std::pair<int, int> *graph, const int numVertices, const int numEdges)
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

				/**
				 *	In traditional Shiloach-Vishkin algorithm, every edge is represented as (u,v) and (v,u) in the graph.
				 *	To save memory, I only have (u,v) in the graph, but iterate the edge twice so I get both (u,v), and (v,u).
				 *
				 *	TODO move grafting to its own function
				 *	TODO move shortcutting to its own function
				 *
				 *
				 */

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

				{
					const int fromVertex = graph[i].second;
					const int toVertex = graph[i].first;

					const int fromComponent = component[fromVertex];
					const int toComponent = component[toVertex];

					if ((fromComponent < toComponent) && (toComponent == component[toComponent]))
					{
						hasGrafted = true;
						component[toComponent] = fromComponent;
					}
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

/// <summary>
/// Parallel Shiloaches-vishkin algorithm. The graph is an array of std::pair<int,int>, where each pair represents a edge from a vertex to antoher.
/// E.g. std::pair{0,3} represents an edge from vertex 0 to vertex 3. The algorithm returns a vector with size |numVertices|, where each index 
/// represents what component that vertex belongs to. E.g. component[7] will give you vertex 7s component.
///
/// This version of the algorithm differ from the original one in that it has a additional step update. This update step causes access to 
/// the component array to be more coalesced. From the paper Fast Parallel Connected Components Algorithms on GPUs by Guojing Cong and Paul Muzio :
///	Theorem 2. On average in each iteration CC-updt issues at least n/2 fewer random accesses than CC
/// </summary>
/// <param name="graph">An array std::pair</param>
/// <param name="numVertices">The number vertices.</param>
/// <param name="numEdges">The number edges.</param>
/// <returns></returns>
vector<int> ShiloachVishkinUpdt(std::pair<int, int> *graph, const int numVertices, const int numEdges)
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
				{
					const int fromVertex = graph[i].first;
					const int toVertex = graph[i].second;


					if (fromVertex < toVertex && toVertex == component[toVertex])
					{
						hasGrafted = true;
						component[toVertex] = fromVertex;
					}
				}


				{
					const int fromVertex = graph[i].second;
					const int toVertex = graph[i].first;


					if (fromVertex < toVertex && toVertex == component[toVertex])
					{
						hasGrafted = true;
						component[toVertex] = fromVertex;
					}
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
#pragma omp for
			for (int i = 0; i < numEdges; ++i)
			{
				graph[i].first = component[graph[i].first];
				graph[i].second = component[graph[i].second];
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
	typedef adjacency_list<vecS, vecS, undirectedS> BoostGraph;


	vector<int> co1;
	vector<int> co2;
	vector<int> co3;


	do
	{
		BoostGraph boostGraph = GenerateRandomGraphBoost(4000000, 0.0000015);
		StdGraph stdGraph;
		StdGraph stdGraphBi;



		const std::pair<int, int> numVertEdg = FromBoostToStdGraph(boostGraph, &stdGraph);
		const std::pair<int, int> numVertEdgBi = FromBoostToStdGraph(boostGraph, &stdGraphBi);


		/*	for (auto it = stdGraph.begin(); it != stdGraph.end(); ++it)
			{
				cout << it->first << "," << it->second << endl;
			}*/

		const int numVerticesBoost = num_vertices(boostGraph);
		const int numEdgesBoost = num_edges(boostGraph);

		const int numVerticesStd = numVertEdg.first;
		const int numEdgesStd = numVertEdg.second;

		const int numVerticesStdBi = numVertEdgBi.first;
		const int numEdgesStdBi = numVertEdgBi.second;


		cout << "numVerti STD  = " << numVerticesStd << endl;
		cout << "numEdges STD  = " << numEdgesStd << endl;

		cout << "numVerti BOOST = " << numVerticesBoost << endl;
		cout << "numEdges BOOST = " << numEdgesBoost << endl;


		const double boostTime = omp_get_wtime();
		co1 = BoostConnectedComponent(boostGraph);
		const double boostTimeEnd = omp_get_wtime() - boostTime;

		const double parTime = omp_get_wtime();
		co3 = ShiloachVishkin(&stdGraph[0], numVerticesStd, numEdgesStd);
		const double parTimeEnd = omp_get_wtime() - parTime;

		const double parUpdTime = omp_get_wtime();
		co2 = ShiloachVishkinUpdt(&stdGraphBi[0], numVerticesStdBi, numEdgesStdBi);
		const double parUpdTimeEnd = omp_get_wtime() - parUpdTime;

		cout << "boost time			= " << boostTimeEnd << endl;
		cout << "paral time			= " << parTimeEnd << endl;
		cout << "paralIpdt time			= " << parUpdTimeEnd << endl;



	} while (co1 == co2 && co2 == co3);

	cout << "boost     = ";
	PrintVector<int>(co1);
	cout << "paral     = ";
	PrintVector<int>(co3);
	cout << "paralUpdt = ";
	PrintVector<int>(co2);


	return 0;
}

