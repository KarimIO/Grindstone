namespace Grindstone {
	public struct WorldContext {
		private IntPtr handle;

		public bool IsValid => handle != IntPtr.Zero;

		public WorldContext(IntPtr handle) {
			this.handle = handle;
		}

		public Entity GetEntityByHandle(uint handle) {
			return new Entity(this, handle);
		}

		public readonly IntPtr Handle { get => handle; }

		public static bool operator ==(WorldContext a, WorldContext b) {
			return Equals(a, b);
		}
		public static bool operator !=(WorldContext a, WorldContext b) {
			return NotEquals(a, b);
		}
		public static implicit operator bool(WorldContext compRef) {
			return compRef.IsValid;
		}

		private static bool Equals(WorldContext a, WorldContext b) {
			return (a.handle == b.handle);
		}
		private static bool NotEquals(WorldContext a, WorldContext b) {
			return !Equals(a, b);
		}

		public readonly override bool Equals(object? obj) {
			if ((obj == null) || !obj.GetType().Equals(typeof(WorldContext))) {
				return false;
			}

			WorldContext otherCxt = (WorldContext)obj;
			return otherCxt.handle == handle;
		}

		public readonly override int GetHashCode() {
			return System.HashCode.Combine(handle);
		}
	}
}
