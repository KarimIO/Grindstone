using System.Runtime.InteropServices;

namespace Grindstone.Ai {
	public class NavmeshAgentComponent {
		System.IntPtr componentPtr;

		public NavmeshAgentComponent(System.IntPtr componentPtr) {
			this.componentPtr = componentPtr;
		}

		public NavmeshAgentComponent(Grindstone.Entity entity) {
			this.componentPtr = EntityGetNavmeshAgentComponent(entity.scene.sceneIntPtr, entity.entityHandle);
		}

		public float MaxMoveSpeed {
			get => NavmeshAgentComponentGetMaxMoveSpeed(componentPtr);
			set => NavmeshAgentComponentSetMaxMoveSpeed(componentPtr, value);
		}

		public void SetMoveTo(Grindstone.Math.Float3 position) {
			NavmeshAgentComponentMoveTo(componentPtr, position);
		}

		#region DllImports
		[DllImport("PluginNavigationMesh")]
		static extern System.IntPtr EntityGetNavmeshAgentComponent(System.IntPtr scene, uint entity);

		[DllImport("PluginNavigationMesh")]
		static extern float NavmeshAgentComponentGetMaxMoveSpeed(System.IntPtr comp);

		[DllImport("PluginNavigationMesh")]
		static extern void NavmeshAgentComponentSetMaxMoveSpeed(System.IntPtr comp, float moveSpeed);

		[DllImport("PluginNavigationMesh")]
		static extern void NavmeshAgentComponentMoveTo(uint entity, System.IntPtr comp, Grindstone.Math.Float3 position);
		#endregion
	}
}
