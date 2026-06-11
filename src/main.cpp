#include "platform.h"
#include "app_state.h"
#include "framebuffer.h"
#include "ui.h"

// Point d'entree : cree la fenetre, la scene de depart, et lance la boucle principale.
int main()
{
    GLFWwindow* window = InitializeGraphics();
    if (!window) return -1;

    AppState state;
    Framebuffer fb(800, 600);

    state.clipWindows.push_back({{ {300,100},{600,100},{800,300},{700,500},{300,500} }});
    state.windowVisible.push_back(true);

    state.subjectPolys.push_back({{ {200,200},{500,150},{600,400},{400,600},{150,400} }});
    state.selectedTriangles.push_back({});
    state.polyVisible.push_back(true);

    state.subjectSegments.push_back({{150,250},{750,350}});
    state.segmentVisible.push_back(true);

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
