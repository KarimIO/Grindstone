#include "PrintReflectionData.hpp"

namespace Grindstone {
	namespace Reflection {
		std::string parseStoredName(std::string v, std::string n) {
			if (v != "") return v;
			// Remove suffix _ if necessary
			if (n[n.size() - 1] == '_')
				n.erase(n.end() - 1);

			auto p = n.find_last_of('.');
			if (p != std::string::npos)
				n = n.substr(p + 1);

			return n;
		}

		std::string parseDisplayName(std::string v, std::string n) {
			if (v != "") return v;

			auto p = n.find_last_of('.');
			if (p != std::string::npos)
				n = n.substr(p + 1);

			bool first = true;
			bool symbols = false;
			for (int i = 0; i < n.size(); ++i) {
				if (!isalnum(n[i])) {
					symbols = true;
					n[i] = ' ';
				}
				else {	
					if (isalpha(n[i])) {
						if (first) {
							n[i] = toupper(n[i]);
							first = false;
						}
						else if (symbols) {
							n[i] = toupper(n[i]);
						}
						else if (isupper(n[i])) {
							n.insert(i, " ");
							++i;
						}
					}
					symbols = false;
				}
			}

			for (size_t i = n.size() - 1u; i > 0u; i--) {
				if (n[i] == ' '&&n[i] == n[i - 1]) {
					n.erase(n.begin() + i);
				}
			}

			if (n[0] == ' ') n.erase(n.begin());
			if (n[n.size() - 1] == ' ') n.erase(n.end() - 1);

			return n;
		}

		std::string stringifyMetadata(Metadata m) {
			short mval = (short)m;
			std::string o;
			if (mval & (short)Metadata::ViewInEditor)
				o += "ViewInEditor, ";
			if (mval & (short)Metadata::SetInEditor)
				o += "SetInEditor, ";
			if (mval & (short)Metadata::ViewInScript)
				o += "ViewInScript, ";
			if (mval & (short)Metadata::SetInScript)
				o += "SetInScript, ";
			if (mval & (short)Metadata::SaveState)
				o += "SaveState, ";

			return o.substr(0, o.size() - 2);
		}
	}
}
