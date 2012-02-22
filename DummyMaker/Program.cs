using System;
using System.Collections.Generic;
using Mono.Cecil;

namespace DummyMaker
{
    class Program
    {
        private static int GetMethodHeaderSize(Mono.Cecil.Binary.Image image, MethodDefinition method)
        {
            int headerSize = 1;

            // check the method header is tiny or fat
            System.IO.BinaryReader br = image.GetReaderAtVirtualAddress(method.RVA);

            byte flag = br.ReadByte();

            uint CorILMethod_FormatMask = ((1 << 3) - 1);
            if ((flag & (CorILMethod_FormatMask >> 1)) == 0x2)
            {
                //System.Console.WriteLine("Tiny header!");
                headerSize = 1;
            }
            else
            {
                //System.Console.WriteLine("Fat header!");
                headerSize = 12;
            }

            br.Close();
            return headerSize;
        }

        static void Main(string[] args)
        {
            if (args.Length <= 0)
            {
                System.Console.WriteLine("need filename");
                return;
            }

            string assemblyName = args[0];

            AssemblyDefinition assembly;

            try
            {
                assembly = AssemblyFactory.GetAssembly(assemblyName);
            }
            catch (Exception e)
            {
                System.Console.WriteLine(e.Message);
                return;
            }

            byte[] dummyBuffer = System.IO.File.ReadAllBytes(assemblyName);
            System.IO.Stream stream = new System.IO.MemoryStream(dummyBuffer);

            Mono.Cecil.Binary.Image image = Mono.Cecil.Binary.Image.GetImage(assemblyName);

            System.Console.WriteLine(assembly.Name);

            Random random = new Random();

            foreach (Mono.Cecil.TypeDefinition asType in assembly.MainModule.Types)
            {
                System.Console.WriteLine(asType.Name);
                foreach(MethodDefinition method in asType.Methods)
                {
                    // RVA to offset
                    long offset = image.ResolveVirtualAddress(method.RVA);
                    offset += GetMethodHeaderSize(image, method);
                    long size = method.Body.CodeSize;

                    System.Console.WriteLine("  + " + method.Name + ":" + method.RVA.Value.ToString("x") + ":" + offset.ToString("x"));

                    stream.Seek(offset, System.IO.SeekOrigin.Begin);
                   
                    for (int i = 0; i < size; ++i)
                    {
                        stream.WriteByte((byte)random.Next(256));
                    }
                    stream.Flush();
                }
                System.Console.WriteLine();
            }

            stream.Close();
            string dummyName = "dummy_" + assemblyName;
            System.IO.File.WriteAllBytes(dummyName,dummyBuffer);
            System.Console.WriteLine(dummyName+" is writed.");
        }
    }
}
