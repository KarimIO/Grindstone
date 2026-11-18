using System.Runtime.InteropServices;

namespace Grindstone.Physics.Jolt {
	[System.Serializable]
	public struct RigidbodyComponent {
		public static string NativeComponentName => "RigidBody";

		public bool IsStatic {
			get => RigidbodyComponentGetIsStatic(ref this);
			set => RigidbodyComponentSetIsStatic(ref this, value);
		}

		public int Layer {
			get => RigidbodyComponentGetLayer(ref this);
			set => RigidbodyComponentSetLayer(ref this, value);
		}

		public float Mass {
			get => RigidbodyComponentGetMass(ref this);
		}

		public float Friction {
			get => RigidbodyComponentGetFriction(ref this);
			set => RigidbodyComponentSetFriction(ref this, value);
		}

		public float Restitution {
			get => RigidbodyComponentGetRestitution(ref this);
			set => RigidbodyComponentSetRestitution(ref this, value);
		}

		public Grindstone.Math.Float3 Position {
			get => RigidbodyComponentGetPosition(ref this);
			set => RigidbodyComponentSetPosition(ref this, value);
		}

		public Grindstone.Math.Quaternion Rotation {
			get => RigidbodyComponentGetRotation(ref this);
			set => RigidbodyComponentSetRotation(ref this, value);
		}

		public void ApplyForce(Grindstone.Math.Float3 pos, Grindstone.Math.Float3 force) {
			RigidbodyComponentApplyForce(ref this, pos, force);
		}

		public void ApplyCentralForce(Grindstone.Math.Float3 force) {
			RigidbodyComponentApplyCentralForce(ref this, force);
		}

		public void ApplyImpulse(Grindstone.Math.Float3 pos, Math.Float3 force) {
			RigidbodyComponentApplyImpulse(ref this, pos, force);
		}

		public void ApplyCentralImpulse(Grindstone.Math.Float3 force) {
			RigidbodyComponentApplyCentralImpulse(ref this, force);
		}


		#region DllImports

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentApplyForce(ref RigidbodyComponent comp, Grindstone.Math.Float3 pos, Grindstone.Math.Float3 force);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentApplyCentralForce(ref RigidbodyComponent comp, Grindstone.Math.Float3 force);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentApplyImpulse(ref RigidbodyComponent comp, Grindstone.Math.Float3 pos, Grindstone.Math.Float3 force);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentApplyCentralImpulse(ref RigidbodyComponent comp, Grindstone.Math.Float3 force);

		[DllImport("PluginJoltPhysics")]
		static extern bool RigidbodyComponentGetIsStatic(ref RigidbodyComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetIsStatic(ref RigidbodyComponent comp, bool isStatic);

		[DllImport("PluginJoltPhysics")]
		static extern int RigidbodyComponentGetLayer(ref RigidbodyComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetLayer(ref RigidbodyComponent comp, int layer);

		[DllImport("PluginJoltPhysics")]
		static extern float RigidbodyComponentGetMass(ref RigidbodyComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern float RigidbodyComponentGetFriction(ref RigidbodyComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetFriction(ref RigidbodyComponent comp, float friction);

		[DllImport("PluginJoltPhysics")]
		static extern float RigidbodyComponentGetRestitution(ref RigidbodyComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetRestitution(ref RigidbodyComponent comp, float restitution);

		[DllImport("PluginJoltPhysics")]
		static extern Grindstone.Math.Float3 RigidbodyComponentGetPosition(ref RigidbodyComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetPosition(ref RigidbodyComponent comp, Grindstone.Math.Float3 position);

		[DllImport("PluginJoltPhysics")]
		static extern Grindstone.Math.Quaternion RigidbodyComponentGetRotation(ref RigidbodyComponent comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetRotation(ref RigidbodyComponent comp, Grindstone.Math.Quaternion rotation);
		#endregion
	}
}
