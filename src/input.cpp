#include "input.h"
#include <algorithm>

// Cherche un sommet ou l'interieur d'une fenetre de decoupe sous la souris.
static bool TryPickClipWindow(AppState& state, Point2D world)
{
    for (size_t w = 0; w < state.clipWindows.size(); w++)
    {
        if (w < state.windowVisible.size() && !state.windowVisible[w])
            continue;

        for (size_t i = 0; i < state.clipWindows[w].vertices.size(); i++)
        {
            if (GetDistance(world, state.clipWindows[w].vertices[i]) < 10.0f / state.camera.zoom)
            {
                state.draggingType = DragType::ClipWindowVertex;
                state.draggingObjIndex = (int)w;
                state.draggingPointIndex = (int)i;
                return true;
            }
        }
    }

    for (size_t w = 0; w < state.clipWindows.size(); w++)
    {
        if (w < state.windowVisible.size() && !state.windowVisible[w])
            continue;

        if (IsPointInPolygon(world, state.clipWindows[w]))
        {
            state.draggingType = DragType::ClipWindowWhole;
            state.draggingObjIndex = (int)w;
            return true;
        }
    }

    return false;
}

// Cherche une extremite ou le corps d'un segment sujet sous la souris.
static bool TryPickSegment(AppState& state, Point2D world)
{
    float thresh = 10.0f / state.camera.zoom;

    for (size_t s = 0; s < state.subjectSegments.size(); s++)
    {
        if (s < state.segmentVisible.size() && !state.segmentVisible[s])
            continue;

        if (GetDistance(world, state.subjectSegments[s].a) < thresh)
        {
            state.draggingType = DragType::SubjectSegmentVertex;
            state.draggingObjIndex = (int)s;
            state.draggingPointIndex = 0;
            return true;
        }
        if (GetDistance(world, state.subjectSegments[s].b) < thresh)
        {
            state.draggingType = DragType::SubjectSegmentVertex;
            state.draggingObjIndex = (int)s;
            state.draggingPointIndex = 1;
            return true;
        }
    }

    for (size_t s = 0; s < state.subjectSegments.size(); s++)
    {
        if (s < state.segmentVisible.size() && !state.segmentVisible[s])
            continue;

        if (DistancePointToSegment(world, state.subjectSegments[s].a, state.subjectSegments[s].b) < thresh)
        {
            state.draggingType = DragType::SubjectSegmentWhole;
            state.draggingObjIndex = (int)s;
            return true;
        }
    }

    return false;
}

// Cherche un sommet ou l'interieur d'un polygone sujet sous la souris.
static bool TryPickPolygon(AppState& state, Point2D world)
{
    float thresh = 10.0f / state.camera.zoom;

    for (size_t p = 0; p < state.subjectPolys.size(); p++)
    {
        if (p < state.polyVisible.size() && !state.polyVisible[p])
            continue;

        for (size_t i = 0; i < state.subjectPolys[p].vertices.size(); i++)
        {
            if (GetDistance(world, state.subjectPolys[p].vertices[i]) < thresh)
            {
                state.draggingType = DragType::SubjectPolyVertex;
                state.draggingObjIndex = (int)p;
                state.draggingPointIndex = (int)i;
                return true;
            }
        }
    }

    for (size_t p = 0; p < state.subjectPolys.size(); p++)
    {
        if (p < state.polyVisible.size() && !state.polyVisible[p])
            continue;

        if (IsPointInPolygon(world, state.subjectPolys[p]))
        {
            state.draggingType = DragType::SubjectPolyWhole;
            state.draggingObjIndex = (int)p;
            return true;
        }
    }

    return false;
}

// Ajoute ou retire un index de triangle dans la liste de selection.
static void ToggleTriangleSelection(std::vector<int>& selected, int triangleIndex)
{
    for (size_t k = 0; k < selected.size(); k++)
    {
        if (selected[k] == triangleIndex)
        {
            selected.erase(selected.begin() + k);
            return;
        }
    }

    selected.push_back(triangleIndex);
}

// Selectionne ou deselectionne le triangle sous la souris.
static bool TryToggleTriangle(AppState& state, Point2D world)
{
    const size_t kMaxTriangulationVerts = 64;

    for (size_t p = 0; p < state.subjectPolys.size() && p < state.selectedTriangles.size(); p++)
    {
        if (p < state.polyVisible.size() && !state.polyVisible[p])
            continue;

        const Polygon& poly = state.subjectPolys[p];
        if (poly.vertices.size() < 3 || poly.vertices.size() > kMaxTriangulationVerts)
            continue;
        if (!IsPointInPolygon(world, poly))
            continue;

        std::vector<Polygon> triangles = TriangulatePolygon(poly);
        for (size_t t = 0; t < triangles.size(); t++)
        {
            const std::vector<Point2D>& v = triangles[t].vertices;
            if (PointInTriangle(world, v[0], v[1], v[2]))
            {
                ToggleTriangleSelection(state.selectedTriangles[p], (int)t);
                return true;
            }
        }
    }

    return false;
}

// Supprime la fenetre, le segment ou le polygone sous la souris.
static bool TryDeleteShapeAt(AppState& state, Point2D world)
{
    float thresh = 10.0f / state.camera.zoom;

    for (size_t w = 0; w < state.clipWindows.size(); w++)
    {
        if (w < state.windowVisible.size() && !state.windowVisible[w])
            continue;

        if (IsPointInPolygon(world, state.clipWindows[w]))
        {
            state.clipWindows.erase(state.clipWindows.begin() + (long)w);
            if (w < state.windowVisible.size())
                state.windowVisible.erase(state.windowVisible.begin() + (long)w);
            return true;
        }
    }

    for (size_t s = 0; s < state.subjectSegments.size(); s++)
    {
        if (s < state.segmentVisible.size() && !state.segmentVisible[s])
            continue;

        if (DistancePointToSegment(world, state.subjectSegments[s].a, state.subjectSegments[s].b) < thresh)
        {
            state.subjectSegments.erase(state.subjectSegments.begin() + (long)s);
            if (s < state.segmentVisible.size())
                state.segmentVisible.erase(state.segmentVisible.begin() + (long)s);
            return true;
        }
    }

    for (size_t p = 0; p < state.subjectPolys.size(); p++)
    {
        if (p < state.polyVisible.size() && !state.polyVisible[p])
            continue;

        if (IsPointInPolygon(world, state.subjectPolys[p]))
        {
            state.subjectPolys.erase(state.subjectPolys.begin() + (long)p);
            if (p < state.selectedTriangles.size())
                state.selectedTriangles.erase(state.selectedTriangles.begin() + (long)p);
            if (p < state.polyVisible.size())
                state.polyVisible.erase(state.polyVisible.begin() + (long)p);
            return true;
        }
    }

    return false;
}

// Applique le deplacement en cours a l'objet selectionne.
static void ApplyDrag(AppState& state, Point2D world)
{
    float dx = world.x - state.lastMousePos.x;
    float dy = world.y - state.lastMousePos.y;

    switch (state.draggingType)
    {
        case DragType::ClipWindowVertex:
            state.clipWindows[state.draggingObjIndex].vertices[state.draggingPointIndex] = world;
            break;

        case DragType::ClipWindowWhole:
            for (Point2D& v : state.clipWindows[state.draggingObjIndex].vertices)
            {
                v.x += dx;
                v.y += dy;
            }
            break;

        case DragType::SubjectSegmentVertex:
            if (state.draggingPointIndex == 0)
                state.subjectSegments[state.draggingObjIndex].a = world;
            else
                state.subjectSegments[state.draggingObjIndex].b = world;
            break;

        case DragType::SubjectSegmentWhole:
            state.subjectSegments[state.draggingObjIndex].a.x += dx;
            state.subjectSegments[state.draggingObjIndex].a.y += dy;
            state.subjectSegments[state.draggingObjIndex].b.x += dx;
            state.subjectSegments[state.draggingObjIndex].b.y += dy;
            break;

        case DragType::SubjectPolyVertex:
            state.subjectPolys[state.draggingObjIndex].vertices[state.draggingPointIndex] = world;
            break;

        case DragType::SubjectPolyWhole:
            for (Point2D& v : state.subjectPolys[state.draggingObjIndex].vertices)
            {
                v.x += dx;
                v.y += dy;
            }
            break;

        default:
            break;
    }
}

// Traite la souris sur le canevas (zoom, deplacement, outils).
void HandleInput(AppState& state, ImVec2 canvasPos, ImVec2 canvasSize)
{
    ImGuiIO& io = ImGui::GetIO();

    Point2D screen;
    screen.x = std::max(0.0f, std::min(io.MousePos.x - canvasPos.x, canvasSize.x));
    screen.y = std::max(0.0f, std::min(io.MousePos.y - canvasPos.y, canvasSize.y));

    Point2D world = state.camera.ScreenToWorld(screen);

    bool hovered = ImGui::IsItemHovered();
    state.isMouseOverCanvas = hovered;
    if (hovered)
        state.lastWorldMousePos = world;

    if (hovered && io.MouseWheel != 0.0f)
    {
        float factor = (io.MouseWheel > 0.0f) ? 1.1f : 1.0f / 1.1f;
        state.camera.ZoomAt(screen, factor);
    }

    if (hovered && ImGui::IsMouseClicked(2))
    {
        state.isPanning = true;
        state.lastMousePos = screen;
    }

    if (io.MouseDown[2] && state.isPanning)
    {
        Point2D delta = { screen.x - state.lastMousePos.x, screen.y - state.lastMousePos.y };
        state.camera.Pan(delta);
        state.lastMousePos = screen;
    }

    if (ImGui::IsMouseReleased(2))
        state.isPanning = false;

    if (hovered && ImGui::IsMouseClicked(1))
        TryDeleteShapeAt(state, world);

    switch (state.activeTool)
    {
        case EditTool::Select:
        {
            if (hovered && ImGui::IsMouseClicked(0))
            {
                state.lastMousePos = world;

                bool picked = TryPickClipWindow(state, world);

                if (!picked && (!state.advancedMode || state.currentTool == ToolMode::CyrusBeck))
                    picked = TryPickSegment(state, world);

                if (!picked)
                    picked = TryPickPolygon(state, world);
            }

            if (io.MouseDown[0] && state.draggingType != DragType::None)
            {
                ApplyDrag(state, world);
                state.lastMousePos = world;
            }
            else if (!io.MouseDown[0])
            {
                state.draggingType = DragType::None;
                state.draggingObjIndex = -1;
                state.draggingPointIndex = -1;
            }
            break;
        }

        case EditTool::AddWindow:
            if (hovered && ImGui::IsMouseClicked(0))
            {
                state.clipWindows.push_back(MakeRegularPolygon(world, kShapePlacementHalfSize, state.newShapeSides));
                state.windowVisible.push_back(true);
                state.activeTool = EditTool::Select;
            }
            break;

        case EditTool::AddPolygon:
            if (hovered && ImGui::IsMouseClicked(0))
            {
                state.subjectPolys.push_back(MakeRegularPolygon(world, kShapePlacementHalfSize, state.newShapeSides));
                state.selectedTriangles.push_back({});
                state.polyVisible.push_back(true);
                state.activeTool = EditTool::Select;
            }
            break;

        case EditTool::AddSegment:
            if (hovered && ImGui::IsMouseClicked(0))
            {
                state.isDrawingSegment = true;
                state.subjectSegments.push_back({world, world});
                state.segmentVisible.push_back(true);
            }
            if (io.MouseDown[0] && state.isDrawingSegment && !state.subjectSegments.empty())
                state.subjectSegments.back().b = world;
            if (ImGui::IsMouseReleased(0))
                state.isDrawingSegment = false;
            break;

        case EditTool::SeedFill:
            if (hovered && ImGui::IsMouseClicked(0))
                state.seeds.push_back(world);
            break;

        case EditTool::TriangleSelect:
            if (hovered && ImGui::IsMouseClicked(0))
                TryToggleTriangle(state, world);
            break;
    }
}
