#include <Engine/Core/Engine.hpp>

#include <Editor/CoreEditor/EditorManager.hpp>
#include <Editor/MaterialEditor/MaterialEditor.hpp>
#include <Editor/SceneEditor/SceneEditor.hpp>
#include <WindowModule/Win32Window.hpp>

#include <chrono>

#include <Engine/Core/Space.hpp>
#include <Engine/UI/UICanvas.hpp>
#include <Engine/UI/UIPanel.hpp>

#include <ExecutableCommon/DLLEngine.hpp>

namespace Grindstone {
    EditorManager::EditorManager() {
    }

    EditorManager::~EditorManager() {
        cleanup();
    }

    void EditorManager::initialize() {
        GRIND_LOG("Initializing Editor...");

        //main_imgui_window_class_ = new ImGuiWindowClass();
        //main_imgui_window_class_->ClassId = 1;

        /*{
            GRIND_PROFILE_SCOPE("Create GraphicsWrapper");
            Grindstone::GraphicsAPI::InstanceCreateInfo createInfo;
            createInfo.width = 1024; // settings->resolution_x_;
            createInfo.height = 768; // settings->resolution_y_;
            createInfo.vsync = true; // settings->vsync_;
            createInfo.inputInterface = &imgui_manager_; // (InputInterface*)engine.getInputManager();
            createInfo.title = "Grindstone";
#ifdef NDEBUG
            createInfo.debug = false;
#else
            createInfo.debug = true;
#endif

            graphics_dll_.setup();
            graphics_wrapper_ = graphics_dll_.createWrapper(createInfo);
        }*/

        /*WindowCreateInfo win_ci;
        win_ci.fullscreen = WindowFullscreenMode::Windowed;
        win_ci.width = 800;
        win_ci.height = 600;
        win_ci.title = "Grind";

        BaseWindow* windows_ = new Win32Window();
        windows_->initialize(win_ci);
        windows_->show();

        while (true) {
            unsigned int left, right, top, bottom;
            windows_->getWindowRect(left, right, top, bottom);
            std::cout << left << ", " << right << ", " << top << ", " << bottom << "\n";
            windows_->swapBuffers();
        }*/

        /*engine_ = new Engine();
        engine_->initialize();
        engine_->run();
        engine_->shutdown();*/
        dll_engine_ = new DLLEngine();
        dll_engine_->initializeDLL();
        engine_ = (Engine *)dll_engine_->launchEngine();
        WindowCreateInfo windowCreateInfo;
        windowCreateInfo.fullscreen = WindowFullscreenMode::Windowed;
        windowCreateInfo.title = "Grindstone";
        windowCreateInfo.width = 1024;
        windowCreateInfo.height = 768;
        windowCreateInfo.input_interface = nullptr;
        auto window = new Win32Window();
        window->initialize(windowCreateInfo);
        engine_->initialize(window);
        //BaseWindow *win = engine_->getWindow();

        //UIPanel* root = new UIPanel("../assets/materials/ui.gmat");
        //Space *space = engine_->addSpace("Editor");
        //space->createObject("UI");

        // UICanvas* canvas = new UICanvas();
        //canvas->initialize();

        //imgui_manager_.initialize(win);
        //addEditor(new MaterialEditor("Wow3",  2, this));
        //addEditor(new SceneEditor("Wow2", 3, this));

        Space* space = engine_->addSpace("Main Game");
        GameObject &object = space->createObject("UI");
        SubSystem *transf_subsys = space->getSubsystem(COMPONENT_TRANSFORM);
        object.createComponent(COMPONENT_TRANSFORM);
        object.createComponent(COMPONENT_UI);
        

        window->show();
        window->setWindowFocus();
        
		begin = begin_frame = std::chrono::steady_clock::now();
        GRIND_LOG("Editor Initialized.");
    }

	void EditorManager::calculateTime() {
		end_frame = std::chrono::steady_clock::now();
		time_current_ = std::chrono::duration_cast<std::chrono::microseconds>(end_frame - begin).count() / 1000000.0;
		time_delta_ = std::chrono::duration_cast<std::chrono::microseconds>(end_frame - begin_frame).count() / 1000000.0;
		begin_frame = end_frame;

	}

	double EditorManager::getTimeCurrent() {
		return time_current_;
	}

	double EditorManager::getTimeDelta() {
		return time_delta_;
	}

    void EditorManager::run() {
		while (!engine_->shouldQuit()) {
            engine_->run();
			calculateTime();
            //graphics_wrapper_->Clear(Grindstone::GraphicsAPI::ClearMode::Both);
            //graphics_wrapper_->HandleEvents();

            //imgui_manager_.newFrame(time_current_);
            
            if (editors_.size() == 0) {
                dashboard();
            }
            else {
                for (auto& editor : editors_) {
                    editor->run();
                }
            }
        }
    }

    void EditorManager::cleanup() {
        for (auto& editor : editors_) {
            editor->cleanup();
        }
    }

    void EditorManager::addEditor(EditorInstance* editor) {
        editors_.emplace_back(editor);
    }

	void EditorManager::openFile(std::string path) {
		size_t pos = path.find_last_of('.');
		std::string extension = path.substr(pos + 1);

		if (extension == "gmf") {
			// Model File
		}
		else if (extension == "json") {
			// Scene File
		}
		else if (extension == "gmat") {
			// Material File
			addEditor(new MaterialEditor(path, 3, this));
		}
		else {
			// Image
		}
		/* else { Unhandled } */
	}

    void EditorManager::dashboard() {
    }
}
