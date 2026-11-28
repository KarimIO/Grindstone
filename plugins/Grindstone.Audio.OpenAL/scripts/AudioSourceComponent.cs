using System.Runtime.InteropServices;

namespace Grindstone {
	[System.Serializable]
	public struct AudioSourceComponent {
		public static string NativeComponentName => "AudioSource";

		public void Play() {
			AudioSourceComponentPlay(ref this);
		}
		public void Pause() {
			AudioSourceComponentPause(ref this);
		}
		public void Stop() {
			AudioSourceComponentStop(ref this);
		}

		#region DllImports

		[DllImport("PluginAudioOpenAL")]
		static extern void AudioSourceComponentPlay(ref AudioSourceComponent comp);

		[DllImport("PluginAudioOpenAL")]
		static extern void AudioSourceComponentPause(ref AudioSourceComponent comp);

		[DllImport("PluginAudioOpenAL")]
		static extern void AudioSourceComponentStop(ref AudioSourceComponent comp);
		#endregion
	}
}
