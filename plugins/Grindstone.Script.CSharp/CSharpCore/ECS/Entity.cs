using System.Runtime.InteropServices;

namespace Grindstone {
	[System.Serializable]
	public struct Entity {
		#region Public Fields
		public uint entityHandle;
		public WorldContext worldContext;
		#endregion

		#region Static Fields
		public static readonly uint InvalidHandle = uint.MaxValue;
		#endregion

		#region Public Methods
		public Entity(WorldContext worldContext, uint handle) {
			entityHandle = handle;
			this.worldContext = worldContext;
		}

		public void Destroy() {
			EntityMarkForDeletion(worldContext.GetHandle(), entityHandle);
		}

		public T? CreateComponent<T>() {
			return ComponentRegistrar.CreateComponent<T>(worldContext.GetHandle(), entityHandle);
		}

		public readonly T? GetComponent<T>() {
			return ComponentRegistrar.GetComponent<T>(worldContext.GetHandle(), entityHandle);
		}

		public void DeleteComponent<T>() {
			ComponentRegistrar.DeleteComponent<T>(worldContext.GetHandle(), entityHandle);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		static extern void EntityMarkForDeletion(IntPtr worldContextHandle, uint entity);
		#endregion
	}
}
