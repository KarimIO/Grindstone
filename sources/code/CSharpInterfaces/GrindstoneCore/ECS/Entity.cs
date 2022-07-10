using System.Runtime.InteropServices;

namespace Grindstone {
	[System.Serializable]
	public struct Entity {
		#region Public Fields
		public uint entityHandle;
		public Scene scene;
		#endregion

		#region Static Fields
		public static uint InvalidHandle = uint.MaxValue;
		#endregion

		#region Public Methods
		public Entity(uint handle, Scene scene) {
			entityHandle = handle;
			this.scene = scene;
		}

		public T CreateComponent<T>() {
			return default;
		}

		public T GetComponent<T>() {
			uint componentType = 4;
			System.IntPtr ptr = EntityGetComponentByType(scene.sceneIntPtr, entityHandle, componentType);
			return Marshal.PtrToStructure<T>(ptr);
		}

		public TransformComponent GetTransformComponent() {
			System.IntPtr ptr = EntityGetTransformComponent(scene.sceneIntPtr, entityHandle);
			return new TransformComponent(ptr);
		}

		public TagComponent GetTagComponent() {
			System.IntPtr ptr = EntityGetTagComponent(scene.sceneIntPtr, entityHandle);
			return new TagComponent(ptr);
		}

		public void DeleteComponent<T>() {}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		static extern System.IntPtr EntityGetTransformComponent(System.IntPtr scene, uint entity);

		[DllImport("EngineCore")]
		static extern System.IntPtr EntityGetTagComponent(System.IntPtr scene, uint entity);

		[DllImport("EngineCore")]
		static extern System.IntPtr EntityGetComponentByType(System.IntPtr scene, uint entity, uint componentType);
		#endregion
	}
}
