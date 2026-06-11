#pragma once
#include "geometry.h"

enum class FractalType
{
    KochSnowflake,
    KochAntiSnowflake,
    KochSquareIsland,
    KochSquareCross
};

// Genere une courbe fractale fermee de type Koch.
Polygon GenerateFractal(FractalType type, Point2D center, float radius, int iterations);
