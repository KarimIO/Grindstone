namespace Grindstone {
	class NativeComponentWrapperFactory {
		private static readonly Dictionary<Type, Func<IntPtr, object>> constructors = new();

		public static void Register<T>(Func<IntPtr, T> ctor) where T : class {
			constructors[typeof(T)] = (ptr => ctor(ptr)!);
		}

		public static T Wrap<T>(IntPtr ptr) where T : class {
			if (constructors.TryGetValue(typeof(T), out var ctor))
				return (T)ctor(ptr);

			throw new InvalidOperationException($"No wrapper registered for {typeof(T).Name}");
		}
	}
}
