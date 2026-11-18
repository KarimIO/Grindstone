using System.Reflection.Metadata;
using System.Runtime.CompilerServices;

namespace Grindstone {
	public struct NativeComponentRef<T> where T : unmanaged {
		private System.IntPtr handlePtr;
		private Entity entity;

		public Entity Entity => entity;
		public bool IsValid => Entity.IsValid && ResolvePointer() != IntPtr.Zero;

		public NativeComponentRef(Entity newEntity, IntPtr initialPtr = default) {
			entity = newEntity;
			handlePtr = initialPtr;
		}

		public static bool operator ==(NativeComponentRef<T> a, NativeComponentRef<T> b) {
			return Equals(a, b);
		}
		public static bool operator !=(NativeComponentRef<T> a, NativeComponentRef<T> b) {
			return NotEquals(a, b);
		}
		public static implicit operator bool(NativeComponentRef<T> compRef) {
			return compRef.IsValid;
		}

		private static bool Equals(NativeComponentRef<T> a, NativeComponentRef<T> b) {
			return (a.entity == b.entity) || (a.handlePtr == b.handlePtr);
		}
		private static bool NotEquals(NativeComponentRef<T> foo, NativeComponentRef<T> fooB) {
			return !Equals(foo, fooB);
		}

		public readonly override bool Equals(object? obj) {
			if (obj == null) return false;
			if (obj.GetType() != typeof(NativeComponentRef<T>)) return false;
			return Equals(this, obj);
		}

		public readonly override int GetHashCode() {
			return HashCode.Combine(entity.GetHashCode(), handlePtr.GetHashCode());
		}

		public ref T Value {
			get {
				IntPtr ptr = ResolvePointer();
				if (ptr == IntPtr.Zero) {
					throw new NullReferenceException($"Component '{typeof(T).Name}' could not be resolved for entity '{entity}'.");
				}

				unsafe {
					return ref Unsafe.AsRef<T>((void*)ptr);
				}
			}
		}

		public bool TryResolve(out IntPtr ptr) {
			ptr = ResolvePointer();
			return ptr != IntPtr.Zero;
		}

		private IntPtr ResolvePointer() {
			handlePtr = ComponentRegistrar.GetComponentPtr<T>(entity.WorldContext.Handle, entity.Handle);
			return handlePtr;
		}
	};
}
