namespace Grindstone {
	public class SmartComponent {
		public readonly Entity entity;
		#region Public Methods
		#region Public Virtual Methods
		public virtual void OnStart() { }
		public virtual void OnAttach() { }
		public virtual void OnUpdate() { }
		public virtual void OnEditorUpdate() { }
		public virtual void OnDestroy() { }
		#endregion

		public void Print(Logger.LogSeverity severity, string message) {
			Logger.Print(severity, message);
		}

		public Entity GetEntity() {
			return entity;
		}

		public T CreateComponent<T>() {
			return entity.CreateComponent<T>();
		}

		public T GetComponent<T>() {
			return entity.GetComponent<T>();
		}

		public void DeleteComponent<T>() {
			entity.DeleteComponent<T>();
		}

		public TransformComponent GetTransform() {
			return GetComponent<TransformComponent>();
		}
		#endregion
	}
}
