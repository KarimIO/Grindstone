using System.Runtime.InteropServices;

namespace Grindstone {
	public static class Time {
		public static double GetTimeSinceLaunch() {
			return TimeGetTimeSinceLaunch();
		}

		public static double GetDeltaTime() {
			return TimeGetDeltaTime();
		}

		[DllImport("EngineCore")]
		private static extern double TimeGetTimeSinceLaunch();

		[DllImport("EngineCore")]
		private static extern double TimeGetDeltaTime();
	}
}
