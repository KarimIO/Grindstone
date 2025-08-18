using System.Reflection;
using System.Runtime.Loader;

namespace Grindstone {
	public class HotReloadContext : AssemblyLoadContext {
		private AssemblyDependencyResolver resolver;

		public HotReloadContext(string dllPath) : base(isCollectible: true) {
			resolver = new AssemblyDependencyResolver(dllPath);
		}

		protected override Assembly Load(AssemblyName assemblyName) {
			string? assemblyPath = resolver.ResolveAssemblyToPath(assemblyName);
			if (assemblyPath != null) {
				return LoadFromAssemblyPath(assemblyPath);
			}
			return null!;
		}
	}
}
