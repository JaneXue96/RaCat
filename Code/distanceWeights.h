#ifndef DISTANCEWEIGHTS_H_INCLUDED
#define DISTANCEWEIGHTS_H_INCLUDED

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
using namespace std;

float calculateManhattanNorm2D(int directionX, int directionY, std::vector<double> spacing);
float calculateEuclidianNorm2D(int directionX, int directionY, std::vector<double> spacing);
float calculateManhattanNorm3D(int directionX, int directionY, int directionZ, std::vector<double> spacing);
float calculateEuclidianNorm3D(int directionX, int directionY, int directionZ, std::vector<double> spacing);
#include "distanceWeights.cpp"

#endif