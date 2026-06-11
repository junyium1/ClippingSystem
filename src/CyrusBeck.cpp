#include "CyrusBeck.h"

// Decoupe un segment [p0, p1] avec la methode parametrique de Cyrus-Beck.
bool CyrusBeckClipper::clipLine(Point2D p0, Point2D p1, const Polygon& clipPolygon, Segment& result)
{
    float tEnter = -1000000.0f;
    float tExit = 1000000.0f;
    Vector2D D = subtract(p1, p0);
    int nbsom = (int)clipPolygon.vertices.size();

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
                return false;
        }
        else
        {
            float t = -wn / dn;

            if (dn > 0.0f)
            {
                if (t > tEnter)
                    tEnter = t;
            }
            else
            {
                if (t < tExit)
                    tExit = t;
            }
        }
    }

    if (tEnter >= tExit)
        return false;

    if (tEnter < 0.0f && tExit > 1.0f)
    {
        result.a = p0;
        result.b = p1;
        return true;
    }

    if (tEnter > 1.0f || tExit < 0.0f)
        return false;

    if (tEnter < 0.0f)
        tEnter = 0.0f;
    if (tExit > 1.0f)
        tExit = 1.0f;

    result.a = { p0.x + D.x * tEnter, p0.y + D.y * tEnter };
    result.b = { p0.x + D.x * tExit, p0.y + D.y * tExit };
    return true;
}

// Decoupe un polygone par un polygone convexe (algorithme de Cyrus-Beck).
Polygon CyrusBeckClipper::clipPolygon(const Polygon& subjectPolygon, const Polygon& clipPolygon)
{
    Polygon output = subjectPolygon;
    size_t nClip = clipPolygon.vertices.size();

    for (size_t i = 0; i < nClip && !output.vertices.empty(); i++)
    {
        Point2D edgeStart = clipPolygon.vertices[i];
        Point2D edgeEnd = clipPolygon.vertices[(i + 1) % nClip];

        Vector2D edge = subtract(edgeEnd, edgeStart);
        Vector2D normale = {-edge.y, edge.x};

        Polygon input = output;
        output.vertices.clear();

        Point2D S = input.vertices.back();
        Vector2D W = subtract(S, edgeStart);
        float sWN = dotProduct(W, normale);

        for (size_t j = 0; j < input.vertices.size(); j++)
        {
            Point2D Pj = input.vertices[j];
            Vector2D Wj = subtract(Pj, edgeStart);
            float pjWN = dotProduct(Wj, normale);

            bool sInside = sWN >= 0.0f;
            bool pjInside = pjWN >= 0.0f;

            if (pjInside)
            {
                if (!sInside)
                {
                    Vector2D D = subtract(Pj, S);
                    float dn = dotProduct(D, normale);
                    float t = -sWN / dn;
                    output.vertices.push_back({ S.x + t * D.x, S.y + t * D.y });
                }
                output.vertices.push_back(Pj);
            }
            else if (sInside)
            {
                Vector2D D = subtract(Pj, S);
                float dn = dotProduct(D, normale);
                float t = -sWN / dn;
                output.vertices.push_back({ S.x + t * D.x, S.y + t * D.y });
            }

            S = Pj;
            sWN = pjWN;
        }
    }

    if (output.vertices.size() < 3)
        return {};

    return output;
}
