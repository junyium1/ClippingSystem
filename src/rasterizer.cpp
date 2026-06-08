//
// Created by az on 08/06/2026.
//

#include "rasterizer.h"
#include <cmath>
#include <vector>

uint32_t ColorToUint(ImVec4 c)
{
  return Framebuffer::PackColor((uint8_t)(c.x * 255), (uint8_t)(c.y * 255), (uint8_t)(c.z * 255), (uint8_t)(c.w * 255));
}

void DrawLine(Framebuffer& fb, Point2D p0, Point2D p1, uint32_t color)
{
  int x0 = (int)p0.x; int y0 = (int)p0.y;
  int x1 = (int)p1.x; int y1 = (int)p1.y;

  int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (true)
  {
    fb.SetPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void DrawHandle(Framebuffer& fb, Point2D p, uint32_t color)
{
  int cx = (int)p.x; int cy = (int)p.y;
  for (int i = -3; i <= 3; i++)
    for (int j = -3; j <= 3; j++)
      fb.SetPixel(cx + i, cy + j, color);
}

void DrawPolygon(Framebuffer& fb, const Polygon& polygon, uint32_t color)
{
  if (polygon.vertices.empty()) return;
  for (size_t i = 0; i < polygon.vertices.size(); i++)
  {
    Point2D v1 = polygon.vertices[i];
    Point2D v2 = polygon.vertices[(i + 1) % polygon.vertices.size()];
    DrawLine(fb, v1, v2, color);
    DrawHandle(fb, v1, color);
  }
}

void ScanlineSeedFill(Framebuffer& fb, int startX, int startY, uint32_t fillColor)
{
  // On mémorise la couleur de la zone à remplir (le fond)
  uint32_t targetColor = fb.GetPixel(startX, startY);

  // Si on clique sur une zone déjà remplie, on annule pour éviter une boucle infinie
  if (targetColor == fillColor) return;

  std::vector<Point2D> stack;
  stack.push_back({(float)startX, (float)startY});

  auto scanLine = [&](int y, int xLeft, int xRight)
  {
    if (y < 0 || y >= fb.height) return;
    bool inSegment = false;
    for (int i = xLeft; i <= xRight; i++)
    {
      // On continue tant qu'on trouve la couleur cible (le fond)
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
  };

  while (!stack.empty())
  {
    Point2D pt = stack.back();
    stack.pop_back();
    int cx = (int)pt.x, cy = (int)pt.y;

    // Sécurité au cas où un autre germe aurait déjà rempli ce pixel
    if (fb.GetPixel(cx, cy) != targetColor) continue;

    int xLeft = cx;
    while (xLeft >= 0 && fb.GetPixel(xLeft, cy) == targetColor) xLeft--;
    xLeft++;

    int xRight = cx;
    while (xRight < fb.width && fb.GetPixel(xRight, cy) == targetColor) xRight++;
    xRight--;

    for (int i = xLeft; i <= xRight; i++) fb.SetPixel(i, cy, fillColor);

    scanLine(cy - 1, xLeft, xRight);
    scanLine(cy + 1, xLeft, xRight);
  }
}