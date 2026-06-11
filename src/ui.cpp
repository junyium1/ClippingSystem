#include "ui.h"
#include "input.h"
#include "renderer.h"
#include "fractal.h"

static const float kToolbarWidth = 130.0f;
static const float kPropertiesWidth = 360.0f;
static const float kStatusBarHeight = 30.0f;
static const float kTopBarHeight = 40.0f;

// Renvoie le nom affiche d'un outil.
static const char* ToolLabel(EditTool tool, bool advanced)
{
    if (advanced)
    {
        switch (tool)
        {
            case EditTool::Select:         return "Selection";
            case EditTool::AddWindow:      return "Ajouter une fenetre";
            case EditTool::AddPolygon:     return "Ajouter un polygone";
            case EditTool::AddSegment:     return "Ajouter un segment";
            case EditTool::SeedFill:       return "Germe (remplissage partiel)";
            case EditTool::TriangleSelect: return "Triangles (remplissage partiel)";
        }
        return "";
    }

    switch (tool)
    {
        case EditTool::Select:         return "Selection";
        case EditTool::AddWindow:      return "Ajouter un masque";
        case EditTool::AddPolygon:     return "Ajouter une forme";
        case EditTool::AddSegment:     return "Ajouter une ligne";
        case EditTool::SeedFill:       return "Pinceau de remplissage";
        case EditTool::TriangleSelect: return "Retoucher des zones";
    }
    return "";
}

// Renvoie le texte d'aide d'un outil.
static const char* ToolHint(EditTool tool, bool advanced)
{
    if (advanced)
    {
        switch (tool)
        {
            case EditTool::Select:         return "Clic gauche : deplacer un sommet ou une forme entiere.\nClic droit : supprimer la forme sous le curseur.";
            case EditTool::AddWindow:      return "Clic gauche sur le canevas : placer une nouvelle fenetre de decoupe.";
            case EditTool::AddPolygon:     return "Clic gauche sur le canevas : placer un nouveau polygone a decouper.";
            case EditTool::AddSegment:     return "Clic gauche + glisser : tracer un segment a decouper (Cyrus-Beck).";
            case EditTool::SeedFill:       return "Clic gauche sur le canevas : poser une graine de remplissage partiel.";
            case EditTool::TriangleSelect: return "Clic gauche sur un triangle : l'ajouter ou le retirer du remplissage partiel.";
        }
        return "";
    }

    switch (tool)
    {
        case EditTool::Select:         return "Clic gauche : deplacer un sommet ou une forme entiere.\nClic droit : supprimer la forme sous le curseur.";
        case EditTool::AddWindow:      return "Clic gauche sur le canevas : placer un nouveau masque de decoupe.";
        case EditTool::AddPolygon:     return "Clic gauche sur le canevas : placer une nouvelle forme.";
        case EditTool::AddSegment:     return "Clic gauche + glisser : tracer une ligne a decouper.";
        case EditTool::SeedFill:       return "Clic gauche sur le canevas : remplir la zone sous le curseur.";
        case EditTool::TriangleSelect: return "Clic gauche sur une zone : la remplir ou la vider.";
    }
    return "";
}

// Dessine un bouton d'outil, surligne s'il est actif.
static void ToolButton(AppState& state, const char* label, EditTool tool)
{
    bool active = (state.activeTool == tool);

    if (active)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.45f, 0.85f, 1.0f));

    if (ImGui::Button(label, ImVec2(-1, 45)))
        state.activeTool = tool;

    if (active)
        ImGui::PopStyleColor();

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", ToolHint(tool, state.advancedMode));

    ImGui::Spacing();
}

// Dessine la barre du haut (titre, zoom, reinitialisation de la vue, mode).
static void RenderTopBar(AppState& state)
{
    ImGui::BeginChild("TopBar", ImVec2(0, kTopBarHeight), true);

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Fenetrage et Remplissage de Polygones");

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(20.0f, 0));

    ImGui::SameLine();
    ImGui::Checkbox("Avance", &state.advancedMode);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Affiche les algorithmes et reglages techniques.");

    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), state.advancedMode ? "Mode : Pedagogique" : "Mode : Produit fini");

    ImGui::SameLine(ImGui::GetWindowWidth() - 240.0f);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Zoom : %d %%", (int)(state.camera.zoom * 100.0f));

    ImGui::SameLine();
    if (ImGui::Button("Reinitialiser la vue"))
    {
        state.camera.zoom = 1.0f;
        state.camera.offset = {0.0f, 0.0f};
    }

    ImGui::EndChild();
}

// Dessine la barre d'outils verticale a gauche.
static void RenderToolbar(AppState& state)
{
    ImGui::BeginChild("Toolbar", ImVec2(kToolbarWidth, -kStatusBarHeight), true);

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.55f, 1.0f), "OUTILS");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));

    ToolButton(state, "Selection", EditTool::Select);

    if (state.advancedMode)
    {
        ToolButton(state, "+ Fenetre", EditTool::AddWindow);
        ToolButton(state, "+ Polygone", EditTool::AddPolygon);

        if (state.currentTool == ToolMode::CyrusBeck)
            ToolButton(state, "+ Segment", EditTool::AddSegment);
    }
    else
    {
        ToolButton(state, "+ Masque", EditTool::AddWindow);
        ToolButton(state, "+ Forme", EditTool::AddPolygon);
        ToolButton(state, "+ Ligne", EditTool::AddSegment);
    }

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.55f, 1.0f), "SOMMETS");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##sides", &state.newShapeSides, 3, 12);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Nombre de sommets des prochains\nmasques / formes ajoutes.");

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));

    if (state.advancedMode)
    {
        ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.55f, 1.0f), "REMPLISSAGE\nPARTIEL");
        ImGui::Dummy(ImVec2(0, 5));
        ToolButton(state, "Germe", EditTool::SeedFill);
        ToolButton(state, "Triangles", EditTool::TriangleSelect);
    }
    else
    {
        ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.55f, 1.0f), "RETOUCHE");
        ImGui::Dummy(ImVec2(0, 5));
        ToolButton(state, "Pinceau", EditTool::SeedFill);
        ToolButton(state, "Zones", EditTool::TriangleSelect);
    }

    ImGui::EndChild();
}

// Supprime tous les masques, formes et lignes de la scene.
static void ClearScene(AppState& state)
{
    state.clipWindows.clear();
    state.subjectPolys.clear();
    state.subjectSegments.clear();
    state.selectedTriangles.clear();
    state.seeds.clear();
    state.windowVisible.clear();
    state.polyVisible.clear();
    state.segmentVisible.clear();
    state.activeTool = EditTool::Select;
}

// Dessine l'onglet de choix de l'algorithme et le resume de la scene.
static void RenderModeTab(AppState& state)
{
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::TextWrapped("Choisissez l'algorithme de decoupage applique aux formes sujets.");
    ImGui::Dummy(ImVec2(0, 8));

    if (ImGui::RadioButton("Sutherland-Hodgman (polygones)", state.currentTool == ToolMode::SutherlandHodgman))
        state.currentTool = ToolMode::SutherlandHodgman;
    ImGui::TextWrapped("Decoupe un polygone quelconque (convexe, concave, croise) par une fenetre convexe.");
    ImGui::Dummy(ImVec2(0, 8));

    if (ImGui::RadioButton("Cyrus-Beck (segments)", state.currentTool == ToolMode::CyrusBeck))
        state.currentTool = ToolMode::CyrusBeck;
    ImGui::TextWrapped("Decoupe un segment, et un polygone, par une fenetre convexe avec la methode parametrique de Cyrus-Beck.");

    if (state.currentTool != ToolMode::CyrusBeck && state.activeTool == EditTool::AddSegment)
        state.activeTool = EditTool::Select;

    ImGui::Dummy(ImVec2(0, 15));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 8));

    ImGui::Text("Scene actuelle");
    ImGui::BulletText("%d fenetre(s) de decoupe", (int)state.clipWindows.size());
    ImGui::BulletText("%d polygone(s) sujet(s)", (int)state.subjectPolys.size());
    ImGui::BulletText("%d segment(s) sujet(s)", (int)state.subjectSegments.size());

    ImGui::Dummy(ImVec2(0, 15));
    if (ImGui::Button("Tout effacer", ImVec2(-1, 35)))
        ClearScene(state);
}

// Dessine l'onglet de choix des couleurs.
static void RenderColorsTab(AppState& state)
{
    const char* windowLabel = state.advancedMode ? "Fenetres" : "Masques";
    const char* subjectLabel = state.advancedMode ? "Polygones/Segments" : "Formes/Lignes";
    const char* clippedLabel = state.advancedMode ? "Resultats decoupes" : "Resultat";

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::ColorEdit4(windowLabel, (float*)&state.colorMenuPoly);
    ImGui::Spacing();
    ImGui::ColorEdit4(subjectLabel, (float*)&state.colorSubject);
    ImGui::Spacing();
    ImGui::ColorEdit4(clippedLabel, (float*)&state.colorClipped);
    ImGui::Spacing();
    ImGui::ColorEdit4("Remplissage", (float*)&state.colorFill);
    ImGui::Spacing();
    ImGui::ColorEdit4("Fond d'ecran", (float*)&state.colorBg);
}

// Dessine les controles de rotation, echelle, cisaillement et translation.
static void RenderTransformControls(AppState& state)
{
    ImGui::SliderFloat("Angle (deg)", &state.transformAngleDeg, -180.0f, 180.0f);
    if (ImGui::Button("Appliquer rotation", ImVec2(-1, 30)))
    {
        float angleRad = state.transformAngleDeg * (3.14159265f / 180.0f);
        for (Polygon& window : state.clipWindows)
            window = RotatePolygon(window, angleRad);
    }

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::SliderFloat("Echelle X", &state.transformScaleX, 0.1f, 3.0f);
    ImGui::SliderFloat("Echelle Y", &state.transformScaleY, 0.1f, 3.0f);
    if (ImGui::Button("Appliquer echelle", ImVec2(-1, 30)))
    {
        for (Polygon& window : state.clipWindows)
            window = ScalePolygon(window, state.transformScaleX, state.transformScaleY);
    }

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::SliderFloat("Cisaillement X", &state.transformShearX, -2.0f, 2.0f);
    ImGui::SliderFloat("Cisaillement Y", &state.transformShearY, -2.0f, 2.0f);
    if (ImGui::Button("Appliquer cisaillement", ImVec2(-1, 30)))
    {
        for (Polygon& window : state.clipWindows)
            window = ShearPolygon(window, state.transformShearX, state.transformShearY);
    }

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::SliderFloat("Translation X", &state.transformTranslateX, -200.0f, 200.0f);
    ImGui::SliderFloat("Translation Y", &state.transformTranslateY, -200.0f, 200.0f);
    if (ImGui::Button("Appliquer translation", ImVec2(-1, 30)))
    {
        for (Polygon& window : state.clipWindows)
            window = TranslatePolygon(window, state.transformTranslateX, state.transformTranslateY);
    }
}

// Dessine l'onglet des fenetres de decoupe et de leurs transformations.
static void RenderWindowsTab(AppState& state)
{
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Checkbox("Ear Cutting (fenetres concaves)", &state.enableEarCutting);
    ImGui::TextWrapped("Decompose une fenetre de decoupe concave en triangles (Cyrus-Beck et Sutherland-Hodgman).");

    ImGui::Dummy(ImVec2(0, 12));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Text("Traitements graphiques (fenetres)");
    ImGui::TextWrapped("S'appliquent a toutes les fenetres de decoupe, autour de leur centre.");
    ImGui::Dummy(ImVec2(0, 8));

    RenderTransformControls(state);
}

// Dessine l'onglet de transformation des masques (interface simplifiee).
static void RenderTransformTab(AppState& state)
{
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Text("Transformer les masques");
    ImGui::TextWrapped("S'applique a tous les masques de la scene, autour de leur centre.");
    ImGui::Dummy(ImVec2(0, 8));

    RenderTransformControls(state);
}

// Dessine l'onglet de remplissage.
static void RenderFillTab(AppState& state)
{
    ImGui::Dummy(ImVec2(0, 5));

    if (state.advancedMode)
    {
        ImGui::Checkbox("Remplissage total (LCA)", &state.enableLCA);
        ImGui::TextWrapped("Remplit par balayage (LCA) les polygones sujets et leurs decoupes.");
    }
    else
    {
        ImGui::Checkbox("Remplir les formes", &state.enableLCA);
        ImGui::TextWrapped("Remplit l'interieur des formes et de leurs decoupes.");
    }

    ImGui::Dummy(ImVec2(0, 12));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));

    if (state.advancedMode)
    {
        ImGui::Text("Regle de remplissage (polygones croises)");
        if (ImGui::RadioButton("Pair/Impair", state.fillRule == FillRule::EvenOdd))
            state.fillRule = FillRule::EvenOdd;
        ImGui::TextWrapped("Compte le nombre d'intersections de chaque scanline.");

        if (ImGui::RadioButton("Enroulement non nul", state.fillRule == FillRule::NonZero))
            state.fillRule = FillRule::NonZero;
        ImGui::TextWrapped("Compte le nombre d'enroulement des aretes (sens de parcours).");

        ImGui::Dummy(ImVec2(0, 12));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Text("Remplissage partiel par germe");
        ImGui::TextWrapped("Outil 'Germe' : cliquez dans une zone fermee pour la remplir par propagation.");
    }
    else
    {
        ImGui::Text("Pinceau de remplissage");
        ImGui::TextWrapped("Outil 'Pinceau' : cliquez dans une zone fermee pour la remplir.");
    }

    if (ImGui::Button(state.advancedMode ? "Effacer les graines" : "Effacer les remplissages", ImVec2(-1, 30)))
        state.seeds.clear();

    ImGui::Dummy(ImVec2(0, 12));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 5));

    if (state.advancedMode)
    {
        ImGui::Text("Remplissage partiel par triangles");
        ImGui::TextWrapped("Outil 'Triangles' : le polygone est subdivise en triangles, cliquez sur un triangle pour le remplir/vider.");
    }
    else
    {
        ImGui::Text("Zones");
        ImGui::TextWrapped("Outil 'Zones' : cliquez sur une partie de la forme pour la remplir ou la vider.");
    }

    if (ImGui::Button(state.advancedMode ? "Effacer la selection de triangles" : "Effacer la selection de zones", ImVec2(-1, 30)))
    {
        for (std::vector<int>& sel : state.selectedTriangles)
            sel.clear();
    }
}

// Renvoie le nom affiche d'un type de fractale.
static const char* FractalTypeName(FractalType type, bool advanced)
{
    if (advanced)
    {
        switch (type)
        {
            case FractalType::KochSnowflake:     return "Flocon de Koch (triangle, convexe)";
            case FractalType::KochAntiSnowflake: return "Anti-flocon de Koch (triangle, concave)";
            case FractalType::KochSquareIsland:  return "Ile de Koch (carre, convexe)";
            case FractalType::KochSquareCross:   return "Croix de Koch (carre, concave)";
        }
        return "";
    }

    switch (type)
    {
        case FractalType::KochSnowflake:     return "Flocon";
        case FractalType::KochAntiSnowflake: return "Anti-flocon";
        case FractalType::KochSquareIsland:  return "Ile";
        case FractalType::KochSquareCross:   return "Croix";
    }
    return "";
}

// Dessine le selecteur de motif et le bouton pour l'ajouter a la scene.
static void RenderFractalGallery(AppState& state)
{
    if (ImGui::BeginCombo("Type", FractalTypeName(state.fractalType, state.advancedMode)))
    {
        for (int i = 0; i < 4; i++)
        {
            FractalType candidate = (FractalType)i;
            bool selected = (state.fractalType == candidate);
            if (ImGui::Selectable(FractalTypeName(candidate, state.advancedMode), selected))
                state.fractalType = candidate;
        }
        ImGui::EndCombo();
    }

    ImGui::SliderInt("Iterations", &state.fractalIterations, 0, 5);

    int verts = (state.fractalType == FractalType::KochSnowflake || state.fractalType == FractalType::KochAntiSnowflake) ? 3 : 4;
    for (int i = 0; i < state.fractalIterations; i++)
        verts *= 4;
    ImGui::TextDisabled("%d sommets generes", verts);

    ImGui::Dummy(ImVec2(0, 8));

    if (ImGui::Button("+ Ajouter le motif", ImVec2(-1, 35)))
    {
        state.subjectPolys.push_back(GenerateFractal(state.fractalType, {400, 300}, 180.0f, state.fractalIterations));
        state.selectedTriangles.push_back({});
        state.polyVisible.push_back(true);
        state.activeTool = EditTool::Select;
    }
}

// Dessine l'onglet de generation de fractales.
static void RenderFractalsTab(AppState& state)
{
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::TextWrapped("Genere une courbe fractale fermee comme polygone sujet : elle peut ensuite etre deplacee, decoupee et remplie comme n'importe quel autre polygone.");
    ImGui::Dummy(ImVec2(0, 10));

    RenderFractalGallery(state);
}

// Dessine l'onglet "Formes" : galerie de motifs et resume de la scene.
static void RenderShapesTab(AppState& state)
{
    ImGui::Dummy(ImVec2(0, 5));
    ImGui::TextWrapped("Utilisez les outils a gauche pour ajouter des masques, des formes ou des lignes. Choisissez ici un motif a generer.");
    ImGui::Dummy(ImVec2(0, 10));

    RenderFractalGallery(state);

    ImGui::Dummy(ImVec2(0, 15));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 8));

    ImGui::Text("Scene actuelle");
    ImGui::BulletText("%d masque(s)", (int)state.clipWindows.size());
    ImGui::BulletText("%d forme(s)", (int)state.subjectPolys.size());
    ImGui::BulletText("%d ligne(s)", (int)state.subjectSegments.size());

    ImGui::Dummy(ImVec2(0, 15));
    if (ImGui::Button("Tout effacer", ImVec2(-1, 35)))
        ClearScene(state);
}

// Centre la camera sur un point donne.
static void FocusCameraOn(AppState& state, Point2D target, ImVec2 canvasSize)
{
    state.camera.offset.x = target.x - (canvasSize.x * 0.5f) / state.camera.zoom;
    state.camera.offset.y = target.y - (canvasSize.y * 0.5f) / state.camera.zoom;
}

// Dessine l'onglet "Calques" (gestionnaire d'objets).
static void RenderLayersTab(AppState& state, ImVec2 canvasSize)
{
    const char* windowHeader = state.advancedMode ? "Fenetres de decoupe" : "Masques";
    const char* windowItem = state.advancedMode ? "Fenetre" : "Masque";
    const char* polyHeader = state.advancedMode ? "Polygones sujets" : "Formes";
    const char* polyItem = state.advancedMode ? "Polygone" : "Forme";
    const char* segHeader = state.advancedMode ? "Segments sujets" : "Lignes";
    const char* segItem = state.advancedMode ? "Segment" : "Ligne";

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::TextWrapped("Gestionnaire d'objets (comme des calques Photoshop). Decochez pour masquer un objet et l'exclure du decoupage. 'Cibler' centre la vue dessus, 'X' le supprime.");
    ImGui::Dummy(ImVec2(0, 10));

    float w = ImGui::GetWindowWidth();

    if (ImGui::CollapsingHeader(windowHeader, ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (state.clipWindows.empty())
            ImGui::TextDisabled("Liste vide.");

        int toDelete = -1;
        for (size_t i = 0; i < state.clipWindows.size() && i < state.windowVisible.size(); i++)
        {
            ImGui::PushID((int)(100 + i));

            bool visible = state.windowVisible[i];
            if (ImGui::Checkbox("##v", &visible))
                state.windowVisible[i] = visible;

            ImGui::SameLine();
            ImGui::Text("%s %d (%d sommets)", windowItem, (int)i + 1, (int)state.clipWindows[i].vertices.size());

            ImGui::SameLine(w - 130.0f);
            if (ImGui::SmallButton("Cibler"))
                FocusCameraOn(state, PolygonCentroid(state.clipWindows[i]), canvasSize);

            ImGui::SameLine(w - 60.0f);
            if (ImGui::SmallButton("X"))
                toDelete = (int)i;

            ImGui::PopID();
        }
        if (toDelete >= 0)
        {
            state.clipWindows.erase(state.clipWindows.begin() + toDelete);
            state.windowVisible.erase(state.windowVisible.begin() + toDelete);
        }
    }

    ImGui::Dummy(ImVec2(0, 8));

    if (ImGui::CollapsingHeader(polyHeader, ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (state.subjectPolys.empty())
            ImGui::TextDisabled("Liste vide.");

        int toDelete = -1;
        for (size_t i = 0; i < state.subjectPolys.size() && i < state.polyVisible.size(); i++)
        {
            ImGui::PushID((int)(1000 + i));

            bool visible = state.polyVisible[i];
            if (ImGui::Checkbox("##v", &visible))
                state.polyVisible[i] = visible;

            ImGui::SameLine();
            int nSel = (i < state.selectedTriangles.size()) ? (int)state.selectedTriangles[i].size() : 0;
            if (nSel > 0)
                ImGui::Text("%s %d (%d sommets, %d tri.)", polyItem, (int)i + 1, (int)state.subjectPolys[i].vertices.size(), nSel);
            else
                ImGui::Text("%s %d (%d sommets)", polyItem, (int)i + 1, (int)state.subjectPolys[i].vertices.size());

            ImGui::SameLine(w - 130.0f);
            if (ImGui::SmallButton("Cibler"))
                FocusCameraOn(state, PolygonCentroid(state.subjectPolys[i]), canvasSize);

            ImGui::SameLine(w - 60.0f);
            if (ImGui::SmallButton("X"))
                toDelete = (int)i;

            ImGui::PopID();
        }
        if (toDelete >= 0)
        {
            state.subjectPolys.erase(state.subjectPolys.begin() + toDelete);
            state.polyVisible.erase(state.polyVisible.begin() + toDelete);
            if ((size_t)toDelete < state.selectedTriangles.size())
                state.selectedTriangles.erase(state.selectedTriangles.begin() + toDelete);
        }
    }

    if (!state.advancedMode || state.currentTool == ToolMode::CyrusBeck)
    {
        ImGui::Dummy(ImVec2(0, 8));

        if (ImGui::CollapsingHeader(segHeader, ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (state.subjectSegments.empty())
                ImGui::TextDisabled("Liste vide.");

            int toDelete = -1;
            for (size_t i = 0; i < state.subjectSegments.size() && i < state.segmentVisible.size(); i++)
            {
                ImGui::PushID((int)(2000 + i));

                bool visible = state.segmentVisible[i];
                if (ImGui::Checkbox("##v", &visible))
                    state.segmentVisible[i] = visible;

                ImGui::SameLine();
                ImGui::Text("%s %d", segItem, (int)i + 1);

                ImGui::SameLine(w - 130.0f);
                if (ImGui::SmallButton("Cibler"))
                {
                    Point2D mid;
                    mid.x = (state.subjectSegments[i].a.x + state.subjectSegments[i].b.x) * 0.5f;
                    mid.y = (state.subjectSegments[i].a.y + state.subjectSegments[i].b.y) * 0.5f;
                    FocusCameraOn(state, mid, canvasSize);
                }

                ImGui::SameLine(w - 60.0f);
                if (ImGui::SmallButton("X"))
                    toDelete = (int)i;

                ImGui::PopID();
            }
            if (toDelete >= 0)
            {
                state.subjectSegments.erase(state.subjectSegments.begin() + toDelete);
                state.segmentVisible.erase(state.segmentVisible.begin() + toDelete);
            }
        }
    }
}

// Dessine le panneau de droite avec ses onglets.
static void RenderPropertiesPanel(AppState& state, ImVec2 canvasSize)
{
    ImGui::BeginChild("Properties", ImVec2(kPropertiesWidth, -kStatusBarHeight), true);

    if (ImGui::BeginTabBar("PropertiesTabs"))
    {
        if (state.advancedMode)
        {
            if (ImGui::BeginTabItem("Mode"))
            {
                RenderModeTab(state);
                ImGui::EndTabItem();
            }
        }
        else
        {
            if (ImGui::BeginTabItem("Formes"))
            {
                RenderShapesTab(state);
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem("Calques"))
        {
            RenderLayersTab(state, canvasSize);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Couleurs"))
        {
            RenderColorsTab(state);
            ImGui::EndTabItem();
        }

        if (state.advancedMode)
        {
            if (ImGui::BeginTabItem("Fenetres"))
            {
                RenderWindowsTab(state);
                ImGui::EndTabItem();
            }
        }
        else
        {
            if (ImGui::BeginTabItem("Transformer"))
            {
                RenderTransformTab(state);
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem("Remplissage"))
        {
            RenderFillTab(state);
            ImGui::EndTabItem();
        }

        if (state.advancedMode)
        {
            if (ImGui::BeginTabItem("Fractales"))
            {
                RenderFractalsTab(state);
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }

    ImGui::EndChild();
}

// Dessine la barre du bas (outil actif et position de la souris).
static void RenderStatusBar(AppState& state)
{
    ImGui::BeginChild("StatusBar", ImVec2(0, 0), true);

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Outil : %s", ToolLabel(state.activeTool, state.advancedMode));
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "  %s", ToolHint(state.activeTool, state.advancedMode));

    if (state.isMouseOverCanvas)
    {
        ImGui::SameLine(ImGui::GetWindowWidth() - 160.0f);
        ImGui::Text("(%.0f, %.0f)", state.lastWorldMousePos.x, state.lastWorldMousePos.y);
    }

    ImGui::EndChild();
}

// Dessine toute l'interface (barre du haut, outils, canevas, panneaux, barre du bas).
void RenderAppUI(AppState& state, Framebuffer& fb)
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    RenderTopBar(state);

    RenderToolbar(state);
    ImGui::SameLine();

    ImGui::BeginChild("Canvas", ImVec2(-kPropertiesWidth, -kStatusBarHeight), true, ImGuiWindowFlags_NoScrollbar);
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    fb.Resize((int)canvasSize.x, (int)canvasSize.y);

    ImGui::InvisibleButton("canvas_btn", canvasSize);
    HandleInput(state, canvasPos, canvasSize);

    RenderCanvas(state, fb);

    ImGui::SetCursorScreenPos(canvasPos);
    ImGui::Image((void*)(intptr_t)fb.textureID, canvasSize);
    ImGui::EndChild();

    ImGui::SameLine();
    RenderPropertiesPanel(state, canvasSize);

    RenderStatusBar(state);

    ImGui::End();
}
