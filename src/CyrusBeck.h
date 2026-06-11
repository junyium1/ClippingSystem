#pragma once
#include "geometry.h"

class CyrusBeckClipper
{
public:
    // Decoupe un segment par un polygone convexe.
    static bool clipLine(Point2D p0, Point2D p1, const Polygon& clipPolygon, Segment& result);

    // Decoupe un polygone par un polygone convexe.
    static Polygon clipPolygon(const Polygon& subjectPolygon, const Polygon& clipPolygon);
};
