#pragma once
#include <utility>
#include <vector>


typedef std::pair<int, int> undirected_edge;


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
std::vector<int> ShiloachVishkinUpdtOmp(std::pair<int, int> *graph, const int numVertices, const int numEdges);

/// <summary>
/// Parallel Shiloaches-vishkin algorithm. The graph is an array of std::pair<int,int>, where each pair represents a edge from a vertex to antoher.
/// E.g. std::pair{0,3} represents an edge from vertex 0 to vertex 3. The algorithm returns a vector with size |numVertices|, where each index 
/// represents what component that vertex belongs to. E.g. component[7] will give you vertex 7s component.
/// </summary>
/// <param name="graph">An array std::pair</param>
/// <param name="numVertices">The number vertices.</param>
/// <param name="numEdges">The number edges.</param>
/// <returns></returns>
std::vector<int> ShiloachVishkinOmp(std::pair<int, int> *graph, const int numVertices, const int numEdges);

