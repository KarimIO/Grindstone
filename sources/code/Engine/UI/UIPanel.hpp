#pragma once

#include <Engine/UI/UINode.hpp>
#include <Engine/Rendering/Renderer2D.hpp>
#include <Engine/AssetManagers/AssetReferences.hpp>

class UIPanel : public UINode {
public:
    UIPanel(std::string material_path);
    void updateLayout();
    ~UIPanel();
private:
    Renderer2D* r2d_;
    unsigned int quad_id_;
};
