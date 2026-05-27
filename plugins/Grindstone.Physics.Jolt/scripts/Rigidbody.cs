using System.Runtime.InteropServices;

namespace Grindstone.Physics.Jolt {
	public class RigidbodyComponent {
		System.IntPtr componentPtr;

		public RigidbodyComponent(System.IntPtr componentPtr) {
			this.componentPtr = componentPtr;
		}

		public bool IsStatic {
			get => RigidbodyComponentGetIsStatic(componentPtr);
			set => RigidbodyComponentSetIsStatic(componentPtr, value);
		}

		public int Layer {
			get => RigidbodyComponentGetLayer(componentPtr);
			set => RigidbodyComponentSetLayer(componentPtr, value);
		}

		public float Mass {
			get => RigidbodyComponentGetMass(componentPtr);
		}

		public float Friction {
			get => RigidbodyComponentGetFriction(componentPtr);
			set => RigidbodyComponentSetFriction(componentPtr, value);
		}

		public float Restitution {
			get => RigidbodyComponentGetRestitution(componentPtr);
			set => RigidbodyComponentSetRestitution(componentPtr, value);
		}

		public Grindstone.Math.Float3 Position {
			get => RigidbodyComponentGetPosition(componentPtr);
			set => RigidbodyComponentSetPosition(componentPtr, value);
		}

		public Grindstone.Math.Quaternion Rotation {
			get => RigidbodyComponentGetRotation(componentPtr);
			set => RigidbodyComponentSetRotation(componentPtr, value);
		}

		public void ApplyForce(Grindstone.Math.Float3 pos, Grindstone.Math.Float3 force) {
			RigidbodyComponentApplyForce(componentPtr, pos, force);
		}

		public void ApplyCentralForce(Grindstone.Math.Float3 force) {
			RigidbodyComponentApplyCentralForce(componentPtr, force);
		}

		public void ApplyImpulse(Grindstone.Math.Float3 pos, Math.Float3 force) {
			RigidbodyComponentApplyImpulse(componentPtr, pos, force);
		}

		public void ApplyCentralImpulse(Grindstone.Math.Float3 force) {
			RigidbodyComponentApplyCentralImpulse(componentPtr, force);
		}


		#region DllImports
		[DllImport("PluginJoltPhysics")]
		public static extern System.IntPtr EntityGetRigidbodyComponent(System.IntPtr scene, uint entity);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentApplyForce(System.IntPtr comp, Grindstone.Math.Float3 pos, Grindstone.Math.Float3 force);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentApplyCentralForce(System.IntPtr comp, Grindstone.Math.Float3 force);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentApplyImpulse(System.IntPtr comp, Grindstone.Math.Float3 pos, Grindstone.Math.Float3 force);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentApplyCentralImpulse(System.IntPtr comp, Grindstone.Math.Float3 force);

		[DllImport("PluginJoltPhysics")]
		static extern bool RigidbodyComponentGetIsStatic(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetIsStatic(System.IntPtr comp, bool isStatic);

		[DllImport("PluginJoltPhysics")]
		static extern int RigidbodyComponentGetLayer(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetLayer(System.IntPtr comp, int layer);

		[DllImport("PluginJoltPhysics")]
		static extern float RigidbodyComponentGetMass(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern float RigidbodyComponentGetFriction(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetFriction(System.IntPtr comp, float friction);

		[DllImport("PluginJoltPhysics")]
		static extern float RigidbodyComponentGetRestitution(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetRestitution(System.IntPtr comp, float restitution);

		[DllImport("PluginJoltPhysics")]
		static extern Grindstone.Math.Float3 RigidbodyComponentGetPosition(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetPosition(System.IntPtr comp, Grindstone.Math.Float3 position);

		[DllImport("PluginJoltPhysics")]
		static extern Grindstone.Math.Quaternion RigidbodyComponentGetRotation(System.IntPtr comp);

		[DllImport("PluginJoltPhysics")]
		static extern void RigidbodyComponentSetRotation(System.IntPtr comp, Grindstone.Math.Quaternion rotation);
		#endregion
	}
}
