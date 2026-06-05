// Geometry.h
#pragma once
#include <vector>

struct Point2D
{
  float x, y;
};

using Vector2D = Point2D;

struct Polygon
{
  std::vector<Point2D> vertices;
};

// Helper function declarations
Vector2D subtract(const Point2D& p1, const Point2D& p2);
float dotProduct(const Vector2D& v1, const Vector2D& v2);