using System;

public class Example : Grindstone.SmartComponent {
	#region Public Fields
	public int id;
	#endregion

	#region Event Methods
	public override void OnAttachComponent() {
		Grindstone.Logger.Print("Entity Attached");
	}

	public override void OnStart() {
		Grindstone.Logger.Print("Entity Start");
	}

	public override void OnUpdate() {
		Grindstone.TransformComponent transf;
		transf = entity.GetTransformComponent();
		double time = Grindstone.Time.GetTimeSinceLaunch();
		transf.Position = GetNewPosition(time);

		var tagComp = entity.GetTagComponent();
		var tag = tagComp.Tag;
		Grindstone.Logger.Print($"Tag({tag})");
	}

	public override void OnEditorUpdate() {
		Grindstone.Logger.Print($"Entity Editor Update() {id++}");
	}

	~Example() {
		Grindstone.Logger.Print("Entity destructed");
	}
	#endregion

	#region Private Methods
	Grindstone.Math.Float3 GetNewPosition(double time) {
		float x = (float)Math.Sin(time);
		float y = 0;
		float z = (float)Math.Cos(time);

		return new Grindstone.Math.Float3(x, y, z) * 2.0f;
	}
	#endregion
}
