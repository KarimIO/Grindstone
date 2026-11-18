using System.Runtime.InteropServices;

namespace Grindstone {
	[System.Serializable]
	public struct Entity {
		#region Private Fields
		private uint entityHandle;
		private WorldContext worldContext;
		#endregion

		#region Static Fields
		public static readonly uint InvalidHandle = uint.MaxValue;
		#endregion

		#region Properties
		public bool IsValid => entityHandle != InvalidHandle && WorldContext.IsValid;
		public uint Handle => entityHandle;
		public WorldContext WorldContext => worldContext;
		#endregion

		#region Public Methods
		public Entity(WorldContext worldContext, uint handle) {
			entityHandle = handle;
			this.worldContext = worldContext;
		}

		public NativeComponentRef<T> GetNativeComponentRef<T>() where T : unmanaged {
			return new NativeComponentRef<T>(this, 0);
		}

		public static bool operator ==(Entity a, Entity b) {
			return Equals(a, b);
		}
		public static bool operator !=(Entity a, Entity b) {
			return NotEquals(a, b);
		}
		public static implicit operator bool(Entity entity) {
			return entity.IsValid;
		}

		private static bool Equals(Entity a, Entity b) {
			return (a.entityHandle == b.entityHandle) && (a.worldContext == b.worldContext);
		}
		private static bool NotEquals(Entity foo, Entity fooB) {
			return !Equals(foo, fooB);
		}

		public readonly override bool Equals(object? obj) {
			if (obj == null) return false;
			if (obj.GetType() != typeof(Entity)) return false;
			return Equals(this, obj);
		}

		public readonly override int GetHashCode() {
			return HashCode.Combine(entityHandle, worldContext.GetHashCode());
		}

		public void Destroy() {
			EntityMarkForDeletion(worldContext.Handle, entityHandle);
		}

		public T? CreateComponent<T>() {
			return default; // return ComponentRegistrar.CreateComponent<T>(worldContext.GetHandle(), entityHandle);
		}

		public readonly bool TryGetComponent<T>(out T? returnValue) {
			returnValue = default;
			return false; // return ComponentRegistrar.TryGetComponent<T>(worldContext.GetHandle(), entityHandle, out returnValue);
		}

		public readonly bool HasComponent<T>() {
			return false; // return ComponentRegistrar.HasComponent<T>(worldContext.GetHandle(), entityHandle);
		}

		public readonly T? GetComponent<T>() {
			return default; // return ComponentRegistrar.GetComponent<T>(worldContext.GetHandle(), entityHandle);
		}

		public void DeleteComponent<T>() {
			// ComponentRegistrar.DeleteComponent<T>(worldContext.GetHandle(), entityHandle);
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		static extern void EntityMarkForDeletion(IntPtr worldContextHandle, uint entity);
		#endregion
	}
}
