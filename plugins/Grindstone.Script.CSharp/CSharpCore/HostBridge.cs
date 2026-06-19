using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Numerics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Text;

namespace Grindstone {
	public static class HostBridge {
		private static HotReloadContext? assemblyContext = null;
		private readonly static Dictionary<int, Assembly?> loadedAssemblies = new();

		private static Assembly? Resolving(AssemblyLoadContext context, AssemblyName name) {
			if (name.Name == "Grindstone.Script.CSharpCore") {
				foreach (var asm in AppDomain.CurrentDomain.GetAssemblies()) {
					if (asm.GetName().Name == "Grindstone.Script.CSharpCore") {
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
			loadedAssemblies.Clear();
			WeakReference weakContext = new(assemblyContext);
			assemblyContext?.Unload();
			assemblyContext = null;

			GC.Collect();
			GC.WaitForPendingFinalizers();
			GC.Collect();


			if (!weakContext.IsAlive) {
				Grindstone.Logger.Print("Unload successful");
			}
			else {
				Grindstone.Logger.PrintError("Unload failed: something is still referencing the context");
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static int LoadAssembly(IntPtr assemblyPathPtr) {
			string assemblyPath = Marshal.PtrToStringAnsi(assemblyPathPtr)!;
			try {
				Grindstone.Logger.Print($"Trying to load assembly '{assemblyPath}'.");

				if (assemblyContext == null) {
					Grindstone.Logger.PrintError("Trying to load an assembly without calling CreateAppDomain first! We'll do it for you but this should be handled by C++.");
					return 0;
				}

				Assembly assembly = assemblyContext.LoadFromAssemblyPath(assemblyPath);
				if (assembly == null) {
					Grindstone.Logger.PrintError($"Failed loading assembly from path '{assemblyPath}'.");
					return 0;
				}

				int nameHash = assemblyPath.GetHashCode();
				loadedAssemblies.Add(nameHash, assembly);

				return nameHash;
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to load assembly {assemblyPath}: {ex.Message}");
				return 0;
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static IntPtr GetTypeCount(int assemblyHashName) {
			if (!loadedAssemblies.TryGetValue(assemblyHashName, out Assembly? assembly) || assembly == null) {
				Grindstone.Logger.PrintError($"Invalid assembly hash: {assemblyHashName}.");
				return 0;
			}

			return assembly.GetTypes().Length;
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static IntPtr CreateComponent(int assemblyHashName, IntPtr classNamePtr, uint entity) {
			string className = Marshal.PtrToStringAnsi(classNamePtr)!;
			try {
				if (!loadedAssemblies.TryGetValue(assemblyHashName, out Assembly? assembly) || assembly == null) {
					Grindstone.Logger.PrintError($"Invalid assembly hash: {assemblyHashName}.");
					return 0;
				}

				Type? type = assembly.GetType(className);
				if (type == null) {
					Grindstone.Logger.PrintError($"Failed to find class '{className}' in assembly '{assembly.GetName()}'");
					return 0;
				}

				object instance = Activator.CreateInstance(type)!;
				GCHandle handle = GCHandle.Alloc(instance);
				IntPtr instancePtr = GCHandle.ToIntPtr(handle);

				FieldInfo? field = type.GetField("entity", BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
				if (field == null) {
					Grindstone.Logger.PrintError($"Trying to create a non-smart component of type '{classNamePtr}' as a smart component. Cannot set entity ({entity}).");
				}
				else {
					field.SetValue(instance, new Entity(entity, new Scene(0)));
				}

				return instancePtr;
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to get type {className} from assembly: {ex.Message}");
				return 0;
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static IntPtr CreateObject(int assemblyHashName, IntPtr classNamePtr) {
			string className = Marshal.PtrToStringAnsi(classNamePtr)!;
			try {
				if (!loadedAssemblies.TryGetValue(assemblyHashName, out Assembly? assembly) || assembly == null) {
					Grindstone.Logger.PrintError($"Invalid assembly hash: {assemblyHashName}.");
					return 0;
				}

				Type? type = assembly.GetType(className);
				if (type == null) {
					Grindstone.Logger.PrintError($"Failed to find class '{className}' in assembly '{assembly.GetName()}'");
					return 0;
				}

				object instance = Activator.CreateInstance(type)!;
				GCHandle handle = GCHandle.Alloc(instance);
				IntPtr instancePtr = GCHandle.ToIntPtr(handle);
				return instancePtr;
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to get type {className} from assembly: {ex.Message}");
				return 0;
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static void CallOnAttach(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				Grindstone.Logger.PrintError("CallOnAttach: Couldn't findobject.");
				return;
			}

			Type type = unboxedObject.GetType();
			MethodInfo? method = type.GetMethod("OnAttach");
			if (method == null) {
				return;
			}

			try {
				method?.Invoke(unboxedObject, null);
			}
			catch (TargetInvocationException ex) {
				Exception inner = ex.InnerException ?? ex;
				Grindstone.Logger.PrintError(
					$"Failed to call OnAttach on {type.FullName}:\n" +
					$"{inner.GetType().Name}: {inner.Message}\n" +
					$"{inner.StackTrace}"
				);
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to call OnAttach on {type.FullName}: {ex.Message}");
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static void CallOnStart(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				Grindstone.Logger.PrintError("CallOnStart: Couldn't findobject.");
				return;
			}

			Type type = unboxedObject.GetType();
			MethodInfo? method = type.GetMethod("OnStart");
			if (method == null) {
				return;
			}

			try {
				method?.Invoke(unboxedObject, null);
			}
			catch (TargetInvocationException ex) {
				Exception inner = ex.InnerException ?? ex;
				Grindstone.Logger.PrintError(
					$"Failed to call OnStart on {type.FullName}:\n" +
					$"{inner.GetType().Name}: {inner.Message}\n" +
					$"{inner.StackTrace}"
				);
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to call OnStart on {type.FullName}: {ex.Message}");
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static void CallOnUpdate(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				Grindstone.Logger.PrintError("CallOnUpdate: Couldn't findobject.");
				return;
			}

			Type type = unboxedObject.GetType();
			MethodInfo? method = type.GetMethod("OnUpdate");
			if (method == null) {
				return;
			}

			try {
				method?.Invoke(unboxedObject, null);
			}
			catch (TargetInvocationException ex) {
				Exception inner = ex.InnerException ?? ex;
				Grindstone.Logger.PrintError(
					$"Failed to call OnUpdate on {type.FullName}:\n" +
					$"{inner.GetType().Name}: {inner.Message}\n" +
					$"{inner.StackTrace}"
				);
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to call OnUpdate on {type.FullName}: {ex.StackTrace}");
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static void CallOnEditorUpdate(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				Grindstone.Logger.PrintError("CallOnEditorUpdate: Couldn't findobject.");
				return;
			}

			Type type = unboxedObject.GetType();
			MethodInfo? method = type.GetMethod("OnEditorUpdate");
			if (method == null) {
				return;
			}

			try {
				method?.Invoke(unboxedObject, null);
			}
			catch (TargetInvocationException ex) {
				Exception inner = ex.InnerException ?? ex;
				Grindstone.Logger.PrintError(
					$"Failed to call OnEditorUpdate on {type.FullName}:\n" +
					$"{inner.GetType().Name}: {inner.Message}\n" +
					$"{inner.StackTrace}"
				);
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to call OnEditorUpdate on {type.FullName}: {ex.Message}");
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static void CallOnDestroy(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			object? unboxedObject = handleBack.Target;

			if (unboxedObject == null) {
				Grindstone.Logger.PrintError("CallOnDestroy: Couldn't findobject.");
				return;
			}

			Type type = unboxedObject.GetType();
			MethodInfo? method = type.GetMethod("OnDestroy");
			if (method == null) {
				return;
			}

			try {
				method?.Invoke(unboxedObject, null);
			}
			catch (TargetInvocationException ex) {
				Exception inner = ex.InnerException ?? ex;
				Grindstone.Logger.PrintError(
					$"Failed to call OnDestroy on {type.FullName}:\n" +
					$"{inner.GetType().Name}: {inner.Message}\n" +
					$"{inner.StackTrace}"
				);
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"Failed to call OnDestroy on {type.FullName}: {ex.Message}");
			}
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static void DestroyObject(IntPtr instancePtr) {
			GCHandle handleBack = GCHandle.FromIntPtr(instancePtr);
			handleBack.Free();
			GC.Collect();
		}

		public enum InspectorFieldType {
			Unknown,
			Entity,
			Component,
			Float, Float2, Float3, Float4,
			Int, Bool, String,
			Quaternion
		}

		[System.Serializable, StructLayout(LayoutKind.Sequential)]
		public unsafe struct FieldMetaData {
			public fixed byte name[128];
			public fixed byte displayName[128];
			public InspectorFieldType fieldType;
			public uint valueArenaOffset;
			public uint valueArenaSize;
			internal IntPtr setter;
		}

		[System.Serializable, StructLayout(LayoutKind.Sequential)]
		public struct FieldCountAndBufferSizeResponse {
			public uint fieldCount;
			public uint bufferSize;
		};

		static unsafe void WriteValueToArena(
			FieldInfo field,
			object obj,
			InspectorFieldType fieldType,
			byte* dest,
			uint size
		) {
			switch (fieldType) {
				case InspectorFieldType.Float: {
					float v = (float)field.GetValue(obj)!;
					*(float*)dest = v;
					break;
				}
				case InspectorFieldType.Float2: {
					Grindstone.Math.Float2 v = (Grindstone.Math.Float2)field.GetValue(obj)!;
					*(Grindstone.Math.Float2*)dest = v;
					break;
				}
				case InspectorFieldType.Float3: {
					Grindstone.Math.Float3 v = (Grindstone.Math.Float3)field.GetValue(obj)!;
					*(Grindstone.Math.Float3*)dest = v;
					break;
				}
				case InspectorFieldType.Float4: {
					Grindstone.Math.Float4 v = (Grindstone.Math.Float4)field.GetValue(obj)!;
					*(Grindstone.Math.Float4*)dest = v;
					break;
				}
				case InspectorFieldType.Quaternion: {
					Grindstone.Math.Quaternion v = (Grindstone.Math.Quaternion)field.GetValue(obj)!;
					*(Grindstone.Math.Quaternion*)dest = v;
					break;
				}
				case InspectorFieldType.Int: {
					int v = (int)field.GetValue(obj)!;
					*(int*)dest = v;
					break;
				}
				case InspectorFieldType.Bool: {
					bool v = (bool)field.GetValue(obj)!;
					*dest = v ? (byte)1 : (byte)0;
					break;
				}
				case InspectorFieldType.String: {
					string s = (string?)field.GetValue(obj) ?? "";
					CopyStringToBuffer(s, dest, (int)size);
					break;
				}
				// Entity, Component: write an IntPtr-sized GCHandle if needed
				// or your own stable numeric ID
			}
		}

		private static InspectorFieldType GetInspectorFieldType(Type t) {
			if (t == typeof(float)) return InspectorFieldType.Float;
			if (t == typeof(Vector2)) return InspectorFieldType.Float2;
			if (t == typeof(Vector3)) return InspectorFieldType.Float3;
			if (t == typeof(Vector4)) return InspectorFieldType.Float4;
			if (t == typeof(int)) return InspectorFieldType.Int;
			if (t == typeof(bool)) return InspectorFieldType.Bool;
			if (t == typeof(string)) return InspectorFieldType.String;
			if (t == typeof(Quaternion)) return InspectorFieldType.Quaternion;
			return InspectorFieldType.Unknown;
		}

		private static uint GetValueSize(InspectorFieldType t) => t switch {
			InspectorFieldType.Float      => 4,
			InspectorFieldType.Float2     => 8,
			InspectorFieldType.Float3     => 12,
			InspectorFieldType.Float4     => 16,
			InspectorFieldType.Quaternion => 16,
			InspectorFieldType.Int        => 4,
			InspectorFieldType.Bool       => 1,
			_                             => 0
		};

		private static string ParseDisplayName(string displayName, string memberName) {
			if (displayName.Length > 0) {
				return displayName;
			}

			// Strip namespace/class prefix
			int dot = memberName.LastIndexOf('.');
			if (dot >= 0) {
				memberName = memberName[(dot + 1)..];
			}

			if (memberName.Length == 0) {
				return displayName;
			}

			StringBuilder sb = new System.Text.StringBuilder(memberName.Length + 8);

			bool first = true;
			bool prevWasSymbol = false;

			foreach (char c in memberName) {
				if (!char.IsLetterOrDigit(c)) {
					prevWasSymbol = true;
					// Collapse into a single space lazily — only emit if sb isn't empty
					if (sb.Length > 0 && sb[^1] != ' ')
						sb.Append(' ');
				}
				else {
					if (char.IsLetter(c)) {
						if (first) {
							sb.Append(char.ToUpperInvariant(c));
							first = false;
						}
						else if (prevWasSymbol) {
							sb.Append(char.ToUpperInvariant(c));
						}
						else if (char.IsUpper(c)) {
							if (sb.Length > 0 && sb[^1] != ' ') {
								sb.Append(' ');
							}

							sb.Append(c);
						}
						else {
							sb.Append(c);
						}
					}
					else {
						// digit
						if (first) {
							first = false;
						}

						sb.Append(c);
					}
					prevWasSymbol = false;
				}
			}

			// Trim trailing space
			if (sb.Length > 0 && sb[^1] == ' ') {
				sb.Length--;
			}

			return sb.ToString();
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static FieldCountAndBufferSizeResponse GetComponentFieldCountAndBufferSize(IntPtr objectPtr) {
			try {
				GCHandle handleBack = GCHandle.FromIntPtr(objectPtr);
				if (handleBack.Target == null) {
					Grindstone.Logger.PrintError($"GetComponentFieldCount: Failed to get object.");
					return new FieldCountAndBufferSizeResponse { fieldCount = 0, bufferSize = 0 };
				}

				object handleObj = handleBack.Target;

				Type? type = handleObj.GetType();
				if (type == null) {
					Grindstone.Logger.PrintError($"GetComponentFieldCount: Failed to get type from object.");
					return new FieldCountAndBufferSizeResponse { fieldCount = 0, bufferSize = 0 };
				}

				uint bufferSize = 0;
				FieldInfo[] fields = type.GetFields(BindingFlags.Public | BindingFlags.Instance);
				foreach (var field in fields) {
					InspectorFieldType fieldType = GetInspectorFieldType(field.FieldType);
					bufferSize += GetValueSize(fieldType);
				}

				return new FieldCountAndBufferSizeResponse { fieldCount = (uint)fields.Length, bufferSize = bufferSize };
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"GetComponentFieldCount Exception: {ex.Message}");
				return new FieldCountAndBufferSizeResponse { fieldCount = 0, bufferSize = 0 };
			}
		}

		private static unsafe void CopyStringToBuffer(string s, byte* buf, int bufLen) {
			int written = Encoding.UTF8.GetBytes(s, new Span<byte>(buf, bufLen - 1));
			buf[written] = 0; // null terminate
		}

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void FieldSetterDelegate(IntPtr componentPtr, IntPtr valuePtr);

		private static unsafe FieldSetterDelegate MakeSetter(FieldInfo field) {
			return (componentPtr, valuePtr) => {
				try {
					object obj = GCHandle.FromIntPtr(componentPtr).Target!;
					InspectorFieldType fieldType = GetInspectorFieldType(field.FieldType);

					object? boxed = fieldType switch {
						InspectorFieldType.Float => *(float*)valuePtr,
						InspectorFieldType.Float2 => *(Vector2*)valuePtr,
						InspectorFieldType.Float3 => *(Vector3*)valuePtr,
						InspectorFieldType.Float4 => *(Vector4*)valuePtr,
						InspectorFieldType.Quaternion => *(Quaternion*)valuePtr,
						InspectorFieldType.Int => *(int*)valuePtr,
						InspectorFieldType.Bool => *(byte*)valuePtr != 0,
						InspectorFieldType.String => Marshal.PtrToStringUTF8(valuePtr),
						_ => null
					};

					if (boxed != null) {
						field.SetValue(obj, boxed);
					}
				}
				catch (Exception ex) {
					Grindstone.Logger.PrintError($"Setter for {field.Name}: {ex.Message}");
				}
			};
		}

		[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
		public static unsafe void FillComponentFields(
			IntPtr objectPtr,
			FieldMetaData* fieldArray,
			uint fieldArrayCount,
			byte* arena,
			uint arenaSize
		) {
			try {
				GCHandle handleBack = GCHandle.FromIntPtr(objectPtr);
				if (handleBack.Target == null) {
					Grindstone.Logger.PrintError($"FillComponentFields: Failed to get object.");
					return;
				}

				object handleObj = handleBack.Target;

				Type? type = handleObj.GetType();
				if (type == null) {
					Grindstone.Logger.PrintError($"FillComponentFields: Failed to get type from object.");
					return;
				}

				FieldInfo[] fields = type.GetFields(BindingFlags.Public | BindingFlags.Instance);
				uint count = System.Math.Min((uint)fields.Length, fieldArrayCount);
				uint offset = 0;
				
				for(uint i = 0; i < count; ++i) {
					FieldInfo field = fields[i];
					InspectorFieldType fieldType = GetInspectorFieldType(field.FieldType);
					uint size = GetValueSize(fieldType);
					
					CopyStringToBuffer(field.Name, fieldArray[i].name, 128);
					CopyStringToBuffer(ParseDisplayName("", field.Name), fieldArray[i].displayName, 128);
					fieldArray[i].fieldType = fieldType;
					fieldArray[i].valueArenaOffset = offset;
					fieldArray[i].valueArenaSize = size;
					fieldArray[i].setter = Marshal.GetFunctionPointerForDelegate(MakeSetter(field));

					if (size > 0 && offset + size <= arenaSize) {
						WriteValueToArena(field, handleObj, fieldType, arena + offset, size);
					}

					offset += size;
				}
			}
			catch (Exception ex) {
				Grindstone.Logger.PrintError($"FillComponentFields Exception: {ex.Message}");
			}
		}
	}
}
