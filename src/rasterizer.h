#pragma once
#include "framebuffer.h"
#include "geometry.h"
#include "imgui.h"
#include <cstdint>

// Convertit une couleur ImGui (0-1) en entier 32 bits.
uint32_t ColorToUint(ImVec4 c);

// Dessine une ligne (algorithme de Bresenham).
void DrawLine(Framebuffer& fb, Point2D p0, Point2D p1, uint32_t color);

// Dessine un petit carre pour representer un sommet.
void DrawHandle(Framebuffer& fb, Point2D p, uint32_t color);

// Dessine le contour d'un polygone.
void DrawPolygon(Framebuffer& fb, const Polygon& polygon, uint32_t color);

// Remplit une zone fermee a partir d'un point de depart.
void ScanlineSeedFill(Framebuffer& fb, int startX, int startY, uint32_t fillColor);

// Remplit un polygone par balayage (lignes de balayage actives).
void FillPolygonLCA(Framebuffer& fb, const Polygon& polygon, uint32_t color, FillRule rule = FillRule::EvenOdd);
