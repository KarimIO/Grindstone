using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Grindstone {
	public unsafe struct NativeComponentOps {
		public delegate* unmanaged[Cdecl]<IntPtr, uint, IntPtr> Get;
		public delegate* unmanaged[Cdecl]<IntPtr, uint, IntPtr> Create;
		public delegate* unmanaged[Cdecl]<IntPtr, uint, void> Destroy;
		public delegate* unmanaged[Cdecl]<IntPtr, uint, bool> Has;
	}

	public class ComponentRegistrar {
		#region Public Methods	
		private static readonly Dictionary<Type, NativeComponentOps> typeToComponentOps = [];

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static unsafe void RegisterComponentOps(
			uint typeId,
			IntPtr getFn,
			IntPtr createFn,
			IntPtr destroyFn,
			IntPtr hasFn
		) {
			typeToComponentOps[Type.EmptyTypes[0]] = new NativeComponentOps {
				Get = (delegate* unmanaged[Cdecl]<IntPtr, uint, IntPtr>)getFn,
				Create = (delegate* unmanaged[Cdecl]<IntPtr, uint, IntPtr>)createFn,
				Destroy = (delegate* unmanaged[Cdecl]<IntPtr, uint, void>)destroyFn,
				Has = (delegate* unmanaged[Cdecl]<IntPtr, uint, bool>)hasFn,
			};
		}

		public static void ScanAssembly(Assembly assembly) {
			foreach (Type type in assembly.GetTypes()) {
				var attr = type.GetCustomAttribute<NativeComponentAttribute>();
				if (attr != null) {
					typeToComponentOps[type] = attr.ComponentName;
				}
			}
		}

		public static uint? GetTypeId(Type type) {
			return typeIds.TryGetValue(type, out uint id)
				? id
				: null;
		}

		public static uint GetTypeIdOrThrow(Type type) {
			if (!typeIds.TryGetValue(type, out uint id)) {
				throw new InvalidOperationException(
					$"Type {type.FullName} is not a registered native component. " +
					$"Did you forget [NativeComponent]?");
			}

			return id;
		}

		internal static T? CreateComponent<T>(IntPtr worldContext, uint entityHandle) {
			ref var ops = ref GetOps(typeof(T));
			return ops.Create(worldContext, entityHandle);
		}

		internal static bool HasComponent<T>(IntPtr worldContext, uint entityHandle) {
			ref var ops = ref GetOps(typeof(T));
			throw new NotImplementedException();
		}

		internal static T? GetComponent<T>(IntPtr worldContext, uint entityHandle) {
			ref var ops = ref GetOps(typeof(T));
			throw new NotImplementedException();
		}

		internal static void DeleteComponent<T>(IntPtr worldContext, uint entityHandle) {
			ref var ops = ref GetOps(typeof(T));
			throw new NotImplementedException();
		}

		private static ref NativeComponentOps GetOps(Type type) {
			if (!typeToComponentOps.TryGetValue(type, out _))
				throw new InvalidOperationException(
					$"No native ops registered for component type {type}. " +
					$"Was it registered with ComponentRegistry::Register<T>()?");

			return ref System.Runtime.InteropServices.CollectionsMarshal.GetValueRefOrNullRef(_ops, typeId);
		}
		#endregion
	}
}
