#pragma once
#include <GLFW/glfw3.h>

// Cree la fenetre, le contexte OpenGL et l'interface ImGui.
GLFWwindow* InitializeGraphics();

// Detruit l'interface ImGui et ferme la fenetre.
void ShutdownGraphics(GLFWwindow* window);

// Demarre une nouvelle frame ImGui.
void BeginGUIFrame();

// Dessine la frame ImGui et l'affiche a l'ecran.
void EndGUIFrame(GLFWwindow* window);
