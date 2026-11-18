using System.Runtime.InteropServices;

namespace Grindstone.Ai {
	[System.Serializable]
	public struct NavmeshAgentComponent {
		public static string NativeComponentName => "NavigationAgent";

		public float MaxMoveSpeed {
			get => NavmeshAgentComponentGetMaxMoveSpeed(ref this);
			set => NavmeshAgentComponentSetMaxMoveSpeed(ref this, value);
		}

		public void SetMoveTo(Grindstone.Math.Float3 position) {
			NavmeshAgentComponentMoveTo(ref this, position);
		}

		#region DllImports

		[DllImport("PluginNavigationMesh")]
		static extern float NavmeshAgentComponentGetMaxMoveSpeed(ref NavmeshAgentComponent comp);

		[DllImport("PluginNavigationMesh")]
		static extern void NavmeshAgentComponentSetMaxMoveSpeed(ref NavmeshAgentComponent comp, float moveSpeed);

		[DllImport("PluginNavigationMesh")]
		static extern void NavmeshAgentComponentMoveTo(ref NavmeshAgentComponent comp, Grindstone.Math.Float3 position);
		#endregion
	}
}
