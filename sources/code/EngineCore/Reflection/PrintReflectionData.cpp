#include "PrintReflectionData.hpp"

namespace Grindstone::Reflection {
	std::string ParseStoredName(std::string displayName, std::string memberName) {
		if (displayName != "") return displayName;

		// Remove suffix _ if necessary
		if (memberName[memberName.size() - 1] == '_')
			memberName.erase(memberName.end() - 1);

		auto p = memberName.find_last_of('.');
		if (p != std::string::npos)
			memberName = memberName.substr(p + 1);

		return memberName;
	}

	std::string ParseDisplayName(std::string displayName, std::string memberName) {
		if (displayName != "") return displayName;

		auto p = memberName.find_last_of('.');
		if (p != std::string::npos)
			memberName = memberName.substr(p + 1);

		bool first = true;
		bool symbols = false;
		for (int i = 0; i < memberName.size(); ++i) {
			if (!isalnum(memberName[i])) {
				symbols = true;
				memberName[i] = ' ';
			}
			else {
				if (isalpha(memberName[i])) {
					if (first) {
						memberName[i] = toupper(memberName[i]);
						first = false;
					}
					else if (symbols) {
						memberName[i] = toupper(memberName[i]);
					}
					else if (isupper(memberName[i])) {
						memberName.insert(i, " ");
						++i;
					}
				}
				symbols = false;
			}
		}

		for (size_t i = memberName.size() - 1u; i > 0u; i--) {
			if (memberName[i] == ' ' && memberName[i] == memberName[i - 1]) {
				memberName.erase(memberName.begin() + i);
			}
		}

		if (memberName[0] == ' ') memberName.erase(memberName.begin());
		if (memberName[memberName.size() - 1] == ' ') memberName.erase(memberName.end() - 1);

		return memberName;
	}

	std::string StringifyMetadata(Metadata m) {
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
