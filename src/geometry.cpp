#include "geometry.h"

Vector2D subtract(const Point2D& p1, const Point2D& p2)
{
  Vector2D result;
  result.x = p1.x - p2.x;
  result.y = p1.y - p2.y;
  return result;
}

float dotProduct(const Vector2D& v1, const Vector2D& v2)
{
  return (v1.x * v2.x) + (v1.y * v2.y);
}