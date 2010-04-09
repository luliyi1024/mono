//
// JSFunctionAttribute.cs:
//
// Author: Cesar Octavio Lopez Nataren
//
// (C) 2003, 2004 Cesar Octavio Lopez Nataren, <cesar@ciencias.unam.mx>
//

//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;

namespace Microsoft.JScript {

	[AttributeUsage (AttributeTargets.Method | AttributeTargets.Constructor)]
	public class JSFunctionAttribute : Attribute {

		JSFunctionAttributeEnum value;
		JSBuiltin built_in_function;

		public JSFunctionAttribute (JSFunctionAttributeEnum value)
		{
			this.value = value;
			this.built_in_function = (JSBuiltin) 0;
		}

		public JSFunctionAttribute (JSFunctionAttributeEnum value, JSBuiltin built_in_function)
		{
			this.value = value;
			this.built_in_function = built_in_function;
		}

		public JSFunctionAttributeEnum GetAttributeValue ()
		{
			return value;
		}
		
		internal bool IsBuiltIn {
			get { return built_in_function != (JSBuiltin) 0; }
		}

		internal JSBuiltin BuiltIn {
			get { return built_in_function; }
		}
	}
}