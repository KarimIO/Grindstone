#include "AssetBrowser.hpp"
#include "../Ui/UiDocument.hpp"
using namespace Grindstone;

const char* DEFAULT_PATH = "../assets/";

Editor::AssetBrowser::AssetBrowser() : path_(DEFAULT_PATH) {
	
}

bool Editor::AssetBrowser::initialize() {
	Ui::Document myDocument;
	Ui::Node* node = myDocument.createPanel("panel", nullptr);
	myDocument.setRoot(node);
	
	return false;
}

void Editor::AssetBrowser::update() {
}
