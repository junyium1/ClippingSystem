#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "geometry.h"
#include "CyrusBeck.h"
#include "SutherlandHodgman.h"
#include "framebuffer.h"
#include "rasterizer.h"
#include <vector>
#include <cmath>
#include <algorithm>

enum class ToolMode
{
    CyrusBeck,
    SutherlandHodgman
};

struct AppState
{
    ToolMode currentTool = ToolMode::CyrusBeck;
    int draggingPointIndex = -1;

    ImVec4 colorMenuPoly = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
    ImVec4 colorSubject  = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
    ImVec4 colorClipped  = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
    ImVec4 colorBg       = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    ImVec4 colorFill     = ImVec4(0.8f, 0.8f, 0.2f, 1.0f);

    std::vector<Point2D> seeds;

    Polygon clipPoly = {{ {300, 100}, {600, 100}, {800, 300}, {700, 500}, {300, 500} }};
    Point2D cb_p0 = {200, 200};
    Point2D cb_p1 = {700, 400};
    Polygon subjectPoly = {{ {200, 200}, {500, 150}, {600, 400}, {400, 600}, {150, 400} }};
};

float GetDistance(Point2D a, Point2D b)
{
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

// Window init
GLFWwindow* InitializeGraphics()
{
    if (!glfwInit())
    {
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "2D Software Rasterizer", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    return window;
}

// Window cleanup
void ShutdownGraphics(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Start frame
void BeginGUIFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

// Render UI to OpenGL
void EndGUIFrame(GLFWwindow* window)
{
    ImGui::Render();
    int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

// Left menu panel
void RenderLeftPanel(AppState& state)
{
    ImGui::BeginChild("LeftPanel", ImVec2(300, 0), true);

    if (ImGui::BeginTabBar("Tools"))
    {
        if (ImGui::BeginTabItem("Cyrus-Beck"))
        {
            if (state.currentTool != ToolMode::CyrusBeck)
            {
                state.currentTool = ToolMode::CyrusBeck;
                state.draggingPointIndex = -1;
            }
            ImGui::ColorEdit4("Clip Window", (float*)&state.colorMenuPoly);
            ImGui::ColorEdit4("Subject", (float*)&state.colorSubject);
            ImGui::ColorEdit4("Result", (float*)&state.colorClipped);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Sutherland-H."))
        {
            if (state.currentTool != ToolMode::SutherlandHodgman)
            {
                state.currentTool = ToolMode::SutherlandHodgman;
                state.draggingPointIndex = -1;
            }
            ImGui::ColorEdit4("Clip Window", (float*)&state.colorMenuPoly);
            ImGui::ColorEdit4("Subject", (float*)&state.colorSubject);
            ImGui::ColorEdit4("Result", (float*)&state.colorClipped);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::ColorEdit4("Fill Color", (float*)&state.colorFill);
    if (ImGui::Button("Clear Seeds"))
    {
        state.seeds.clear();
    }

    ImGui::EndChild();
}

// Mouse logic
void HandleInput(AppState& state, ImVec2 canvasPos, ImVec2 canvasSize)
{
    ImGuiIO& io = ImGui::GetIO();
    Point2D mouse = { io.MousePos.x - canvasPos.x, io.MousePos.y - canvasPos.y };

    mouse.x = std::max(0.0f, std::min(mouse.x, canvasSize.x));
    mouse.y = std::max(0.0f, std::min(mouse.y, canvasSize.y));

    bool isHovered = ImGui::IsItemHovered();

    if (isHovered && ImGui::IsMouseClicked(0))
    {
        if (state.currentTool == ToolMode::CyrusBeck)
        {
            if (GetDistance(mouse, state.cb_p0) < 15.0f) state.draggingPointIndex = 0;
            else if (GetDistance(mouse, state.cb_p1) < 15.0f) state.draggingPointIndex = 1;
        }
        else
        {
            for (size_t i = 0; i < state.subjectPoly.vertices.size(); i++)
            {
                if (GetDistance(mouse, state.subjectPoly.vertices[i]) < 15.0f)
                {
                    state.draggingPointIndex = (int)i;
                    break;
                }
            }
        }
    }

    if (isHovered && ImGui::IsMouseClicked(1))
    {
        state.seeds.push_back(mouse);
    }

    if (!io.MouseDown[0])
    {
        state.draggingPointIndex = -1;
    }

    if (state.draggingPointIndex != -1)
    {
        if (state.currentTool == ToolMode::CyrusBeck)
        {
            if (state.draggingPointIndex == 0) state.cb_p0 = mouse;
            else state.cb_p1 = mouse;
        }
        else
        {
            state.subjectPoly.vertices[state.draggingPointIndex] = mouse;
        }
    }
}

// Software rasterization
void RenderCanvas(AppState& state, Framebuffer& fb)
{
    fb.Clear(ColorToUint(state.colorBg));

    uint32_t polyCol = ColorToUint(state.colorMenuPoly);
    uint32_t subjCol = ColorToUint(state.colorSubject);
    uint32_t clipCol = ColorToUint(state.colorClipped);
    uint32_t fillCol = ColorToUint(state.colorFill);

    DrawPolygon(fb, state.clipPoly, polyCol);

    if (state.currentTool == ToolMode::CyrusBeck)
    {
        DrawLine(fb, state.cb_p0, state.cb_p1, subjCol);
        DrawHandle(fb, state.cb_p0, subjCol);
        DrawHandle(fb, state.cb_p1, subjCol);

        auto clipped = CyrusBeckClipper::clipLine(state.cb_p0, state.cb_p1, state.clipPoly);
        if (clipped.has_value())
        {
            DrawLine(fb, clipped->first, clipped->second, clipCol);
            DrawHandle(fb, clipped->first, clipCol);
            DrawHandle(fb, clipped->second, clipCol);
        }
    }
    else
    {
        DrawPolygon(fb, state.subjectPoly, subjCol);
        Polygon clippedPoly = SutherlandHodgmanClipper::clipPolygon(state.subjectPoly, state.clipPoly);
        DrawPolygon(fb, clippedPoly, clipCol);
    }

    for (const auto& seed : state.seeds)
    {
      ScanlineSeedFill(fb, (int)seed.x, (int)seed.y, fillCol);
    }

    fb.UpdateTexture();
}

// Main application UI assembly
void RenderAppUI(AppState& state, Framebuffer& fb)
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    RenderLeftPanel(state);
    ImGui::SameLine();

    ImGui::BeginChild("CanvasChild", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    fb.Resize((int)canvasSize.x, (int)canvasSize.y);

    ImGui::InvisibleButton("canvas_btn", canvasSize);
    HandleInput(state, canvasPos, canvasSize);

    RenderCanvas(state, fb);

    ImGui::SetCursorScreenPos(canvasPos);
    ImGui::Image((void*)(intptr_t)fb.textureID, canvasSize);
    ImGui::EndChild();

    ImGui::End();
}


int main()
{
    GLFWwindow* window = InitializeGraphics();
    if (!window) return -1;

    AppState state;
    Framebuffer fb(800, 600);

    // Minimal render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        BeginGUIFrame();
        RenderAppUI(state, fb);
        EndGUIFrame(window);
    }

    ShutdownGraphics(window);
    return 0;
}