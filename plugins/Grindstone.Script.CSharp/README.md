# C# Scripting support

This plugin is developed by the Grindstone Foundation. It's designed to allow developers to extend gameplay using C#, a simpler language to C++.

## Registered Items

### Script (Component)

This is a system that adds a Smart Component script object to an entity.

### CSharp Update (System)

This is a system that runs script update loops only for gameplay (not editors).

### CSharp Editor Update (System)

This is a system that runs script update loops only for editors.

## Example Script

Here is an example script that utilizes input features and the OnUpdate loop to create a moving player.

using Grindstone.Math;
using System;

public class Player : Grindstone.SmartComponent
{
    #region Public Fields
    public float speed = 10.0f;
    #endregion

    #region Private Fields
    private EulerAngles lookEuler = new EulerAngles(0.0f, 0.0f, 0.0f);
    private bool isFirstFrame = true;
    #endregion

    #region Event Methods
    public override void OnUpdate() {
        if (isFirstFrame)
        {
            Grindstone.Input.InputManager.CursorMode = Grindstone.Input.CursorMode.Disabled;
            Grindstone.Input.InputManager.IsMouseRawMotion = true;
        }

        bool isWindowFocused = Grindstone.Input.InputManager.IsWindowFocused;
        if (!isWindowFocused)
        {
            return;
        }

        Grindstone.Window window = Grindstone.Window.Current;
        if (Grindstone.Input.InputManager.IsKeyDown(Grindstone.Input.KeyboardKey.Escape))
        {
            window.Close();
        }

        Float2 halfWindowSize = window.Size / 2.0f;
        if (isFirstFrame) {
            Grindstone.Input.InputManager.MousePosition = halfWindowSize;
            isFirstFrame = false;
            return;
        }

        Float2 mousePos = Grindstone.Input.InputManager.MousePosition;
        Float2 lookVec = new Float2(
            (halfWindowSize.x - mousePos.x) / halfWindowSize.x,
            (halfWindowSize.y - mousePos.y) / halfWindowSize.y
        );
        Grindstone.Input.InputManager.MousePosition = halfWindowSize;

        bool w = Grindstone.Input.InputManager.IsKeyDown(Grindstone.Input.KeyboardKey.W);
        bool s = Grindstone.Input.InputManager.IsKeyDown(Grindstone.Input.KeyboardKey.S);
        bool a = Grindstone.Input.InputManager.IsKeyDown(Grindstone.Input.KeyboardKey.A);
        bool d = Grindstone.Input.InputManager.IsKeyDown(Grindstone.Input.KeyboardKey.D);

        float fwd = (w ? 1.0f : 0.0f) - (s ? 1.0f : 0.0f);
        float rgt = (d ? 1.0f : 0.0f) - (a ? 1.0f : 0.0f);
        var transf = entity.GetTransformComponent();
        Float3 movementDirection =
            transf.Forward * fwd +
            transf.Right * rgt;

        float dt = (float)Grindstone.Time.GetDeltaTime();
        Float3 offset = movementDirection * dt * speed;

        float mouseSensitivity = 20.0f;
        lookEuler.pitch += mouseSensitivity * lookVec.x * dt;
        lookEuler.yaw += mouseSensitivity * lookVec.y * dt;

        const float maxViewAngle = (float)Math.PI * 0.45f;

        if (lookEuler.yaw > maxViewAngle)
        {
            lookEuler.yaw = maxViewAngle;
        }

        if (lookEuler.yaw < -maxViewAngle)
        {
            lookEuler.yaw = -maxViewAngle;
        }

        transf.Rotation = new Quaternion(lookEuler);
        transf.Position += offset;
    }
    #endregion
}

