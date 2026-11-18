using System.Runtime.InteropServices;

namespace Grindstone.Physics.Jolt {
	[System.Serializable]
	public struct CharacterRigidbodyControllerComponent {
		public static string NativeComponentName => "CharacterRigidbodyController";

		public bool IsOnGround {
			get => CharacterRigidbodyControllerComponentGetIsOnGround(ref this);
		}

		public Grindstone.Math.Float3 Velocity {
			get => CharacterRigidbodyControllerComponentGetVelocity(ref this);
			set => CharacterRigidbodyControllerComponentSetVelocity(ref this, value);
		}

		public Grindstone.Math.Quaternion Rotation {
			get => CharacterRigidbodyControllerComponentGetRotation(ref this);
			set => CharacterRigidbodyControllerComponentSetRotation(ref this, value);
		}

		#region DllImports

		[DllImport("PluginJoltPhysics")]
		static extern bool CharacterRigidbodyControllerComponentGetIsOnGround(ref CharacterRigidbodyControllerComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern Grindstone.Math.Float3 CharacterRigidbodyControllerComponentGetVelocity(ref CharacterRigidbodyControllerComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern void CharacterRigidbodyControllerComponentSetVelocity(ref CharacterRigidbodyControllerComponent comp, Grindstone.Math.Float3 velocity);

		[DllImport("PluginJoltPhysics")]
		static extern Grindstone.Math.Quaternion CharacterRigidbodyControllerComponentGetRotation(ref CharacterRigidbodyControllerComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern void CharacterRigidbodyControllerComponentSetRotation(ref CharacterRigidbodyControllerComponent comp, Grindstone.Math.Quaternion rotation);
		#endregion
	}
}
