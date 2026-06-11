#pragma once
#include <vector>
#include <cstdint>
#include <GLFW/glfw3.h>

class Framebuffer
{
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

    // Assemble 4 composantes de couleur en un seul entier.
    static uint32_t PackColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
        return (a << 24) | (b << 16) | (g << 8) | r;
    }
};
