//
// Created by az on 08/06/2026.
//

#ifndef DEV_FRAMEBUFFER_H
#define DEV_FRAMEBUFFER_H

#pragma once
#include <vector>
#include <cstdint>
#include <GLFW/glfw3.h> // Pour les types GL

class Framebuffer {
public:
  int width;
  int height;
  std::vector<uint32_t> pixels;
  GLuint textureID;

  Framebuffer(int w, int h);
  ~Framebuffer();

  void Resize(int new_w, int new_h);

  void Clear(uint32_t color);

  void SetPixel(int x, int y, uint32_t color);

  uint32_t GetPixel(int x, int y) const;

  void UpdateTexture();

  // Utilitaire pour encoder RGBA dans un seul uint32_t
  static uint32_t PackColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (a << 24) | (b << 16) | (g << 8) | r;
  }
};

#endif // DEV_FRAMEBUFFER_H
