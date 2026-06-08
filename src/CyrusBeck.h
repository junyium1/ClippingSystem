// CyrusBeck.h
#pragma once
#include "geometry.h"
#include <optional>
#include <utility>

class CyrusBeckClipper
{
public:
  // Takes the starting and ending points of a line segment, and a convex clipping polygon.
  // Returns the clipped start and end points, or std::nullopt if the line is completely outside.
  static std::optional<std::pair<Point2D, Point2D>> clipLine(
      Point2D p0,
      Point2D p1,
      const Polygon& clipPolygon
  );
};