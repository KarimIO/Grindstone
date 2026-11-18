using System.Diagnostics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Grindstone {
	public unsafe struct NativeComponentOps {
		public delegate* unmanaged[Cdecl]<IntPtr, uint, IntPtr> Create;
		public delegate* unmanaged[Cdecl]<IntPtr, uint, ref IntPtr, bool> TryGet;
		public delegate* unmanaged[Cdecl]<IntPtr, uint, bool> Has;
		public delegate* unmanaged[Cdecl]<IntPtr, uint, void> Destroy;
	}

	public class ComponentRegistrar {
		#region Public Methods	
		private static readonly Dictionary<Type, NativeComponentOps> typeToNativeComponentCallbacks = [];

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static unsafe void RegisterComponentOps(
			uint typeId,
			IntPtr createFn,
			IntPtr tryGetFn,
			IntPtr hasFn,
			IntPtr destroyFn
		) {
			typeToNativeComponentCallbacks[Type.EmptyTypes[0]] = new NativeComponentOps {
				Create = (delegate* unmanaged[Cdecl]<IntPtr, uint, IntPtr>)createFn,
				TryGet = (delegate* unmanaged[Cdecl]<IntPtr, uint, ref IntPtr, bool>)tryGetFn,
				Destroy = (delegate* unmanaged[Cdecl]<IntPtr, uint, void>)destroyFn,
				Has = (delegate* unmanaged[Cdecl]<IntPtr, uint, bool>)hasFn,
			};
		}

		/*
		public static void ScanAssembly(Assembly assembly) {
			foreach (Type type in assembly.GetTypes()) {
				if (type.IsAssignableTo(typeof(NativeComponent))) {
					Grindstone.Hash hash = Grindstone.Hash.MurmurOAAT64(type.ToString());
					typeToNativeComponentCallbacks[type] = GetEcsOpsForComponent(hash.GetValue());
				}
			}
		}
		*/

		private static NativeComponentOps GetOps(Type type) {
			if (!typeToNativeComponentCallbacks.TryGetValue(type, out NativeComponentOps ops))
				throw new InvalidOperationException(
					$"No native ops registered for component type {type}. " +
					$"Was it registered with ComponentRegistry::Register<T>()?");

			return ops;
		}

		internal static nint GetComponentPtr<T>(nint worldContextHandle, uint entityHandle) where T : unmanaged {
			Debug.Assert(worldContextHandle != IntPtr.Zero && entityHandle != 0);
			if (!typeToNativeComponentCallbacks.TryGetValue(typeof(T), out NativeComponentOps ops)) {
				PropertyInfo? property = typeof(T).GetProperty("NativeComponentName");
				if (property == null) {
					return 0;
				}

				MethodInfo? method = property.GetGetMethod();
				if (method == null) {
					return 0;
				}

				string nativeComponentName = (string)method.Invoke(null, null)!;
				ops = GetComponentAccessBindings(nativeComponentName);
				typeToNativeComponentCallbacks[typeof(T)] = ops;
			}

			bool hasComponent = false;
			IntPtr intPtr = 0;
			unsafe {
				hasComponent = ops.TryGet(worldContextHandle, entityHandle, ref intPtr);
			}
			if (hasComponent) {
				return intPtr;
			}

			return 0;
		}

		[DllImport("EngineCore")]
		private static extern NativeComponentOps GetComponentAccessBindings(string componentName);
		#endregion
	}
}
