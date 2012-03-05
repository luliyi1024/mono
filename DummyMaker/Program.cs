using System;
using System.Collections.Generic;
using Mono.Cecil;
using System.Collections;

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

        private static string GetSystemTypeName(TypeReference typeRef)
        {
            if (typeRef is ArrayType)
            {
                ArrayType atype = typeRef as ArrayType;
                return GetSystemTypeNameString(atype.ElementType.FullName)+"[]";
            }
            else if (typeRef is ReferenceType)
            {
                ReferenceType rtype = typeRef as ReferenceType;
                return GetSystemTypeNameString(rtype.ElementType.FullName) + "&";
            }
            else
            {
                return GetSystemTypeNameString(typeRef.FullName);
            }
        }

        private static string GetSystemTypeNameString(string fullname)
        {
            if (fullname == "System.Void") return "void";

            else if (fullname == "System.Char") return "char";
            else if (fullname == "System.Boolean") return "bool";
            else if (fullname == "System.Byte") return "byte";
            else if (fullname == "System.SByte") return "sbyte";
            else if (fullname == "System.UInt16") return "uint16";
            else if (fullname == "System.Int16") return "int16";
            else if (fullname == "System.UInt32") return "uint";
            else if (fullname == "System.Int32") return "int";
            else if (fullname == "System.UInt64") return "ulong";
            else if (fullname == "System.Int64") return "long";
            else if (fullname == "System.UIntPtr") return "uintptr";
            else if (fullname == "System.IntPtr") return "intptr";
            else if (fullname == "System.Single") return "single";
            else if (fullname == "System.Double") return "double";
            else if (fullname == "System.String") return "string";
            else if (fullname == "System.Object") return "object";

            else return fullname;
        }

        private static string GetParameterTypeName(ParameterDefinition param)
        {
            if (param.ParameterType is GenericInstanceType)
            {
                GenericInstanceType gtype = param.ParameterType as GenericInstanceType;

                string fullname = gtype.Namespace + "." + gtype.Name+"<";
                for (int i = 0; i < gtype.GenericArguments.Count; i++)
                {
                    TypeReference argType = gtype.GenericArguments[i];

                    fullname += GetSystemTypeName(argType);
                    if (i != (gtype.GenericArguments.Count - 1))
                        fullname += ", ";
                }
                fullname += ">";
                return fullname;
            }
            else 
                return GetSystemTypeName(param.ParameterType);
        }

        private static string GetMethodFullName(MethodDefinition method)
        {
            TypeDefinition type = method.DeclaringType;
            string fullname = type.FullName + ":"+method.Name+" (";

            for (int i = 0; i < method.Parameters.Count; i++)
            {
                ParameterDefinition param = method.Parameters[i];

                fullname += GetParameterTypeName(param);
                if (i != (method.Parameters.Count - 1))
                    fullname += ",";
            }

            fullname += ")";
            return fullname;
        }

        private static Dictionary<string, string> ReadDefMethodMap(string defFile) 
        {
            Dictionary<string, string> methodMap = new Dictionary<string, string>();
            string[] methods = System.IO.File.ReadAllLines(defFile);
            foreach (string s in methods)
            {
                string prefix = "Compile:           ";
                if (s.Substring(0, prefix.Length) == prefix)
                {
                    string methodFullname = s.Substring(prefix.Length);
                    if (!methodMap.ContainsKey(methodFullname))
                        methodMap.Add(methodFullname, methodFullname);
                }
            }
            System.Console.WriteLine("Find " + methodMap.Count + " methods in define file.");
            return methodMap;
        }

        private static ArrayList CollectMethods(AssemblyDefinition assembly)
        {
            ArrayList methodList = new ArrayList();

            foreach (TypeDefinition asType in assembly.MainModule.Types)
            {
                //System.Console.WriteLine("---- "+asType.FullName);
                methodList.AddRange(asType.Constructors);
                methodList.AddRange(asType.Methods);
            }

            return methodList;
        }

        static void Main(string[] args)
        {
            if (args.Length <= 0)
            {
                System.Console.WriteLine("need filename");
                return;
            }

            string assemblyName = args[0];

            Dictionary<string,string> defMethodMap = null;

            // 是否从指定文件中获取需要处理的函数列表
            bool useDefinedMethods = false;

            if (args.Length >= 2)
            {
                try
                {
                    defMethodMap = ReadDefMethodMap(args[1]);
                    useDefinedMethods = true;
                }
                catch (Exception e)
                {
                    System.Console.WriteLine(e.Message);
                    return;
                }
            }

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

            Mono.Cecil.Binary.Image image = assembly.MainModule.Image;

            ArrayList methodList = CollectMethods(assembly);

            byte[] dummyBuffer = System.IO.File.ReadAllBytes(assemblyName);
            System.IO.Stream stream = new System.IO.MemoryStream(dummyBuffer);

            System.Console.WriteLine("Assembly: " + assembly.Name);
            System.Console.WriteLine();

            Random random = new Random();

            int methodRewrited = 0;

            foreach (MethodDefinition method in methodList)
            {
                string methodFullName = GetMethodFullName(method);

                if (useDefinedMethods && defMethodMap.ContainsKey(methodFullName))
                {
                    System.Console.WriteLine(methodFullName);
                    methodRewrited++;
                }
                else
                {
                    System.Console.WriteLine("!Skip    " + methodFullName);
                    continue;
                }

                //System.Console.Write("  + " + method.Name/* + ":" + method.RVA.Value.ToString("x") + ":" + offset.ToString("x")*/);

                if (method.RVA.Value == 0)
                {
                    System.Console.WriteLine("(RVA is 0, skip)");
                    continue;
                }

                // RVA to offset
                long offset = image.ResolveVirtualAddress(method.RVA);
                offset += GetMethodHeaderSize(image, method);
                long size = method.Body.CodeSize;

                stream.Seek(offset, System.IO.SeekOrigin.Begin);

                stream.WriteByte(0xEE);
                for (int i = 1; i < size; ++i)
                {
                    stream.WriteByte((byte)random.Next(128));
                }
                stream.Flush();
            }

            stream.Close();

            System.Console.WriteLine();
            System.Console.WriteLine("methodTotel: " + methodList.Count + "    methodRewrited: " + methodRewrited);
            float percent = methodRewrited * 100 / methodList.Count;
            System.Console.WriteLine(percent+"% methods rewrited.");

            string dummyName = assemblyName + ".dummy";
            System.IO.File.WriteAllBytes(dummyName,dummyBuffer);
            System.Console.WriteLine(dummyName+" is writed.");
        }
    }
}
