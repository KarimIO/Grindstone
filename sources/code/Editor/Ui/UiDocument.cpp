#include "UiDocument.hpp"
#include "UiNodePanel.hpp"
#include "UiNodeText.hpp"
using namespace Grindstone;

void Ui::Document::setRoot(Ui::Node* node) {
	root_ = node;
}

Ui::Node* Ui::Document::createPanel(const char* tag, Ui::Node* parent) {
	Ui::Panel* node = new Ui::Panel();
	node->tag_ = tag;
	parent->nodes_.push_back(node);
	return node;
}

Ui::Node* Ui::Document::createText(const char* text, Ui::Node* parent) {
	Ui::Text* node = new Ui::Text();
	node->text_ = text;
	parent->nodes_.push_back(node);
	return node;
}
