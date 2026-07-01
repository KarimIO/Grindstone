namespace Grindstone {
	// Wraps some entity functionality, contains the entity, and provides OnX methods to be called
	// by HostBridge.
	public class SmartComponent {
		private readonly Entity entity;
		#region Public Methods
		#region Public Virtual Methods
		public virtual void OnStart() { }
		public virtual void OnAttach() { }
		public virtual void OnUpdate() { }
		public virtual void OnEditorUpdate() { }
		public virtual void OnDestroy() { }
		#endregion

		public Entity GetEntity() {
			return entity;
		}

		public T? CreateComponent<T>() {
			return entity.CreateComponent<T>();
		}

		public T? GetComponent<T>() {
			return entity.GetComponent<T>();
		}

		public void DeleteComponent<T>() {
			entity.DeleteComponent<T>();
		}
		#endregion
	}
}
