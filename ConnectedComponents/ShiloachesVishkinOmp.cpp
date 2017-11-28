#include "ShiloachesVishkinOmp.h"

std::vector<int> ShiloachVishkinOmp(std::pair<int, int> *graph, const int numVertices, const int numEdges)
{
	std::vector<int> component(numVertices);

	/************** Component vector init *************/
#pragma omp parallel for
	for (int i = 0; i < numVertices; i++)
	{
		component[i] = i;
	}
	/************** Component vector init *************/



	bool hasGrafted = true;

	while (hasGrafted)
	{
		hasGrafted = false;

#pragma omp parallel
		{
			/************** Grafting *************/
#pragma omp for
			for (int i = 0; i < numEdges; ++i)
			{
				const int fromVertex = graph[i].first;
				const int toVertex = graph[i].second;

				int fromComponent = component[fromVertex];
				int toComponent = component[toVertex];

				if ((fromComponent < toComponent) && (toComponent == component[toComponent]))
				{
					if (!hasGrafted) hasGrafted = true;
					component[toComponent] = fromComponent;

				}

				fromComponent = component[toVertex];
				toComponent = component[fromVertex];

				if ((fromComponent < toComponent) && (toComponent == component[toComponent]))
				{
					if (!hasGrafted) hasGrafted = true;
					component[toComponent] = fromComponent;
				}

			}
			/************** Grafting *************/


			/************** Compressing *************/
#pragma omp for
			for (int i = 0; i < numVertices; ++i)
			{
				while (component[i] != component[component[i]])
				{
					component[i] = component[component[i]];
				}
			}
			/************** Compressing *************/
		}
	}

	return component;
}


std::vector<int> ShiloachVishkinUpdtOmp(std::pair<int, int> *graph, const int numVertices, const int numEdges)
{
	std::vector<int> component(numVertices);

	/************** Component vector init *************/
#pragma omp parallel for
	for (int i = 0; i < numVertices; i++)
	{
		component[i] = i;
	}
	/************** Component vector init *************/



	bool hasGrafted = true;

	while (hasGrafted)
	{
		hasGrafted = false;


#pragma omp parallel 
		{
			/************** Grafting *************/
#pragma omp for
			for (int i = 0; i < numEdges; ++i)
			{

				int fromVertex = graph[i].first;
				int toVertex = graph[i].second;


				if (fromVertex < toVertex && toVertex == component[toVertex])
				{
					if (!hasGrafted) hasGrafted = true;
					component[toVertex] = fromVertex;
				}

				const int temp = fromVertex;
				fromVertex = toVertex;
				toVertex = temp;

				if (fromVertex < toVertex && toVertex == component[toVertex])
				{
					if (!hasGrafted) hasGrafted = true;
					component[toVertex] = fromVertex;
				}

			}
			/************** Grafting *************/



			/************** Compressing *************/
#pragma omp for
			for (int i = 0; i < numVertices; ++i)
			{
				while (component[i] != component[component[i]])
				{
					component[i] = component[component[i]];
				}
			}
			/************** Compressing *************/


			/************** Updating *************/
#pragma omp for
			for (int i = 0; i < numEdges; ++i)
			{
				graph[i].first = component[graph[i].first];
				graph[i].second = component[graph[i].second];
			}
			/************** Updating *************/
		}
	}


	return component;
}
