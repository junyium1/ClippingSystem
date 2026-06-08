//
// Created by az on 08/06/2026.
//

#ifndef DEV_RASTERIZER_H
#define DEV_RASTERIZER_H

#pragma once
#include "framebuffer.h"
#include "geometry.h"
#include "imgui.h"
#include <cstdint>

uint32_t ColorToUint(ImVec4 c);

// Fonctions de tracé vectoriel au pixel
void DrawLine(Framebuffer& fb, Point2D p0, Point2D p1, uint32_t color);
void DrawHandle(Framebuffer& fb, Point2D p, uint32_t color);
void DrawPolygon(Framebuffer& fb, const Polygon& polygon, uint32_t color);

// Algorithme remplissage à germe
void ScanlineSeedFill(Framebuffer& fb, int startX, int startY, uint32_t fillColor);
#endif // DEV_RASTERIZER_H
