#include "Manager.hpp"
#include "../EngineCore.hpp"
#include "../ECS/Core.hpp"
using namespace Grindstone;

SceneManager::SceneManager(EngineCore* core) : engine_core_(core) {
}


Scene* SceneManager::loadScene(const char *path) {
    auto s = new Scene();
    s->load(path);
    scenes_[path] = s;

    engine_core_->getEcsCore()->registerController(*s->getECS());

    return s;
}

Scene* SceneManager::addEmptyScene(const char *name) {
    auto s = new Scene();
    scenes_[name] = s;

    engine_core_->getEcsCore()->registerController(*s->getECS());

    return s;
}