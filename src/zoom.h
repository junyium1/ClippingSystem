#pragma once
#include "geometry.h"

struct Camera2D
{
    float zoom = 1.0f;
    Point2D offset = {0.0f, 0.0f};

    // Convertit une coordonnee du monde vers l'ecran.
    Point2D WorldToScreen(const Point2D& p) const
    {
        Point2D result;
        result.x = (p.x - offset.x) * zoom;
        result.y = (p.y - offset.y) * zoom;
        return result;
    }

    // Convertit une coordonnee de l'ecran vers le monde.
    Point2D ScreenToWorld(const Point2D& p) const
    {
        Point2D result;
        result.x = p.x / zoom + offset.x;
        result.y = p.y / zoom + offset.y;
        return result;
    }

    // Zoom en gardant fixe le point sous la souris.
    void ZoomAt(const Point2D& screenPos, float zoomFactor)
    {
        Point2D before = ScreenToWorld(screenPos);

        zoom *= zoomFactor;
        if (zoom < 0.1f)
            zoom = 0.1f;
        if (zoom > 20.0f)
            zoom = 20.0f;

        Point2D after = ScreenToWorld(screenPos);

        offset.x += (before.x - after.x);
        offset.y += (before.y - after.y);
    }

    // Deplace la camera a l'ecran.
    void Pan(const Point2D& deltaScreen)
    {
        offset.x -= deltaScreen.x / zoom;
        offset.y -= deltaScreen.y / zoom;
    }
};
