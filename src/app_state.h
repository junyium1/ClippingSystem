#pragma once
#include "geometry.h"
#include "zoom.h"
#include "fractal.h"
#include "imgui.h"
#include <vector>

enum class ToolMode
{
    CyrusBeck,
    SutherlandHodgman
};

enum class EditTool
{
    Select,
    AddWindow,
    AddPolygon,
    AddSegment,
    SeedFill,
    TriangleSelect
};

const float kShapePlacementHalfSize = 80.0f;

enum class DragType
{
    None,
    ClipWindowVertex,
    ClipWindowWhole,
    SubjectPolyVertex,
    SubjectPolyWhole,
    SubjectSegmentVertex,
    SubjectSegmentWhole
};

struct AppState
{
    ToolMode currentTool = ToolMode::SutherlandHodgman;
    EditTool activeTool = EditTool::Select;

    // false = interface "Produit fini" (simple), true = interface "Pedagogique" (technique)
    bool advancedMode = false;

    DragType draggingType = DragType::None;
    int draggingObjIndex = -1;
    int draggingPointIndex = -1;
    bool isDrawingSegment = false;
    bool isPanning = false;
    bool isMouseOverCanvas = false;
    Point2D lastMousePos = {0, 0};
    Point2D lastWorldMousePos = {0, 0};

    Camera2D camera;

    ImVec4 colorMenuPoly = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
    ImVec4 colorSubject = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
    ImVec4 colorClipped = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
    ImVec4 colorBg = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    ImVec4 colorFill = ImVec4(0.8f, 0.8f, 0.2f, 1.0f);

    bool enableLCA = false;
    FillRule fillRule = FillRule::EvenOdd;

    float transformAngleDeg = 15.0f;
    float transformScaleX = 1.1f;
    float transformScaleY = 1.1f;
    float transformShearX = 0.2f;
    float transformShearY = 0.0f;
    float transformTranslateX = 20.0f;
    float transformTranslateY = 0.0f;

    bool enableEarCutting = true;

    std::vector<std::vector<int>> selectedTriangles;

    int fractalIterations = 3;
    FractalType fractalType = FractalType::KochSnowflake;

    int newShapeSides = 4;

    std::vector<Point2D> seeds;
    std::vector<Polygon> clipWindows;
    std::vector<Polygon> subjectPolys;
    std::vector<Segment> subjectSegments;

    std::vector<bool> windowVisible;
    std::vector<bool> polyVisible;
    std::vector<bool> segmentVisible;
};
