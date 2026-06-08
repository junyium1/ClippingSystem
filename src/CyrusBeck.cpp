#include "CyrusBeck.h"
#include "geometry.h"
#include <limits>

std::optional<std::pair<Point2D, Point2D>> CyrusBeckClipper::clipLine(
    Point2D p0,
    Point2D p1,
    const Polygon& clipPolygon)
{
  //1
  float tinf = std::numeric_limits<float>::lowest();
  float tsup = std::numeric_limits<float>::max();
  Vector2D D = subtract(p1, p0);
  int nbsom = clipPolygon.vertices.size();

  //2
  for (int i = 0; i < nbsom; i++)
  {
    Point2D currentVertex = clipPolygon.vertices[i];
    Point2D nextVertex = clipPolygon.vertices[(i + 1) % nbsom];

    Vector2D edge = subtract(nextVertex, currentVertex);
    Vector2D normale = {-edge.y, edge.x};
    Vector2D w = subtract(p0, currentVertex);
    float dn = dotProduct(D, normale);
    float wn = dotProduct(w, normale);

    if (dn == 0.0f)
    {
      if (wn < 0.0f)
      {
        return std::nullopt;
      }
    }
    else
    {
      float t = -wn / dn;

      if (dn > 0.0f)
      {
        if (t > tinf)
        {
          tinf = t;
        }
      }
      else
      {
        if (t < tsup)
        {
          tsup = t;
        }
      }
    }
  }

  //3
  if (tinf < tsup)
  {
    if (tinf < 0.0f && tsup > 1.0f)
    {
      return std::make_pair(p0, p1);
    }
    else if (tinf > 1.0f || tsup < 0.0f)
    {
      return std::nullopt;
    }
    else
    {
      if (tinf < 0.0f) tinf = 0.0f;
      if (tsup > 1.0f) tsup = 1.0f;

      Point2D p0_clipped = { p0.x + D.x * tinf, p0.y + D.y * tinf };
      Point2D p1_clipped = { p0.x + D.x * tsup, p0.y + D.y * tsup };

      return std::make_pair(p0_clipped, p1_clipped);
    }
  }

  return std::nullopt;
}