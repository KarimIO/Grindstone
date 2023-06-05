namespace Grindstone {
	public struct Rect {
		public uint left;
		public uint top;
		public uint right;
		public uint bottom;

		public Rect(uint left, uint top, uint right, uint bottom) {
			this.left = left;
			this.top = top;
			this.right = right;
			this.bottom = bottom;
		}
	}
}
