#include "UiRenderTree.hpp"
#include "UiDocument.hpp"
#include "UiRenderer.hpp"
using namespace Grindstone;

bool Ui::RenderTree::initialize(Ui::Document& doc) {
	auto node = doc.getRoot();
	node->children_.resize(1);
	root_node_ = initializeNode(node, nullptr);

	return false;
}

Ui::RenderTree::Node* Ui::RenderTree::initializeNode(Ui::Node* node, Ui::RenderTree::Node* rt_node_parent) {
	Ui::RenderTree::Node* rt_node	 = new Ui::RenderTree::Node();
	rt_node_parent->children_.push_back(rt_node);

	auto& layout = rt_node->layout_;

	layout.left_ = rand() / RAND_MAX;
	layout.right_ = rand() / RAND_MAX;
	layout.up_ = rand() / RAND_MAX;
	layout.down_ = rand() / RAND_MAX;

	rt_node->children_.resize(rt_node_parent->children_.size());
	for (auto child : node->children_) {
		initializeNode(child, rt_node);
	}

	return rt_node;
}

void Ui::RenderTree::render() {
	renderNode(root_node_);
}

void Ui::RenderTree::renderNode(Ui::RenderTree::Node* rt_node) {
	auto& layout = rt_node->layout_;

	renderer_->addQuad(layout.left_, layout.up_, layout.right_, layout.down_);

	for (auto child : rt_node->children_) {
		renderNode(child);
	}
}
