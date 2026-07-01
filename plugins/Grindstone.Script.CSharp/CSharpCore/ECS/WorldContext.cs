namespace Grindstone {
	public struct WorldContext {
		private IntPtr handle;

		public WorldContext(IntPtr handle) {
			this.handle = handle;
		}

		public readonly IntPtr GetHandle() {
			return handle;
		}
	}
}
