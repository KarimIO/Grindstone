#pragma once

namespace Grindstone {
	namespace ECS {
		class IComponentFactory {
		public:
			void* createComponent();
		};
	}
}
