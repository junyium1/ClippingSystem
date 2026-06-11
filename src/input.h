#pragma once
#include "app_state.h"
#include "imgui.h"

// Gere les interactions souris sur le canevas (selection, ajout, glisser).
void HandleInput(AppState& state, ImVec2 canvasPos, ImVec2 canvasSize);
