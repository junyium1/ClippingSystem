#include "fractal.h"
#include <cmath>

// Remplace une arete par 4 aretes en forme de pic (recursif).
static void KochEdge(Point2D a, Point2D b, int depth, float angle, std::vector<Point2D>& out)
{
    if (depth <= 0)
    {
        out.push_back(a);
        return;
    }

    Point2D p1 = { a.x + (b.x - a.x) / 3.0f, a.y + (b.y - a.y) / 3.0f };
    Point2D p2 = { a.x + 2.0f * (b.x - a.x) / 3.0f, a.y + 2.0f * (b.y - a.y) / 3.0f };

    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    Point2D apex;
    apex.x = p1.x + dx * std::cos(angle) - dy * std::sin(angle);
    apex.y = p1.y + dx * std::sin(angle) + dy * std::cos(angle);

    KochEdge(a, p1, depth - 1, angle, out);
    KochEdge(p1, apex, depth - 1, angle, out);
    KochEdge(apex, p2, depth - 1, angle, out);
    KochEdge(p2, b, depth - 1, angle, out);
}

// Applique KochEdge sur chaque arete d'un polygone de base.
static Polygon BuildKochFromBase(const std::vector<Point2D>& base, int iterations, float angle)
{
    Polygon result;
    size_t n = base.size();
    for (size_t i = 0; i < n; i++)
        KochEdge(base[i], base[(i + 1) % n], iterations, angle, result.vertices);
    return result;
}

// Construit une fractale de Koch a partir d'un triangle ou d'un carre.
Polygon GenerateFractal(FractalType type, Point2D center, float radius, int iterations)
{
    const float kPi = 3.14159265358979f;
    const float kOutward = -kPi / 3.0f;
    const float kInward = kPi / 3.0f;

    switch (type)
    {
        case FractalType::KochSnowflake:
        case FractalType::KochAntiSnowflake:
        {
            std::vector<Point2D> triangle = {
                { center.x, center.y - radius },
                { center.x - radius * 0.8660254f, center.y + radius * 0.5f },
                { center.x + radius * 0.8660254f, center.y + radius * 0.5f }
            };
            float angle = (type == FractalType::KochSnowflake) ? kOutward : kInward;
            return BuildKochFromBase(triangle, iterations, angle);
        }

        case FractalType::KochSquareIsland:
        case FractalType::KochSquareCross:
        {
            std::vector<Point2D> square = {
                { center.x - radius, center.y - radius },
                { center.x - radius, center.y + radius },
                { center.x + radius, center.y + radius },
                { center.x + radius, center.y - radius }
            };
            float angle = (type == FractalType::KochSquareIsland) ? kOutward : kInward;
            return BuildKochFromBase(square, iterations, angle);
        }
    }

    return {};
}
