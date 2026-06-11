#include "geometry.h"
#include <cmath>
#include <algorithm>

// Vecteur allant de p2 vers p1.
Vector2D subtract(const Point2D& p1, const Point2D& p2)
{
    Vector2D result;
    result.x = p1.x - p2.x;
    result.y = p1.y - p2.y;
    return result;
}

// Produit scalaire de deux vecteurs.
float dotProduct(const Vector2D& v1, const Vector2D& v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y);
}

// Distance entre deux points.
float GetDistance(Point2D a, Point2D b)
{
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

// Teste si un point est a l'interieur d'un polygone (ray casting).
bool IsPointInPolygon(Point2D p, const Polygon& poly)
{
    bool inside = false;
    for (size_t i = 0, j = poly.vertices.size() - 1; i < poly.vertices.size(); j = i++)
    {
        const Point2D& vi = poly.vertices[i];
        const Point2D& vj = poly.vertices[j];
        if (((vi.y > p.y) != (vj.y > p.y)) &&
            (p.x < (vj.x - vi.x) * (p.y - vi.y) / (vj.y - vi.y) + vi.x))
            inside = !inside;
    }
    return inside;
}

// Distance entre un point et un segment [v, w].
float DistancePointToSegment(Point2D p, Point2D v, Point2D w)
{
    float l2 = (w.x - v.x) * (w.x - v.x) + (w.y - v.y) * (w.y - v.y);
    if (l2 == 0.0f)
        return GetDistance(p, v);

    float t = std::max(0.0f, std::min(1.0f,
        ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2));

    Point2D proj = { v.x + t * (w.x - v.x), v.y + t * (w.y - v.y) };
    return GetDistance(p, proj);
}

// Calcule le centre (moyenne des sommets) d'un polygone.
Point2D PolygonCentroid(const Polygon& poly)
{
    Point2D c = {0.0f, 0.0f};
    if (poly.vertices.empty())
        return c;

    for (size_t i = 0; i < poly.vertices.size(); i++)
    {
        c.x += poly.vertices[i].x;
        c.y += poly.vertices[i].y;
    }

    c.x /= (float)poly.vertices.size();
    c.y /= (float)poly.vertices.size();
    return c;
}

// Construit un polygone regulier a "sides" sommets.
Polygon MakeRegularPolygon(Point2D center, float radius, int sides)
{
    const float kPi = 3.14159265358979f;

    Polygon poly;
    for (int i = 0; i < sides; i++)
    {
        float angle = -kPi / 2.0f + (float)i * (2.0f * kPi / (float)sides);
        Point2D v;
        v.x = center.x + radius * std::cos(angle);
        v.y = center.y + radius * std::sin(angle);
        poly.vertices.push_back(v);
    }
    return poly;
}

// Tourne un polygone autour de son centre.
Polygon RotatePolygon(const Polygon& poly, float angleRad)
{
    Point2D c = PolygonCentroid(poly);
    float ca = std::cos(angleRad);
    float sa = std::sin(angleRad);

    Polygon result;
    for (size_t i = 0; i < poly.vertices.size(); i++)
    {
        float dx = poly.vertices[i].x - c.x;
        float dy = poly.vertices[i].y - c.y;
        Point2D v;
        v.x = c.x + dx * ca - dy * sa;
        v.y = c.y + dx * sa + dy * ca;
        result.vertices.push_back(v);
    }
    return result;
}

// Change l'echelle d'un polygone autour de son centre.
Polygon ScalePolygon(const Polygon& poly, float sx, float sy)
{
    Point2D c = PolygonCentroid(poly);

    Polygon result;
    for (size_t i = 0; i < poly.vertices.size(); i++)
    {
        Point2D v;
        v.x = c.x + (poly.vertices[i].x - c.x) * sx;
        v.y = c.y + (poly.vertices[i].y - c.y) * sy;
        result.vertices.push_back(v);
    }
    return result;
}

// Translate un polygone.
Polygon TranslatePolygon(const Polygon& poly, float dx, float dy)
{
    Polygon result;
    for (size_t i = 0; i < poly.vertices.size(); i++)
    {
        Point2D v;
        v.x = poly.vertices[i].x + dx;
        v.y = poly.vertices[i].y + dy;
        result.vertices.push_back(v);
    }
    return result;
}

// Applique un cisaillement a un polygone autour de son centre.
Polygon ShearPolygon(const Polygon& poly, float shx, float shy)
{
    Point2D c = PolygonCentroid(poly);

    Polygon result;
    for (size_t i = 0; i < poly.vertices.size(); i++)
    {
        float dx = poly.vertices[i].x - c.x;
        float dy = poly.vertices[i].y - c.y;
        Point2D v;
        v.x = c.x + dx + shx * dy;
        v.y = c.y + shy * dx + dy;
        result.vertices.push_back(v);
    }
    return result;
}

// Produit vectoriel (en 2D) de deux vecteurs.
static float Cross(const Vector2D& a, const Vector2D& b)
{
    return a.x * b.y - a.y * b.x;
}

// Teste si un polygone est convexe.
bool IsConvexPolygon(const Polygon& poly)
{
    size_t n = poly.vertices.size();
    if (n < 4)
        return true;

    bool gotPositive = false;
    bool gotNegative = false;

    for (size_t i = 0; i < n; i++)
    {
        Point2D a = poly.vertices[i];
        Point2D b = poly.vertices[(i + 1) % n];
        Point2D c = poly.vertices[(i + 2) % n];

        float cross = Cross(subtract(b, a), subtract(c, b));
        if (cross > 0.0f)
            gotPositive = true;
        if (cross < 0.0f)
            gotNegative = true;
        if (gotPositive && gotNegative)
            return false;
    }
    return true;
}

// Teste si un point est a l'interieur d'un triangle.
bool PointInTriangle(Point2D p, Point2D a, Point2D b, Point2D c)
{
    float d1 = Cross(subtract(b, a), subtract(p, a));
    float d2 = Cross(subtract(c, b), subtract(p, b));
    float d3 = Cross(subtract(a, c), subtract(p, c));

    bool hasNeg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
    bool hasPos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);

    return !(hasNeg && hasPos);
}

// Decoupe un polygone en triangles (algorithme des oreilles).
std::vector<Polygon> TriangulatePolygon(const Polygon& poly)
{
    std::vector<Polygon> triangles;
    std::vector<Point2D> verts = poly.vertices;
    if (verts.size() < 3)
        return triangles;

    float signedArea = 0.0f;
    for (size_t i = 0; i < verts.size(); i++)
    {
        Point2D a = verts[i];
        Point2D b = verts[(i + 1) % verts.size()];
        signedArea += a.x * b.y - b.x * a.y;
    }
    bool ccw = signedArea > 0.0f;

    int guard = 0;
    while (verts.size() > 3 && guard++ < 10000)
    {
        size_t n = verts.size();
        bool earFound = false;

        for (size_t i = 0; i < n; i++)
        {
            size_t prev = (i + n - 1) % n;
            size_t next = (i + 1) % n;

            Point2D a = verts[prev];
            Point2D b = verts[i];
            Point2D c = verts[next];

            float cross = Cross(subtract(b, a), subtract(c, b));
            if (ccw ? (cross <= 0.0f) : (cross >= 0.0f))
                continue;

            bool anyInside = false;
            for (size_t j = 0; j < n; j++)
            {
                if (j == prev || j == i || j == next)
                    continue;
                if (PointInTriangle(verts[j], a, b, c))
                {
                    anyInside = true;
                    break;
                }
            }
            if (anyInside)
                continue;

            triangles.push_back({{a, b, c}});
            verts.erase(verts.begin() + i);
            earFound = true;
            break;
        }

        if (!earFound)
            break;
    }

    if (verts.size() == 3)
        triangles.push_back({{verts[0], verts[1], verts[2]}});

    return triangles;
}
