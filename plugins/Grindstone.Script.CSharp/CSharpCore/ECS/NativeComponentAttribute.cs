namespace Grindstone {
	[AttributeUsage(AttributeTargets.Class)]
	public class NativeComponentAttribute : Attribute {
		public string ComponentName { get; }
		public NativeComponentAttribute(string componentName) {
			ComponentName = componentName;
		}
	}
}
