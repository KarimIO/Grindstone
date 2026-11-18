namespace Grindstone {
	public struct Hash {
		private readonly ulong value = 0;

		public readonly ulong GetValue() {
			return value;
		}

		public Hash(ulong value) {
			this.value = value;
		}

		public static Hash MurmurOAAT64(string key) {
			ulong h = 525201411107845655;
			foreach (char c in key) {
				h ^= c;
				h *= 0x5bd1e9955bd1e995;
				h ^= h >> 47;
			}

			return new Hash();
		}
	}
}

