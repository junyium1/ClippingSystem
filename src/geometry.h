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

struct Segment
{
    Point2D a;
    Point2D b;
};

enum class FillRule
{
    EvenOdd,
    NonZero
};

// Vecteur allant de p2 vers p1.
Vector2D subtract(const Point2D& p1, const Point2D& p2);

// Produit scalaire de deux vecteurs.
float dotProduct(const Vector2D& v1, const Vector2D& v2);

// Distance entre deux points.
float GetDistance(Point2D a, Point2D b);

// Teste si un point est a l'interieur d'un polygone.
bool IsPointInPolygon(Point2D p, const Polygon& poly);

// Distance entre un point et un segment [v, w].
float DistancePointToSegment(Point2D p, Point2D v, Point2D w);

// Calcule le centre d'un polygone.
Point2D PolygonCentroid(const Polygon& poly);

// Construit un polygone regulier a "sides" sommets.
Polygon MakeRegularPolygon(Point2D center, float radius, int sides);

// Translate un polygone.
Polygon TranslatePolygon(const Polygon& poly, float dx, float dy);

// Tourne un polygone autour de son centre.
Polygon RotatePolygon(const Polygon& poly, float angleRad);

// Change l'echelle d'un polygone autour de son centre.
Polygon ScalePolygon(const Polygon& poly, float sx, float sy);

// Applique un cisaillement a un polygone autour de son centre.
Polygon ShearPolygon(const Polygon& poly, float shx, float shy);

// Teste si un polygone est convexe.
bool IsConvexPolygon(const Polygon& poly);

// Teste si un point est a l'interieur d'un triangle.
bool PointInTriangle(Point2D p, Point2D a, Point2D b, Point2D c);

// Decoupe un polygone en triangles.
std::vector<Polygon> TriangulatePolygon(const Polygon& poly);
