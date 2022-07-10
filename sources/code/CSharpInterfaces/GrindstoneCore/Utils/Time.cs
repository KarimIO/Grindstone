using System.Runtime.InteropServices;

namespace Grindstone {
	public static class Time {
		#region Static Methods
		public static double GetTimeSinceLaunch() {
			return TimeGetTimeSinceLaunch();
		}

		public static double GetDeltaTime() {
			return TimeGetDeltaTime();
		}
		#endregion

		#region DllImports
		[DllImport("EngineCore")]
		private static extern double TimeGetTimeSinceLaunch();

		[DllImport("EngineCore")]
		private static extern double TimeGetDeltaTime();
		#endregion
	}
}
