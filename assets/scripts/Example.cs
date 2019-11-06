using System;

public class Example : BaseComponent
{
    public int Id;
    private String Name;

    public Example()
    {
        this.Name = "Karim";
        System.Console.WriteLine("Entity constructed");
    }

    public override void start()
    {
        System.Console.WriteLine("Entity Start");
    }

    public override void update()
    {
        System.Console.WriteLine("Entity Update()");
    }

    ~Example()
    {
        System.Console.WriteLine("Entity destructed");
    }

    public void Process()
    {
        throw new NotImplementedException("Not implemented yet");
    }

    public String GetName()
    {
        return Name;
    }
}