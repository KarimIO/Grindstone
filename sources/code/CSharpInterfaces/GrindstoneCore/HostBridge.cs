using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Runtime.Loader;
using System.IO;

namespace Grindstone {
	public static class HostBridge {
		private static HotReloadContext? assemblyContext = null;

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
		public static void CreateAppDomain() {
			assemblyContext = new HotReloadContext("D:\\Work\\InOrdinate\\Grindstone\\Sandbox\\bin");
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
		public static IntPtr CreateObject(IntPtr assemblyPathPtr, IntPtr classNamePtr) {
			try {
				string assemblyPath = Marshal.PtrToStringAnsi(assemblyPathPtr)!;
				string className = Marshal.PtrToStringAnsi(classNamePtr)!;
				Grindstone.Logger.Print($"Trying to load class '{className}' from assembly '{assemblyPath}'.");

				if (assemblyContext == null) {
					Grindstone.Logger.PrintError("Trying to load an assembly without calling CreateAppDomain first! We'll do it for you but this should be handled by C++.");
					assemblyContext = new HotReloadContext("D:\\Work\\InOrdinate\\Grindstone\\Sandbox\\bin");
					assemblyContext.Resolving += Resolving;
				}

				Assembly assembly = assemblyContext.LoadFromAssemblyPath(assemblyPath);
				if (assembly == null) {
					Grindstone.Logger.PrintError($"Failed loading assembly from path '{assemblyPath}'.");
					return -1;
				}

				foreach(Type testType in assembly.GetTypes()) {
					Grindstone.Logger.Print($" - {testType}");
				}

				Type? type = assembly.GetType(className);
				if (type == null) {
					Grindstone.Logger.PrintError($"Failed to find class '{className}' in assembly '{assemblyPath}'");
					return -1;
				}

				object instance = Activator.CreateInstance(type)!;
				GCHandle handle = GCHandle.Alloc(instance);
				IntPtr instancePtr = GCHandle.ToIntPtr(handle);
				return instancePtr;
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to load assembly: {ex.Message}");
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
