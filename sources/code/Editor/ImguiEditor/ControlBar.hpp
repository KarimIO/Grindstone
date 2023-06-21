#pragma once

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			class ControlBar {
			public:
				ControlBar();
				void Render();
			private:
				void OnPressPlay();
				void OnPressPause();
				void OnPressManipulateTranslate();
				void OnPressManipulateRotate();
				void OnPressManipulateScale();
				void OnPressManipulateToggleWorldLocal();
			};
		}
	}
}
