// SutherlandHodgman.h
#pragma once
#include "Geometry.h"

class SutherlandHodgmanClipper
{
public:
  // Takes a subject polygon and clips it against a clip polygon.
  // Returns the newly formed clipped polygon.
  static Polygon clipPolygon(
      const Polygon& subjectPolygon,
      const Polygon& clipPolygon
  );

private:
  // Helper function: Clips a polygon against a single infinite edge
  static Polygon clipAgainstEdge(
      const Polygon& poly,
      const Point2D& edgeStart,
      const Point2D& edgeEnd
  );

  // Helper function: Determines if a point is "inside" an edge
  static bool isInside(
      const Point2D& pt,
      const Point2D& edgeStart,
      const Point2D& edgeEnd
  );

  // Helper function: Finds intersection of a line segment and an edge
  static Point2D computeIntersection(
      const Point2D& s, const Point2D& e,
      const Point2D& edgeStart, const Point2D& edgeEnd
  );
};