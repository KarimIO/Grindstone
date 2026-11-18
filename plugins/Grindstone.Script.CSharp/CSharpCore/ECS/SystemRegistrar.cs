using System;

namespace Grindstone {
	public class SystemRegistrar {
		#region Static Methods
		public static void AddSystem(string name, Action<WorldContext> system) { }
		public static void RemoveSystem(string name) { }
		#endregion
	}
}
