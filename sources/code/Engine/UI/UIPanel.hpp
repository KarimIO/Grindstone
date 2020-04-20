#pragma once

#include <Engine/UI/UINode.hpp>

#include <Engine/AssetManagers/AssetReferences.hpp>

class UIPanel : public UINode {
public:
    UIPanel(std::string material_path);
    ~UIPanel();
private:
    unsigned int quad_id_;
};
