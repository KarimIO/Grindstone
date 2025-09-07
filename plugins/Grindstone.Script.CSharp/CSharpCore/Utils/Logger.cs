using System.Runtime.InteropServices;

namespace Grindstone {
	public class Logger {
		#region Enums
		public enum LogSeverity {
			Info,
			Trace,
			Warning,
			Error
		};
		#endregion

		#region Static Methods
		public static void Print(LogSeverity logSeverity, string message) {
			LoggerLog((int)logSeverity, message);
		}

		public static void Print(string message) {
			LoggerLog((int)LogSeverity.Info, message);
		}

		public static void PrintInfo(string message) {
			LoggerLog((int)LogSeverity.Info, message);
		}

		public static void PrintTrace(string message) {
			LoggerLog((int)LogSeverity.Trace, message);
		}

		public static void PrintWarning(string message) {
			LoggerLog((int)LogSeverity.Warning, message);
		}

		public static void PrintError(string message) {
			LoggerLog((int)LogSeverity.Error, message);
		}
		#endregion

		#region DllImports
		[DllImport("PluginScriptCSharp.dll")]
		private static extern void LoggerLog(int logSeverity, string message);
		#endregion
	}
}
