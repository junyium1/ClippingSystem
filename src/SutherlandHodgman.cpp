#include "SutherlandHodgman.h"

// Decoupe un polygone par chaque arete de la fenetre, l'une apres l'autre.
Polygon SutherlandHodgmanClipper::clipPolygon(const Polygon& subjectPolygon, const Polygon& clipPolygon)
{
    Polygon outputPolygon = subjectPolygon;
    int numClipVertices = (int)clipPolygon.vertices.size();

    for (int i = 0; i < numClipVertices; i++)
    {
        Point2D edgeStart = clipPolygon.vertices[i];
        Point2D edgeEnd = clipPolygon.vertices[(i + 1) % numClipVertices];

        outputPolygon = clipAgainstEdge(outputPolygon, edgeStart, edgeEnd);

        if (outputPolygon.vertices.empty())
            break;
    }

    return outputPolygon;
}

// Garde uniquement la partie du polygone du bon cote de l'arete.
Polygon SutherlandHodgmanClipper::clipAgainstEdge(const Polygon& polygon, const Point2D& edgeStart, const Point2D& edgeEnd)
{
    Polygon outputPolygon;

    if (polygon.vertices.empty())
        return outputPolygon;

    Point2D S = polygon.vertices.back();

    for (size_t j = 0; j < polygon.vertices.size(); j++)
    {
        Point2D Pj = polygon.vertices[j];

        bool sInside = isInside(S, edgeStart, edgeEnd);
        bool PjInside = isInside(Pj, edgeStart, edgeEnd);

        if (PjInside)
        {
            if (!sInside)
                outputPolygon.vertices.push_back(computeIntersection(S, Pj, edgeStart, edgeEnd));

            outputPolygon.vertices.push_back(Pj);
        }
        else if (sInside)
        {
            outputPolygon.vertices.push_back(computeIntersection(S, Pj, edgeStart, edgeEnd));
        }

        S = Pj;
    }

    return outputPolygon;
}

// Teste si un point est a l'interieur de l'arete (a gauche).
bool SutherlandHodgmanClipper::isInside(const Point2D& pt, const Point2D& edgeStart, const Point2D& edgeEnd)
{
    Vector2D edgeDir = subtract(edgeEnd, edgeStart);
    Vector2D normale = {-edgeDir.y, edgeDir.x};

    Vector2D v = subtract(pt, edgeStart);
    return dotProduct(v, normale) >= 0.0f;
}

// Calcule le point d'intersection entre le segment [s, e] et l'arete.
Point2D SutherlandHodgmanClipper::computeIntersection(const Point2D& s, const Point2D& e, const Point2D& edgeStart, const Point2D& edgeEnd)
{
    Vector2D edgeDir = subtract(edgeEnd, edgeStart);
    Vector2D normale = {-edgeDir.y, edgeDir.x};

    Vector2D D = subtract(e, s);
    float num = dotProduct(subtract(edgeStart, s), normale);
    float den = dotProduct(D, normale);
    float t = (den == 0.0f) ? 0.0f : num / den;

    return {s.x + t * D.x, s.y + t * D.y};
}
