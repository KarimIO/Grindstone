using System.Runtime.InteropServices;

namespace Grindstone.Physics.Jolt {
	public class CharacterRigidbodyControllerComponent {
		System.IntPtr componentPtr;

		public CharacterRigidbodyControllerComponent(System.IntPtr componentPtr) {
			this.componentPtr = componentPtr;
		}

		public CharacterRigidbodyControllerComponent(Grindstone.Entity entity) {
			componentPtr = EntityGetCharacterRigidbodyControllerComponent(entity.scene.sceneIntPtr, entity.entityHandle);
		}

		public bool IsOnGround {
			get => CharacterRigidbodyControllerComponentGetIsOnGround(componentPtr);
		}

		public Grindstone.Math.Float3 Velocity {
			get => CharacterRigidbodyControllerComponentGetVelocity(componentPtr);
			set => CharacterRigidbodyControllerComponentSetVelocity(componentPtr, value);
		}

		public Grindstone.Math.Quaternion Rotation {
			get => CharacterRigidbodyControllerComponentGetRotation(componentPtr);
			set => CharacterRigidbodyControllerComponentSetRotation(componentPtr, value);
		}

		#region DllImports
		[DllImport("PluginJoltPhysics")]
		public static extern System.IntPtr EntityGetCharacterRigidbodyControllerComponent(System.IntPtr scene, uint entity);

		[DllImport("PluginJoltPhysics")]
		static extern bool CharacterRigidbodyControllerComponentGetIsOnGround(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern Grindstone.Math.Float3 CharacterRigidbodyControllerComponentGetVelocity(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern void CharacterRigidbodyControllerComponentSetVelocity(System.IntPtr comp, Grindstone.Math.Float3 velocity);

		[DllImport("PluginJoltPhysics")]
		static extern Grindstone.Math.Quaternion CharacterRigidbodyControllerComponentGetRotation(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern void CharacterRigidbodyControllerComponentSetRotation(System.IntPtr comp, Grindstone.Math.Quaternion rotation);
		#endregion
	}
}
