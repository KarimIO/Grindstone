using System;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Runtime.Loader;
using System.IO;

namespace Grindstone {
	public static class HostBridge {
		private static HotReloadContext? assemblyContext = null;
		private readonly static Dictionary<int, Assembly?> loadedAssemblies = new();

		private static Assembly? Resolving(AssemblyLoadContext context, AssemblyName name) {
			if (name.Name == "GrindstoneCSharpCore") {
				foreach (var asm in AppDomain.CurrentDomain.GetAssemblies()) {
					if (asm.GetName().Name == "GrindstoneCSharpCore") {
						return asm;
					}
				}
			}

			return null;
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static void CreateAppDomain(IntPtr assemblyDirectoryPtr) {
			string assemblyPath = Marshal.PtrToStringAnsi(assemblyDirectoryPtr)!;
			assemblyContext = new HotReloadContext(assemblyPath);
			assemblyContext.Resolving += Resolving;
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static void UnloadAppDomain() {
			assemblyContext?.Unload();
			assemblyContext = null;

			GC.Collect();
			GC.WaitForPendingFinalizers();
			GC.Collect();
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static int LoadAssembly(IntPtr assemblyPathPtr) {
			string assemblyPath = Marshal.PtrToStringAnsi(assemblyPathPtr)!;
			try {
				Grindstone.Logger.Print($"Trying to load assembly '{assemblyPath}'.");

				if (assemblyContext == null) {
					Grindstone.Logger.PrintError("Trying to load an assembly without calling CreateAppDomain first! We'll do it for you but this should be handled by C++.");
					return -1;
				}

				Assembly assembly = assemblyContext.LoadFromAssemblyPath(assemblyPath);
				if (assembly == null) {
					Grindstone.Logger.PrintError($"Failed loading assembly from path '{assemblyPath}'.");
					return -1;
				}

				int nameHash = assemblyPath.GetHashCode();
				loadedAssemblies.Add(nameHash, assembly);

				return nameHash;
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to load assembly {assemblyPath}: {ex.Message}");
				return -1;
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static IntPtr GetTypeCount(int assemblyHashName) {
			if (!loadedAssemblies.TryGetValue(assemblyHashName, out Assembly? assembly) || assembly == null) {
				Grindstone.Logger.PrintError($"Invalid assembly hash: {assemblyHashName}.");
				return -1;
			}

			return assembly.GetTypes().Length;
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static IntPtr CreateObject(int assemblyHashName, IntPtr classNamePtr) {
			string className = Marshal.PtrToStringAnsi(classNamePtr)!;
			try {
				if (!loadedAssemblies.TryGetValue(assemblyHashName, out Assembly? assembly) || assembly == null) {
					Grindstone.Logger.PrintError($"Invalid assembly hash: {assemblyHashName}.");
					return -1;
				}

				Type? type = assembly.GetType(className);
				if (type == null) {
					Grindstone.Logger.PrintError($"Failed to find class '{className}' in assembly '{assembly.GetName()}'");
					return -1;
				}

				object instance = Activator.CreateInstance(type)!;
				GCHandle handle = GCHandle.Alloc(instance);
				IntPtr instancePtr = GCHandle.ToIntPtr(handle);
				return instancePtr;
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to get type {className} from assembly: {ex.Message}");
				return -1;
			}
		}

		[UnmanagedCallersOnly(EntryPoint = "CallOnAttach")]
		public static void CallOnAttach(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				return;
			}

			var method = unboxedObject.GetType().GetMethod("OnAttach");
			method?.Invoke(unboxedObject, null);
		}

		[UnmanagedCallersOnly(EntryPoint = "CallOnStart")]
		public static void CallOnStart(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				return;
			}

			var method = unboxedObject.GetType().GetMethod("OnStart");
			method?.Invoke(unboxedObject, null);
		}

		[UnmanagedCallersOnly(EntryPoint = "CallOnUpdate")]
		public static void CallOnUpdate(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				return;
			}

			var method = unboxedObject.GetType().GetMethod("OnUpdate");
			method?.Invoke(unboxedObject, null);
		}

		[UnmanagedCallersOnly(EntryPoint = "CallOnEditorUpdate")]
		public static void CallOnEditorUpdate(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				return;
			}

			var method = unboxedObject.GetType().GetMethod("OnEditorUpdate");
			method?.Invoke(unboxedObject, null);
		}

		[UnmanagedCallersOnly(EntryPoint = "CallOnDestroy")]
		public static void CallOnDestroy(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				return;
			}

			var method = unboxedObject.GetType().GetMethod("OnDestroy");
			method?.Invoke(unboxedObject, null);
		}

		[UnmanagedCallersOnly(EntryPoint = "DestroyObject")]
		public static void DestroyObject(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			handleBack.Free();
		}

		[UnmanagedCallersOnly(EntryPoint = "ListFields")]
		public static IntPtr ListFields(IntPtr assemblyPathPtr, IntPtr classNamePtr) {
			string assemblyPath = Marshal.PtrToStringAnsi(assemblyPathPtr)!;
			string className = Marshal.PtrToStringAnsi(classNamePtr)!;

			Assembly assembly = Assembly.LoadFrom(assemblyPath);
			Type? type = assembly.GetType(className);

			if (type == null) {
				return -1;
			}

			var fields = type.GetFields(BindingFlags.Public | BindingFlags.Instance);

			string result = "";
			foreach (FieldInfo field in fields) {
				result += field.Name + ',';
			}

			if (result.EndsWith(',')) {
				result = result.Substring(0, result.Length - 1);
			}

			return Marshal.StringToHGlobalAnsi(result);
		}
	}
}
