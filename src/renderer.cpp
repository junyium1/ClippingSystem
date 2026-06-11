#include "renderer.h"
#include "rasterizer.h"
#include "CyrusBeck.h"
#include "SutherlandHodgman.h"

// Convertit les sommets d'un polygone du monde vers l'ecran.
static Polygon ToScreen(const Polygon& poly, const Camera2D& cam)
{
    Polygon out;
    for (size_t i = 0; i < poly.vertices.size(); i++)
        out.vertices.push_back(cam.WorldToScreen(poly.vertices[i]));
    return out;
}

// Convertit un segment du monde vers l'ecran.
static Segment ToScreen(const Segment& seg, const Camera2D& cam)
{
    Segment out;
    out.a = cam.WorldToScreen(seg.a);
    out.b = cam.WorldToScreen(seg.b);
    return out;
}

// Teste si un index de triangle est dans la liste de selection.
static bool IsTriangleSelected(const std::vector<int>& selected, int triangleIndex)
{
    for (size_t i = 0; i < selected.size(); i++)
        if (selected[i] == triangleIndex)
            return true;
    return false;
}

// Decoupe un polygone par une fenetre avec Cyrus-Beck (triangles si concave).
static std::vector<Polygon> ClipPolygonAgainstWindowCyrusBeck(const Polygon& subject, const Polygon& window, bool earCutting)
{
    std::vector<Polygon> result;

    if (!earCutting || IsConvexPolygon(window))
    {
        Polygon clipped = CyrusBeckClipper::clipPolygon(subject, window);
        if (!clipped.vertices.empty())
            result.push_back(clipped);
        return result;
    }

    std::vector<Polygon> triangles = TriangulatePolygon(window);
    for (size_t i = 0; i < triangles.size(); i++)
    {
        Polygon clipped = CyrusBeckClipper::clipPolygon(subject, triangles[i]);
        if (!clipped.vertices.empty())
            result.push_back(clipped);
    }
    return result;
}

// Decoupe un polygone par une fenetre avec Sutherland-Hodgman (triangles si concave).
static std::vector<Polygon> ClipPolygonAgainstWindowSutherlandHodgman(const Polygon& subject, const Polygon& window, bool earCutting)
{
    std::vector<Polygon> result;

    if (!earCutting || IsConvexPolygon(window))
    {
        Polygon clipped = SutherlandHodgmanClipper::clipPolygon(subject, window);
        if (!clipped.vertices.empty())
            result.push_back(clipped);
        return result;
    }

    std::vector<Polygon> triangles = TriangulatePolygon(window);
    for (size_t i = 0; i < triangles.size(); i++)
    {
        Polygon clipped = SutherlandHodgmanClipper::clipPolygon(subject, triangles[i]);
        if (!clipped.vertices.empty())
            result.push_back(clipped);
    }
    return result;
}

// Decoupe un segment par une fenetre avec Cyrus-Beck (triangles si concave).
static std::vector<Segment> ClipSegmentAgainstWindow(const Segment& seg, const Polygon& window, bool earCutting)
{
    std::vector<Segment> result;

    if (!earCutting || IsConvexPolygon(window))
    {
        Segment clipped;
        if (CyrusBeckClipper::clipLine(seg.a, seg.b, window, clipped))
            result.push_back(clipped);
        return result;
    }

    std::vector<Polygon> triangles = TriangulatePolygon(window);
    for (size_t i = 0; i < triangles.size(); i++)
    {
        Segment clipped;
        if (CyrusBeckClipper::clipLine(seg.a, seg.b, triangles[i], clipped))
            result.push_back(clipped);
    }
    return result;
}

// Dessine les segments sujets decoupes avec Cyrus-Beck.
static void RenderSegmentsCyrusBeck(AppState& state, Framebuffer& fb, uint32_t subjCol, uint32_t clipCol)
{
    for (size_t s = 0; s < state.subjectSegments.size(); s++)
    {
        if (s < state.segmentVisible.size() && !state.segmentVisible[s])
            continue;

        const Segment& seg = state.subjectSegments[s];
        Segment screenSeg = ToScreen(seg, state.camera);

        DrawLine(fb, screenSeg.a, screenSeg.b, subjCol);
        DrawHandle(fb, screenSeg.a, subjCol);
        DrawHandle(fb, screenSeg.b, subjCol);

        std::vector<Segment> clippedSegs;
        clippedSegs.push_back(seg);

        for (size_t w = 0; w < state.clipWindows.size(); w++)
        {
            if (w < state.windowVisible.size() && !state.windowVisible[w])
                continue;

            const Polygon& window = state.clipWindows[w];
            if (window.vertices.size() < 3)
                continue;

            std::vector<Segment> next;
            for (size_t i = 0; i < clippedSegs.size(); i++)
            {
                std::vector<Segment> pieces = ClipSegmentAgainstWindow(clippedSegs[i], window, state.enableEarCutting);
                for (size_t j = 0; j < pieces.size(); j++)
                    next.push_back(pieces[j]);
            }
            clippedSegs = next;
            if (clippedSegs.empty())
                break;
        }

        for (size_t i = 0; i < clippedSegs.size(); i++)
        {
            Segment screenClipped = ToScreen(clippedSegs[i], state.camera);
            DrawLine(fb, screenClipped.a, screenClipped.b, clipCol);
            DrawHandle(fb, screenClipped.a, clipCol);
            DrawHandle(fb, screenClipped.b, clipCol);
        }
    }
}

// Dessine les polygones sujets decoupes avec Cyrus-Beck.
static void RenderPolygonsCyrusBeck(AppState& state, Framebuffer& fb, uint32_t subjCol, uint32_t clipCol, uint32_t fillCol)
{
    for (size_t p = 0; p < state.subjectPolys.size(); p++)
    {
        if (p < state.polyVisible.size() && !state.polyVisible[p])
            continue;

        const Polygon& poly = state.subjectPolys[p];
        Polygon screenPoly = ToScreen(poly, state.camera);
        if (state.enableLCA)
            FillPolygonLCA(fb, screenPoly, fillCol, state.fillRule);
        DrawPolygon(fb, screenPoly, subjCol);

        std::vector<Polygon> clippedPolys;
        clippedPolys.push_back(poly);

        for (size_t w = 0; w < state.clipWindows.size(); w++)
        {
            if (w < state.windowVisible.size() && !state.windowVisible[w])
                continue;

            const Polygon& window = state.clipWindows[w];
            if (window.vertices.size() < 3)
                continue;

            std::vector<Polygon> next;
            for (size_t i = 0; i < clippedPolys.size(); i++)
            {
                std::vector<Polygon> pieces = ClipPolygonAgainstWindowCyrusBeck(clippedPolys[i], window, state.enableEarCutting);
                for (size_t j = 0; j < pieces.size(); j++)
                    next.push_back(pieces[j]);
            }
            clippedPolys = next;
            if (clippedPolys.empty())
                break;
        }

        for (size_t i = 0; i < clippedPolys.size(); i++)
        {
            Polygon screenClipped = ToScreen(clippedPolys[i], state.camera);
            if (state.enableLCA)
                FillPolygonLCA(fb, screenClipped, clipCol, state.fillRule);
            DrawPolygon(fb, screenClipped, clipCol);
        }
    }
}

// Dessine les polygones sujets decoupes avec Sutherland-Hodgman.
static void RenderSutherlandHodgman(AppState& state, Framebuffer& fb, uint32_t subjCol, uint32_t clipCol, uint32_t fillCol)
{
    for (size_t p = 0; p < state.subjectPolys.size(); p++)
    {
        if (p < state.polyVisible.size() && !state.polyVisible[p])
            continue;

        const Polygon& poly = state.subjectPolys[p];
        Polygon screenPoly = ToScreen(poly, state.camera);
        if (state.enableLCA)
            FillPolygonLCA(fb, screenPoly, fillCol, state.fillRule);
        DrawPolygon(fb, screenPoly, subjCol);

        std::vector<Polygon> clippedPolys;
        clippedPolys.push_back(poly);

        for (size_t w = 0; w < state.clipWindows.size(); w++)
        {
            if (w < state.windowVisible.size() && !state.windowVisible[w])
                continue;

            const Polygon& window = state.clipWindows[w];
            if (window.vertices.size() < 3)
                continue;

            std::vector<Polygon> next;
            for (size_t i = 0; i < clippedPolys.size(); i++)
            {
                std::vector<Polygon> pieces = ClipPolygonAgainstWindowSutherlandHodgman(clippedPolys[i], window, state.enableEarCutting);
                for (size_t j = 0; j < pieces.size(); j++)
                    next.push_back(pieces[j]);
            }
            clippedPolys = next;
            if (clippedPolys.empty())
                break;
        }

        for (size_t i = 0; i < clippedPolys.size(); i++)
        {
            Polygon screenClipped = ToScreen(clippedPolys[i], state.camera);
            if (state.enableLCA)
                FillPolygonLCA(fb, screenClipped, clipCol, state.fillRule);
            DrawPolygon(fb, screenClipped, clipCol);
        }
    }
}

// Dessine le remplissage partiel des triangles selectionnes.
static void RenderPartialTriangleFill(AppState& state, Framebuffer& fb, uint32_t fillCol, uint32_t outlineCol)
{
    const size_t kMaxTriangulationVerts = 64;

    for (size_t p = 0; p < state.subjectPolys.size() && p < state.selectedTriangles.size(); p++)
    {
        if (p < state.polyVisible.size() && !state.polyVisible[p])
            continue;

        const Polygon& poly = state.subjectPolys[p];
        if (poly.vertices.size() < 3 || poly.vertices.size() > kMaxTriangulationVerts)
            continue;

        std::vector<Polygon> triangles = TriangulatePolygon(poly);
        const std::vector<int>& selected = state.selectedTriangles[p];

        for (size_t t = 0; t < triangles.size(); t++)
        {
            Polygon screenTri = ToScreen(triangles[t], state.camera);

            if (IsTriangleSelected(selected, (int)t))
                FillPolygonLCA(fb, screenTri, fillCol, state.fillRule);

            if (state.activeTool == EditTool::TriangleSelect)
                DrawPolygon(fb, screenTri, outlineCol);
        }
    }
}

// Dessine un apercu transparent de la prochaine forme a placer.
static void RenderPlacementPreview(AppState& state, Framebuffer& fb)
{
    if (!state.isMouseOverCanvas)
        return;
    if (state.activeTool != EditTool::AddWindow && state.activeTool != EditTool::AddPolygon)
        return;

    Polygon preview = MakeRegularPolygon(state.lastWorldMousePos, kShapePlacementHalfSize, state.newShapeSides);
    Polygon screenPreview = ToScreen(preview, state.camera);
    DrawPolygon(fb, screenPreview, ColorToUint(ImVec4(1.0f, 1.0f, 1.0f, 0.5f)));
}

// Dessine toute la scene (fenetres, formes, decoupes, remplissages).
void RenderCanvas(AppState& state, Framebuffer& fb)
{
    fb.Clear(ColorToUint(state.colorBg));

    uint32_t polyCol = ColorToUint(state.colorMenuPoly);
    uint32_t subjCol = ColorToUint(state.colorSubject);
    uint32_t clipCol = ColorToUint(state.colorClipped);
    uint32_t fillCol = ColorToUint(state.colorFill);

    for (size_t w = 0; w < state.clipWindows.size(); w++)
    {
        if (w < state.windowVisible.size() && !state.windowVisible[w])
            continue;

        const Polygon& window = state.clipWindows[w];
        DrawPolygon(fb, ToScreen(window, state.camera), polyCol);

        if (state.advancedMode && state.enableEarCutting && !IsConvexPolygon(window))
        {
            uint32_t earCol = ColorToUint(ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
            std::vector<Polygon> triangles = TriangulatePolygon(window);
            for (size_t i = 0; i < triangles.size(); i++)
                DrawPolygon(fb, ToScreen(triangles[i], state.camera), earCol);
        }
    }

    if (state.advancedMode)
    {
        if (state.currentTool == ToolMode::CyrusBeck)
        {
            RenderSegmentsCyrusBeck(state, fb, subjCol, clipCol);
            RenderPolygonsCyrusBeck(state, fb, subjCol, clipCol, fillCol);
        }
        else
        {
            RenderSutherlandHodgman(state, fb, subjCol, clipCol, fillCol);
        }
    }
    else
    {
        RenderSegmentsCyrusBeck(state, fb, subjCol, clipCol);
        RenderSutherlandHodgman(state, fb, subjCol, clipCol, fillCol);
    }

    RenderPartialTriangleFill(state, fb, fillCol, polyCol);
    RenderPlacementPreview(state, fb);

    for (size_t i = 0; i < state.seeds.size(); i++)
    {
        Point2D screenSeed = state.camera.WorldToScreen(state.seeds[i]);
        ScanlineSeedFill(fb, (int)screenSeed.x, (int)screenSeed.y, fillCol);
    }

    fb.UpdateTexture();
}
