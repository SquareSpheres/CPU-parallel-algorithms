#include <iostream>
#include <vector>
#include <omp.h>
#include <algorithm>
#include <time.h>
#include <math.h>



struct Point {

	double x, y;

	bool operator <(const Point &p) const {
		return x < p.x || (x == p.x && y < p.y);
	}

	bool operator >(const Point &p) const {
		return x > p.x || (x == p.x && y > p.y);
	}

	bool operator ==(const Point &p) const {
		return x == p.x && y == p.y;
	}

	bool operator !=(const Point &p) const {
		return x != p.x || y != p.y;
	}

	friend std::ostream& operator<< (std::ostream& stream, const Point& p) {
		return stream << p.x << "," << p.y;
	}

};


typedef std::pair<int, int> IndexPair;


/**
*	Counterclockwise turns
*
*	counterclockwise if ccw > 0
*	clockwise if ccw < 0
*	colinear if ccw == 0
**/
double CCW(Point a, Point b, Point c);



/**
*	Source : http://geomalgorithms.com/a15-_tangents.html
*
*	Finds the Right-Left tangent from A to B
**/
IndexPair RLTangentLine(std::vector<Point> * polygonA, int rightMostA, std::vector<Point> * polygonB);


/**
*	Source : http://geomalgorithms.com/a15-_tangents.html
*
*	Finds the Left-Right tangent from A to B
**/
IndexPair LRTangentLine(std::vector<Point> * polygonA, int rightMostA, std::vector<Point> * polygonB);


/**
*	Parallel algorithm for finding the convex hull from a set of points
*
*	points						: array of points
*	numPoints					: size of array
*
*	returns						: vector of points representing the convex hull
*
**/
std::vector<Point>* ParallelConvexHull(Point * const points, const int numPoints);

/**
*	Merge two convex hulls into one
*
*	polygonA					: first convex hull
*	polygonB					: second convex hull
*	rightMostA				: index of rightmost point in A. Will be replaced by rightmostw in new hull.
*
*	returns						: vector of points representing the merged convex hull
*
**/
std::vector<Point>* mergeHulls(std::vector<Point>* const polygonA, std::vector<Point>* const polygonB, int * rightMostA);

/**
*
*	Source : https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
*
*	Sequntial algorithm for finding the convex hull from a set of points.
*
*	points						: array of points
*	numPoints					: size of array
*	rightMostIndex		: will contain rightmost index when function returns
*
*	returns						: vector of points representing the convex hull
*
**/
std::vector<Point> * Monotone_Chain_Convex_Hull(Point * const points, const int numPoints, int * rightMostIndex);


/**
*	Partition points into segments so that each segment does not have points with equal x-values.
*	It does not guarantee that all processors will be utilized. Number of processors utilized
*	will be equal to the size of the vector returned.
*
*	points					: array of points
*	numPoints				: size of array
*	numProcs				: how many processors are available
*
*	returns					: vector with start and end index for processors
*
**/
std::vector<std::pair<int, int>> dividePoints(Point * const points, const int numPoints, const int numProcs);


void ConcurrentMemoryAllocationTest();


void printPointVector(std::vector<Point> * vec) {

	for (std::vector<Point>::iterator it = vec->begin(); it != vec->end(); ++it) {
		std::cout << (*it).x << "," << (*it).y << std::endl;
	}

}


int main()
{

	std::cout << "=============TEST START=============== " << std::endl;

	srand(time(NULL));

	const int TEST_SIZE = 30000000;
	Point * TEST_SET = new Point[TEST_SIZE];

	for (size_t i = 0; i < TEST_SIZE; i++)
	{

		double x = (double)rand() / RAND_MAX;
		double y = (double)rand() / RAND_MAX;

		TEST_SET[i] = { x, y };
	}

	std::sort(TEST_SET, TEST_SET + TEST_SIZE);

	int a;

	double parTimeStart = omp_get_wtime();
	std::vector<Point> * PAR = ParallelConvexHull(TEST_SET, TEST_SIZE);
	double parTimeEnd = omp_get_wtime() - parTimeStart;

	std::cout << "Parallel time = " << parTimeEnd << std::endl;

	double timeStart = omp_get_wtime();
	std::vector<Point> * SEQ = Monotone_Chain_Convex_Hull(TEST_SET, TEST_SIZE, &a);
	double timeEnd = omp_get_wtime() - timeStart;

	std::cout << "Sequential time = " << timeEnd << std::endl;


	std::cout << "(SEQ == PAR) = " << (*SEQ == *PAR) << std::endl;

	std::cout << "=============TEST END================= " << std::endl;


	delete[] TEST_SET;
	delete PAR;
	delete SEQ;



#ifdef _WIN32
	int wait;
	std::cin >> wait;
#endif

	return 0;
}


double CCW(Point a, Point b, Point c)
{
	return (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
}

IndexPair RLTangentLine(std::vector<Point> * polygonA, int rightMostA, std::vector<Point> * polygonB)
{
	int ix1 = rightMostA;
	int ix2 = 0;
	int ix1Max = polygonA->size();
	int ix2Max = polygonB->size();

	int done = false;
	while (done == false) {
		done = true;

		int ix1Rot = (ix1 + 1) % ix1Max;

		while (CCW(polygonB->at(ix2), polygonA->at(ix1), polygonA->at(ix1Rot)) <= 0) {
			ix1 = (ix1 + 1) % ix1Max;
			ix1Rot = (ix1 + 1) % ix1Max;

		}

		int ix2Rot = ix2 - 1;
		if (ix2Rot < 0)
			ix2Rot = ix2Max - 1;

		while (CCW(polygonA->at(ix1), polygonB->at(ix2), polygonB->at(ix2Rot)) >= 0) {

			--ix2;
			if (ix2 < 0)
				ix2 = ix2Max - 1;

			--ix2Rot;
			if (ix2Rot < 0)
				ix2Rot = ix2Max - 1;


			done = false;
		}

	}

	return IndexPair(ix1, ix2);
}

IndexPair LRTangentLine(std::vector<Point>* polygonA, int rightMostA, std::vector<Point>* polygonB)
{

	int ix1 = rightMostA;
	int ix2 = 0;
	int ix1Max = polygonA->size();
	int ix2Max = polygonB->size();

	int done = false;
	while (done == false) {
		done = true;

		int ix1Rot = ix1 - 1;
		if (ix1Rot < 0)
			ix1Rot = ix1Max - 1;

		while (CCW(polygonB->at(ix2), polygonA->at(ix1), polygonA->at(ix1Rot)) >= 0) {

			ix1--;
			if (ix1 < 0)
				ix1 = ix1Max - 1;
			ix1Rot--;
			if (ix1Rot < 0)
				ix1Rot = ix1Max - 1;
		}

		int ix2Rot = (ix2 + 1) % ix2Max;

		while (CCW(polygonA->at(ix1), polygonB->at(ix2), polygonB->at(ix2Rot)) <= 0) {
			ix2 = (ix2 + 1) % ix2Max;
			ix2Rot = (ix2 + 1) % ix2Max;

			done = false;

		}
	}



	return IndexPair(ix1, ix2);
}

std::vector<Point>* ParallelConvexHull(Point * const points, const int numPoints)
{

	std::vector<std::pair<int, int>> startEndVectices = dividePoints(points, numPoints, omp_get_num_procs());

	int numProcs = startEndVectices.size();

	std::vector<Point>	** localHulls = new std::vector<Point>*[numProcs];

	int * rightMostValues = new int[numProcs];


#pragma omp parallel num_threads(numProcs)
	{
		int pid = omp_get_thread_num();
		if (pid < numProcs) {

			std::pair<int, int> startEnd = startEndVectices.at(pid);
			int start = startEnd.first;
			int end = startEnd.second;

			int localRightMost=0;

			std::vector<Point> * localHull = Monotone_Chain_Convex_Hull(&points[start], end - start + 1, &localRightMost);

			rightMostValues[pid] = localRightMost;
			localHulls[pid] = localHull;


#pragma omp barrier

			for (size_t i = 0; i < (int)ceil(log2(numProcs)); i++)
			{

				if (pid % (int)pow(2, i + 1) == 0) {

					int mergeWith = pid + (int)pow(2, i);

					if (mergeWith < numProcs) {



						std::vector<Point> * newHull = mergeHulls(localHulls[pid], localHulls[mergeWith], &rightMostValues[pid]);


						delete localHulls[pid];
						delete localHulls[mergeWith];

						localHulls[pid] = newHull;
					}
				}

#pragma omp barrier

			}

		}
	}




	return localHulls[0];
}


std::vector<Point>* mergeHulls(std::vector<Point>* const polygonA, std::vector<Point>* const polygonB, int * rightMostA)
{

	IndexPair upper = RLTangentLine(polygonA, *rightMostA, polygonB);
	IndexPair lower = LRTangentLine(polygonA, *rightMostA, polygonB);


	std::vector<Point> * newHull = new std::vector<Point>(2 * (polygonA->size() + polygonB->size()));

	std::vector<Point>::iterator itA = polygonA->begin();

	int newHullIndex = 0;


	while ((*itA) != (*polygonA)[lower.first]) {
		(*newHull)[newHullIndex++] = (*itA++);
	}

	(*newHull)[newHullIndex++] = ((*polygonA)[lower.first]);


	std::vector<Point>::iterator itB = polygonB->begin() + lower.second;

	while ((*itB) != (*polygonB)[upper.second]) {
		(*newHull)[newHullIndex++] = (*itB++);
	}

	(*newHull)[newHullIndex++] = ((*polygonB)[upper.second]);

	itA = polygonA->begin() + upper.first;
	while ((*itA) != (*polygonA)[polygonA->size() - 1]) {
		(*newHull)[newHullIndex++] = (*itA++);
	}

	(*newHull)[newHullIndex++] = ((*polygonA)[polygonA->size() - 1]);


	newHull->resize(newHullIndex);

	int max_x_index = 0;
	for (size_t i = 0; i < newHull->size(); i++)
	{
		if ((*newHull)[i].x > (*newHull)[max_x_index].x)
			max_x_index = i;
	}


	*rightMostA = max_x_index;


	return newHull;

}

std::vector<std::pair<int, int>> dividePoints(Point * const points, const int numPoints, const int numProcs) {



	std::vector<std::pair<int, int>> startEndIndecies;


	if (numProcs == 1) {
		startEndIndecies.push_back(std::pair<int, int>({ 0,numPoints - 1 }));
		return startEndIndecies;
	}

	int numPointsPerProc = numPoints / numProcs;


	int procCount = 0;
	int start = 0;
	int end = start + (numPointsPerProc - 1);


	while (true) {

		if (end >= numPoints) {
			end = numPoints - 1;
			startEndIndecies.push_back(std::pair<int, int>(start, end));
			break;
		}


		if (procCount > 1) {
			while (points[start].x == points[start - 1].x) {
				start++;
				if (start >= numPoints)
					break;
			}
		}

		while (points[end].x == points[end + 1].x) {
			end++;
			if (end >= numPoints)
				break;
		}

		procCount++;

		startEndIndecies.push_back(std::pair<int, int>(start, end));

		start = end + 1;

		numPointsPerProc = (numPoints - end) / (numProcs - procCount);

		end = start + (numPointsPerProc - 1);

	}

	return startEndIndecies;

}

std::vector<Point> * Monotone_Chain_Convex_Hull(Point * const points, const int numPoints, int * rightMostIndex)
{

	int k = 0;
	if (numPoints == 1) {
		std::vector<Point> *H = new std::vector<Point>;
		H->push_back(points[0]);
		return H;
	}

	Point *  H = new Point[numPoints];


	for (int i = 0; i < numPoints; ++i) {
		while (k >= 2 && CCW(H[k - 2], H[k - 1], points[i]) <= 0) k--;

		H[k++] = points[i];

	}

	*rightMostIndex = k - 1;

	for (int i = numPoints - 2, t = k + 1; i >= 0; i--) {
		while (k >= t && CCW(H[k - 2], H[k - 1], points[i]) <= 0) k--;

		H[k++] = points[i];
	}

	std::vector<Point> * convexHull = new std::vector<Point>(H, H + k);

	delete[] H;

	return convexHull;
}


void ConcurrentMemoryAllocationTest() {



#pragma omp parallel
	{
		double timeStart = omp_get_wtime();
		Point * p = (Point*)malloc(10000000 * sizeof(Point));

		Point * aux = (Point*)malloc(10000000 * sizeof(Point));
		for (size_t i = 0; i < 10000000; i++)
		{
			aux[i] = p[i];
		}

		double endTime = omp_get_wtime() - timeStart;



		free(p);
		free(aux);


#pragma omp critical
		{
			std::cout << "Time = " << endTime << std::endl;
		}
	}

}
