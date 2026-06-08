#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "geometry.h"
#include "CyrusBeck.h"
#include "SutherlandHodgman.h"
#include <vector>
#include <cmath>
#include <algorithm>

float distance(Point2D a, Point2D b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

enum class ToolMode {
    CyrusBeck,
    SutherlandHodgman
};

int main() {
    if (!glfwInit()) return -1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Moteur de Rendu 2D - Clipping", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- ÉTAT GLOBAL ---
    ToolMode currentTool = ToolMode::CyrusBeck;
    int draggingPointIndex = -1;

    // Couleurs
    ImVec4 colorMenuPoly = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
    ImVec4 colorSubject   = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
    ImVec4 colorClipped   = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);

    // Fenêtre de découpe (Sens Horaire)
    Polygon clipPoly;
    clipPoly.vertices = {
        {300.0f, 100.0f}, {600.0f, 100.0f}, {800.0f, 300.0f},
        {700.0f, 500.0f}, {300.0f, 500.0f}
    };

    // Données Cyrus-Beck
    Point2D cb_p0 = { 200.0f, 200.0f };
    Point2D cb_p1 = { 700.0f, 400.0f };

    // Données Sutherland-Hodgman
    Polygon subjectPoly;
    subjectPoly.vertices = {
        {200.0f, 200.0f}, {500.0f, 150.0f}, {600.0f, 400.0f},
        {400.0f, 600.0f}, {150.0f, 400.0f}
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- FENÊTRE MAÎTRE ---
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("MainWorkspace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        // --- 1. PANNEAU DE CONTRÔLE ---
        ImGui::BeginChild("PanelGaucher", ImVec2(300, 0), true);

        if (ImGui::BeginTabBar("OutilsAlgorithmiques")) {

            if (ImGui::BeginTabItem("Cyrus-Beck")) {
                if (currentTool != ToolMode::CyrusBeck) {
                    currentTool = ToolMode::CyrusBeck;
                    draggingPointIndex = -1; // Sécurité au switch d'onglet
                }
                ImGui::Spacing();
                ImGui::Text("Découpage de segment");
                ImGui::Separator();
                ImGui::ColorEdit4("Fenêtre", (float*)&colorMenuPoly);
                ImGui::ColorEdit4("Segment", (float*)&colorSubject);
                ImGui::ColorEdit4("Résultat", (float*)&colorClipped);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Sutherland-H.")) {
                if (currentTool != ToolMode::SutherlandHodgman) {
                    currentTool = ToolMode::SutherlandHodgman;
                    draggingPointIndex = -1; // Sécurité au switch d'onglet
                }
                ImGui::Spacing();
                ImGui::Text("Découpage de polygone");
                ImGui::Separator();
                ImGui::ColorEdit4("Fenêtre", (float*)&colorMenuPoly);
                ImGui::ColorEdit4("Polygone Cible", (float*)&colorSubject);
                ImGui::ColorEdit4("Polygone Découpé", (float*)&colorClipped);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::EndChild();
        ImGui::SameLine();

        // --- 2. CANVAS DE RENDU ---
        ImGui::BeginChild("CanvasDroit", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        ImGui::InvisibleButton("canvas_invisible", canvas_size);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        auto toScreen = [&](Point2D p) { return ImVec2(canvas_p0.x + p.x, canvas_p0.y + p.y); };
        auto toCanvas = [&](ImVec2 p) { return Point2D{p.x - canvas_p0.x, p.y - canvas_p0.y}; };

        Point2D mousePoint = toCanvas(io.MousePos);
        mousePoint.x = std::max(0.0f, std::min(mousePoint.x, canvas_size.x));
        mousePoint.y = std::max(0.0f, std::min(mousePoint.y, canvas_size.y));

        bool is_hovered = ImGui::IsItemHovered();

        // --- GESTION DE LA SOURIS ---
        if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (currentTool == ToolMode::CyrusBeck) {
                if (distance(mousePoint, cb_p0) < 15.0f) draggingPointIndex = 0;
                else if (distance(mousePoint, cb_p1) < 15.0f) draggingPointIndex = 1;
            }
            else if (currentTool == ToolMode::SutherlandHodgman) {
                for (size_t i = 0; i < subjectPoly.vertices.size(); i++) {
                    if (distance(mousePoint, subjectPoly.vertices[i]) < 15.0f) {
                        draggingPointIndex = (int)i;
                        break;
                    }
                }
            }
        }

        if (!io.MouseDown[0]) draggingPointIndex = -1;

        if (draggingPointIndex != -1) {
            if (currentTool == ToolMode::CyrusBeck) {
                if (draggingPointIndex == 0) cb_p0 = mousePoint;
                else if (draggingPointIndex == 1) cb_p1 = mousePoint;
            }
            else if (currentTool == ToolMode::SutherlandHodgman) {
                subjectPoly.vertices[draggingPointIndex] = mousePoint;
            }
        }

        // --- DESSIN DU CANVAS ---
        // Fond
        draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y), IM_COL32(30, 30, 30, 255));

        // 1. Tracé de la Fenêtre de découpe
        ImU32 polyCol = ImColor(colorMenuPoly);
        for (size_t i = 0; i < clipPoly.vertices.size(); i++) {
            Point2D v1 = clipPoly.vertices[i];
            Point2D v2 = clipPoly.vertices[(i + 1) % clipPoly.vertices.size()];
            draw_list->AddLine(toScreen(v1), toScreen(v2), polyCol, 2.0f);
        }

        // 2. Tracé selon l'outil (Blocs STRICTEMENT séparés)
        ImU32 subjCol = ImColor(colorSubject);
        ImU32 clipCol = ImColor(colorClipped);

        if (currentTool == ToolMode::CyrusBeck) {

            // Dessin Segment
            draw_list->AddLine(toScreen(cb_p0), toScreen(cb_p1), subjCol, 1.0f);
            draw_list->AddCircleFilled(toScreen(cb_p0), 6.0f, subjCol);
            draw_list->AddCircleFilled(toScreen(cb_p1), 6.0f, subjCol);

            // Algorithme
            auto clipped = CyrusBeckClipper::clipLine(cb_p0, cb_p1, clipPoly);
            if (clipped.has_value()) {
                draw_list->AddLine(toScreen(clipped->first), toScreen(clipped->second), clipCol, 4.0f);
                draw_list->AddCircleFilled(toScreen(clipped->first), 5.0f, clipCol);
                draw_list->AddCircleFilled(toScreen(clipped->second), 5.0f, clipCol);
            }

        } else if (currentTool == ToolMode::SutherlandHodgman) {

            // Dessin Polygone Cible (Lignes fines rouges)
            for (size_t i = 0; i < subjectPoly.vertices.size(); i++) {
                Point2D v1 = subjectPoly.vertices[i];
                Point2D v2 = subjectPoly.vertices[(i + 1) % subjectPoly.vertices.size()];
                draw_list->AddLine(toScreen(v1), toScreen(v2), subjCol, 1.0f);
                draw_list->AddCircleFilled(toScreen(v1), 6.0f, subjCol);
            }

            // Application de l'algorithme Sutherland-Hodgman
            Polygon clippedPoly = SutherlandHodgmanClipper::clipPolygon(subjectPoly, clipPoly);

            // Dessin du Résultat Découpé (Lignes épaisses vertes)
            if (clippedPoly.vertices.size() >= 2) {
                for (size_t i = 0; i < clippedPoly.vertices.size(); i++) {
                    Point2D v1 = clippedPoly.vertices[i];
                    Point2D v2 = clippedPoly.vertices[(i + 1) % clippedPoly.vertices.size()];
                    draw_list->AddLine(toScreen(v1), toScreen(v2), clipCol, 4.0f);
                    draw_list->AddCircleFilled(toScreen(v1), 5.0f, clipCol);
                }
            }
        }

        ImGui::EndChild();
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}