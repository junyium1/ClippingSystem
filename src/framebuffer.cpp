// Framebuffer.cpp
#include "framebuffer.h"

Framebuffer::Framebuffer(int w, int h) : width(w), height(h) {
  pixels.resize(width * height, 0xFF000000); // Initialise en noir opaque

  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

Framebuffer::~Framebuffer() {
  glDeleteTextures(1, &textureID);
}

void Framebuffer::Resize(int new_w, int new_h) {
  if (width == new_w && height == new_h) return;

  width = new_w;
  height = new_h;
  pixels.resize(width * height);
  Clear(0xFF202020); // Gris foncé par défaut

  // Re-alloue la texture GPU avec la nouvelle taille
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void Framebuffer::Clear(uint32_t color) {
  std::fill(pixels.begin(), pixels.end(), color);
}

void Framebuffer::SetPixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= width || y < 0 || y >= height) return;

  pixels[y * width + x] = color;
}

uint32_t Framebuffer::GetPixel(int x, int y) const {
  if (x < 0 || x >= width || y < 0 || y >= height) return 0;
  return pixels[y * width + x];
}

void Framebuffer::UpdateTexture() {
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}