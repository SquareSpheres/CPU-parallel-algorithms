#include "stdafx.h"
#include "ShiloachVishkinAcc.h"
#include <unordered_map>


int* ShiloachVishkinAcc(undirected_edge* graph, const int numVertices, const int numEdges)
{
	int* component = new int[numVertices];

	for (int i = 0; i < numVertices; i++)
	{
		component[i] = i;
	}

	bool hasGrafted = true;

	while (hasGrafted)
	{
		hasGrafted = false;

#pragma acc kernels
		{

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

int* ShiloachVishkinUpdtAcc(undirected_edge* graph, const int numVertices, const int numEdges)
{
	int *  component = new int[numVertices];


	for (int i = 0; i < numVertices; i++)
	{
		component[i] = i;
	}

	bool hasGrafted = true;

	while (hasGrafted)
	{
		hasGrafted = false;


#pragma acc kernels 
		{


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

			for (int i = 0; i < numVertices; ++i)
			{
				while (component[i] != component[component[i]])
				{
					component[i] = component[component[i]];
				}
			}

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