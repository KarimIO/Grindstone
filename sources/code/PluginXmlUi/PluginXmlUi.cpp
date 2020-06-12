#include <iostream>
#include <map>

#include "Stylesheet.hpp"
#include "../../deps/pugixml/src/pugixml.hpp"

namespace Grindstone {
    namespace PluginXmlUi {
        class UiElement {
        public:
            UiElement(const char *tag, const char* name) : name_(name), tag_(tag) {}
            static UiElement* create(const char* name, const char* tag) {
                return new UiElement(name, tag);
            }
        public:
            std::string name_;
            std::string tag_;
            std::vector<UiElement*> children_;
        };

        using UiElementFactoryFn = UiElement * (*)(const char*, const char*);

        class UiVisualElement : UiElement {
        public:
            UiVisualElement(const char *tag, const char* name) : UiElement(tag, name) {};
            static UiElement *create(const char* tag, const char* name) {
                return new UiVisualElement(name, tag);
            }
        };

        class UiStylesheet : UiElement {
        public:
            UiStylesheet(const char *tag, const char* name) : UiElement(tag, name) {};
            static UiElement* create(const char* tag, const char* name) {
                return new UiStylesheet(name, tag);
            }

            void loadStylesheet() {
                Grindstone::PluginXmlUi::Stylesheet stylesheet;
                if (!stylesheet.loadFromFile("../engineassets/ui/style.css")) {
                    std::cout << "Failed to load style.css\r\n";
                }
            }
        };

        class UiInclude : UiElement {
        public:
            UiInclude(const char *tag, const char* name) : UiElement(tag, name) {};
            static UiElement* create(const char* tag, const char* name) {
                return new UiInclude(name, tag);
            }
        };

        std::map<std::string, UiElementFactoryFn> pluginFactoryList;
        UiElement* tryCreate(const char* tag, const char* name) {
            std::map<std::string, UiElementFactoryFn>::iterator it;
            it = pluginFactoryList.find(tag);
            if (it != pluginFactoryList.end()) {
                return it->second(tag, name);
            }
            else {
                std::cout << "Unable to create: " << tag << "\r\n";
                return nullptr;
            }
        }
    }
}

const char* node_types[] =
{
    "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration"
};

using namespace Grindstone::PluginXmlUi;

struct simple_walker : pugi::xml_tree_walker
{
    virtual bool for_each(pugi::xml_node& node)
    {
        const char* type = node.name();
        UiElement *element = tryCreate(node.name(), node.attribute("name").value());

        return true; // continue traversal
    }
};

void traverseElement(UiElement* element, unsigned int depth) {
    for (unsigned int i = 0; i < depth; ++i) {
        std::cout << " ";
    }

    for each (UiElement * child in element->children_) {
        traverseElement(child, depth);
    }
}

int main() {
    pluginFactoryList["Root"]       = UiElement::create;
    pluginFactoryList["Include"]    = UiInclude::create;
    pluginFactoryList["Element"]    = UiElement::create;
    pluginFactoryList["Meta"]       = UiElement::create;
    pluginFactoryList["Stylesheet"] = UiStylesheet::create;
    pluginFactoryList["Panel"]      = UiVisualElement::create;

    pugi::xml_document doc;

    if (!doc.load_file("../engineassets/ui/editor.xml")) return -1;
    
    
    simple_walker walker;
    doc.traverse(walker);

    UiElement* root = nullptr;
    traverseElement(root, 0);
}