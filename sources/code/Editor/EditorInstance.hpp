#pragma once

namespace Grindstone {
	class EditorInstance {
	public:
		virtual ~EditorInstance() {};
	public:
		virtual void run() = 0;
		virtual void cleanup() = 0;
		virtual unsigned int getID() const = 0;
	};
}