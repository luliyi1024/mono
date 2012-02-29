using System;

namespace HelloWorld
{
    class Program
    {
        static void Main(string[] args)
        {
            TestClass t = new TestClass();
            int c = t.IntAdd(5,4);
            t.PrintT<int>(c);
        }
    }

    class TestClass
    {
        public int IntAdd(int a, int b)
        {
            return a + b;
        }

        public int IntAdd2(int a, int b)
        {
            return 0;
        }

        public void PrintT<T>(T o)
        {
            System.Console.WriteLine("HelloWorld!");
            System.Console.WriteLine(o.ToString());
        }
    }
}
