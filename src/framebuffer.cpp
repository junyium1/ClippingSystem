#include "framebuffer.h"
#include <algorithm>

// Cree le framebuffer et la texture OpenGL associee.
Framebuffer::Framebuffer(int w, int h)
    : width(w), height(h)
{
    pixels.resize(width * height, 0xFF000000);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

// Libere la texture OpenGL.
Framebuffer::~Framebuffer()
{
    glDeleteTextures(1, &textureID);
}

// Change la taille du framebuffer et de la texture.
void Framebuffer::Resize(int new_w, int new_h)
{
    if (width == new_w && height == new_h)
        return;

    width = new_w;
    height = new_h;
    pixels.resize(width * height);
    Clear(0xFF202020);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

// Remplit tous les pixels avec une meme couleur.
void Framebuffer::Clear(uint32_t color)
{
    std::fill(pixels.begin(), pixels.end(), color);
}

// Modifie la couleur d'un pixel.
void Framebuffer::SetPixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;

    pixels[y * width + x] = color;
}

// Lit la couleur d'un pixel.
uint32_t Framebuffer::GetPixel(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        return 0;

    return pixels[y * width + x];
}

// Envoie les pixels modifies vers la texture OpenGL.
void Framebuffer::UpdateTexture()
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}
