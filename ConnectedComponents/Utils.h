#pragma once

#include <vector>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace std;
using namespace boost;


void GenerateRandomGraphToFile(const std::string filename, const int numVertices, const float prob);
adjacency_list<vecS, vecS, undirectedS> GenerateRandomGraphBoost(const int numVertices, const float prob);
std::pair<int, int> ReadGraphFromFile(const std::string filename, std::vector<std::pair<int, int>>* buffer);
std::pair<int, int> FromBoostToStdGraph(adjacency_list<vecS, vecS, undirectedS> &boostGraph, std::vector<std::pair<int, int>>* buffer);
std::pair<int, int> FromBoostToStdGraphBi(adjacency_list<vecS, vecS, undirectedS> &boostGraph, std::vector<std::pair<int, int>>* buffer);


template <class T>
void PrintVector(std::vector<T> &vec)
{
	std::stringstream stream;
	for (int i = 0; i < vec.size(); ++i)
	{
		if (i == 0) stream << "[" << vec[i] <<",";
		else if (i == vec.size() - 1) stream << vec[i] << "]";
		else 	stream << vec[i] << ",";

	}

	std::cout << stream.str() << endl;
}