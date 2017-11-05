#include "stdafx.h"
#include "Utils.h"
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/random/linear_congruential.hpp>
#include <fstream>
#include <ctime>


using namespace std;
using namespace boost;


adjacency_list<vecS, vecS, undirectedS> GenerateRandomGraphBoost(const int numVertices, const float prob)
{
	typedef adjacency_list<vecS, vecS, undirectedS> Graph;
	typedef erdos_renyi_iterator<boost::minstd_rand, Graph> ERGen;

	// random generator
	boost::minstd_rand gen;
	gen.seed(time(nullptr));
	Graph g(ERGen(gen, numVertices, prob), ERGen(), numVertices);
	return g;
}



void GenerateRandomGraphToFile(const std::string filename, const int numVertices, const float prob)
{
	typedef adjacency_list<vecS, vecS, undirectedS> Graph;
	typedef erdos_renyi_iterator<boost::minstd_rand, Graph> ERGen;

	// random generator
	boost::minstd_rand gen;
	gen.seed(time(nullptr));
	Graph g(ERGen(gen, numVertices, prob), ERGen(), numVertices);

	graph_traits<Graph>::edge_iterator ei, ei_end;


	std::ofstream outFile;
	outFile.open(filename);
	// print num vertives
	outFile << numVertices << endl;
	// print num edges
	outFile << num_edges(g) << endl;

	for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
	{
		outFile << source(*ei, g) << " " << target(*ei, g) << endl;
	}

}

std::pair<int, int> ReadGraphFromFile(const std::string filename, std::vector<std::pair<int, int>>* buffer)
{
	string line;
	ifstream myfile(filename);
	if (myfile.is_open())
	{
		getline(myfile, line);
		int numVertices = stoi(line);
		getline(myfile, line);
		int numEdges = stoi(line);

		while (getline(myfile, line))
		{

			auto split = line.find(" ");
			int from = stoi(line.substr(0, split));
			int to = stoi(line.substr(split, line.size()));
			buffer->push_back(std::pair<int, int>{from, to});

		}
		myfile.close();
		return std::pair<int, int>{numVertices, numEdges};
	}

	return std::pair<int, int>{0, 0};
}


std::pair<int, int> FromBoostToStdGraph(adjacency_list<vecS, vecS, undirectedS> &boostGraph, std::vector<std::pair<int, int>>* buffer)
{
	typedef adjacency_list<vecS, vecS, undirectedS> Graph;

	buffer->clear();
	buffer->reserve(num_edges(boostGraph));


	std::pair<int, int> numVerticesAndEdges{ num_vertices(boostGraph), num_edges(boostGraph) };

	graph_traits<Graph>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(boostGraph); ei != ei_end; ++ei)
	{
		buffer->push_back(std::pair<int, int>{source(*ei, boostGraph), target(*ei, boostGraph)});
	}

	return numVerticesAndEdges;

}

std::pair<int, int> FromBoostToStdGraphBi(adjacency_list<vecS, vecS, undirectedS>& boostGraph, std::vector<std::pair<int, int>>* buffer)
{

	typedef adjacency_list<vecS, vecS, undirectedS> Graph;

	buffer->clear();
	buffer->reserve(2 * num_edges(boostGraph));

	std::pair<int, int> numVerticesAndEdges{ num_vertices(boostGraph),2 * num_edges(boostGraph) };

	graph_traits<Graph>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(boostGraph); ei != ei_end; ++ei)
	{
		buffer->push_back(std::pair<int, int>{source(*ei, boostGraph), target(*ei, boostGraph)});
		buffer->push_back(std::pair<int, int> {target(*ei, boostGraph), source(*ei, boostGraph)});
	}

	return numVerticesAndEdges;
}
