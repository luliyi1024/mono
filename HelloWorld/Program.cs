﻿using System;

namespace HelloWorld
{
    class Program
    {
        static void Main(string[] args)
        {
            TestClass t = new TestClass();
            int c = t.IntAdd(5,4);
            t.PrintT<int>(c);
            c = t.IntAdd2(5, 4);
            t.PrintT<int>(c);
        }
    }

    class Test2Class
    {
        int m=0;

        public int test()
        {
            return m;
        }
    }

    class TestClass
    {
        public System.Collections.Generic.List<Test2Class> m_test2List=new System.Collections.Generic.List<Test2Class>();

        public int IntAdd(int a, int b)
        {
            return a + b;
        }

        public int IntAdd2(int a, int b)
        {
            return m_test2List.Count;
        }

        public void PrintT<T>(T o)
        {
            System.Console.WriteLine("HelloWorld!");
            System.Console.WriteLine(o.ToString());
        }
    }
}
