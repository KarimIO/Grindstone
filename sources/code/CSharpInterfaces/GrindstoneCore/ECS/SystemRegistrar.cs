using System;

namespace Grindstone {
	public class SystemRegistrar {
		public static void AddSystem(string name, Action<Scene> system) { }
		public static void RemoveSystem(string name) { }
	}
}
