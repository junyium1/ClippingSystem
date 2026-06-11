#include "rasterizer.h"
#include <cmath>
#include <vector>
#include <algorithm>

// Une arete du polygone, avec sa pente inverse pour le balayage.
struct Edge
{
    float yMin;
    float yMax;
    float xAtYMin;
    float invSlope;
    int dir;
};

// Une arete active pour la ligne de balayage courante.
struct ActiveEdge
{
    float yMax;
    float x;
    float invSlope;
    int dir;
};

// Compare deux aretes par leur y minimum.
static bool CompareEdgesByYMin(const Edge& a, const Edge& b)
{
    return a.yMin < b.yMin;
}

// Compare deux aretes actives par leur abscisse.
static bool CompareActiveEdgesByX(const ActiveEdge& a, const ActiveEdge& b)
{
    return a.x < b.x;
}

// Remplit une portion horizontale entre deux abscisses.
static void FillSpan(Framebuffer& fb, int y, float xa, float xb, uint32_t color)
{
    int xStart = std::max(0, (int)std::round(xa));
    int xEnd = std::min(fb.width - 1, (int)std::round(xb));
    for (int x = xStart; x <= xEnd; x++)
        fb.SetPixel(x, y, color);
}

// Cherche sur une ligne les pixels a remplir et les empile.
static void FindSeedsOnRow(Framebuffer& fb, std::vector<Point2D>& stack, uint32_t targetColor, int y, int xLeft, int xRight)
{
    if (y < 0 || y >= fb.height)
        return;

    bool inSegment = false;
    for (int i = xLeft; i <= xRight; i++)
    {
        if (fb.GetPixel(i, y) == targetColor)
        {
            if (!inSegment)
            {
                stack.push_back({(float)i, (float)y});
                inSegment = true;
            }
        }
        else
        {
            inSegment = false;
        }
    }
}

// Convertit une couleur ImGui (0-1) en entier 32 bits.
uint32_t ColorToUint(ImVec4 c)
{
    return Framebuffer::PackColor((uint8_t)(c.x * 255), (uint8_t)(c.y * 255), (uint8_t)(c.z * 255), (uint8_t)(c.w * 255));
}

// Dessine une ligne entre deux points (algorithme de Bresenham).
void DrawLine(Framebuffer& fb, Point2D p0, Point2D p1, uint32_t color)
{
    int x0 = (int)p0.x;
    int y0 = (int)p0.y;
    int x1 = (int)p1.x;
    int y1 = (int)p1.y;

    int dx = std::abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        fb.SetPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

// Dessine un petit carre pour representer un sommet.
void DrawHandle(Framebuffer& fb, Point2D p, uint32_t color)
{
    int cx = (int)p.x;
    int cy = (int)p.y;
    for (int i = -3; i <= 3; i++)
        for (int j = -3; j <= 3; j++)
            fb.SetPixel(cx + i, cy + j, color);
}

// Dessine le contour d'un polygone, sommet par sommet.
void DrawPolygon(Framebuffer& fb, const Polygon& polygon, uint32_t color)
{
    if (polygon.vertices.empty())
        return;

    for (size_t i = 0; i < polygon.vertices.size(); i++)
    {
        Point2D v1 = polygon.vertices[i];
        Point2D v2 = polygon.vertices[(i + 1) % polygon.vertices.size()];
        DrawLine(fb, v1, v2, color);
        DrawHandle(fb, v1, color);
    }
}

// Remplit une zone fermee par propagation depuis un point de depart.
void ScanlineSeedFill(Framebuffer& fb, int startX, int startY, uint32_t fillColor)
{
    uint32_t targetColor = fb.GetPixel(startX, startY);
    if (targetColor == fillColor)
        return;

    std::vector<Point2D> stack;
    stack.push_back({(float)startX, (float)startY});

    while (!stack.empty())
    {
        Point2D pt = stack.back();
        stack.pop_back();
        int cx = (int)pt.x;
        int cy = (int)pt.y;

        if (fb.GetPixel(cx, cy) != targetColor)
            continue;

        int xLeft = cx;
        while (xLeft >= 0 && fb.GetPixel(xLeft, cy) == targetColor)
            xLeft--;
        xLeft++;

        int xRight = cx;
        while (xRight < fb.width && fb.GetPixel(xRight, cy) == targetColor)
            xRight++;
        xRight--;

        for (int i = xLeft; i <= xRight; i++)
            fb.SetPixel(i, cy, fillColor);

        FindSeedsOnRow(fb, stack, targetColor, cy - 1, xLeft, xRight);
        FindSeedsOnRow(fb, stack, targetColor, cy + 1, xLeft, xRight);
    }
}

// Remplit un polygone par balayage avec une liste d'aretes actives.
void FillPolygonLCA(Framebuffer& fb, const Polygon& polygon, uint32_t color, FillRule rule)
{
    if (polygon.vertices.size() < 3)
        return;

    std::vector<Edge> SI;
    float minY = polygon.vertices[0].y;
    float maxY = polygon.vertices[0].y;

    for (size_t i = 0; i < polygon.vertices.size(); i++)
    {
        Point2D p1 = polygon.vertices[i];
        Point2D p2 = polygon.vertices[(i + 1) % polygon.vertices.size()];

        minY = std::min(minY, std::min(p1.y, p2.y));
        maxY = std::max(maxY, std::max(p1.y, p2.y));

        if (p1.y == p2.y)
            continue;

        int dir = (p2.y > p1.y) ? 1 : -1;
        if (p1.y > p2.y)
            std::swap(p1, p2);

        float invSlope = (p2.x - p1.x) / (p2.y - p1.y);

        Edge edge;
        edge.yMin = p1.y;
        edge.yMax = p2.y;
        edge.xAtYMin = p1.x;
        edge.invSlope = invSlope;
        edge.dir = dir;
        SI.push_back(edge);
    }

    if (SI.empty())
        return;

    std::sort(SI.begin(), SI.end(), CompareEdgesByYMin);

    std::vector<ActiveEdge> LCA;

    size_t nextEdge = 0;
    int yStart = (int)std::ceil(minY);
    int yEnd = (int)std::floor(maxY);

    for (int y = yStart; y <= yEnd; y++)
    {
        while (nextEdge < SI.size() && SI[nextEdge].yMin <= (float)y)
        {
            const Edge& e = SI[nextEdge];

            ActiveEdge active;
            active.yMax = e.yMax;
            active.x = e.xAtYMin + ((float)y - e.yMin) * e.invSlope;
            active.invSlope = e.invSlope;
            active.dir = e.dir;
            LCA.push_back(active);

            nextEdge++;
        }

        for (size_t i = 0; i < LCA.size(); )
        {
            if (LCA[i].yMax <= (float)y)
                LCA.erase(LCA.begin() + i);
            else
                i++;
        }

        std::sort(LCA.begin(), LCA.end(), CompareActiveEdgesByX);

        if (rule == FillRule::EvenOdd)
        {
            for (size_t i = 0; i + 1 < LCA.size(); i += 2)
                FillSpan(fb, y, LCA[i].x, LCA[i + 1].x, color);
        }
        else
        {
            int winding = 0;
            for (size_t i = 0; i + 1 < LCA.size(); i++)
            {
                winding += LCA[i].dir;
                if (winding != 0)
                    FillSpan(fb, y, LCA[i].x, LCA[i + 1].x, color);
            }
        }

        for (size_t i = 0; i < LCA.size(); i++)
            LCA[i].x += LCA[i].invSlope;
    }
}
