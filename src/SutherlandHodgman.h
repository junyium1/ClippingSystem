#pragma once
#include "geometry.h"

class SutherlandHodgmanClipper
{
public:
    // Decoupe un polygone par un polygone convexe.
    static Polygon clipPolygon(const Polygon& subjectPolygon, const Polygon& clipPolygon);

private:
    // Decoupe un polygone par une seule arete de la fenetre.
    static Polygon clipAgainstEdge(const Polygon& poly, const Point2D& edgeStart, const Point2D& edgeEnd);

    // Teste si un point est du bon cote d'une arete.
    static bool isInside(const Point2D& pt, const Point2D& edgeStart, const Point2D& edgeEnd);

    // Calcule le point d'intersection entre [s, e] et l'arete.
    static Point2D computeIntersection(const Point2D& s, const Point2D& e, const Point2D& edgeStart, const Point2D& edgeEnd);
};
