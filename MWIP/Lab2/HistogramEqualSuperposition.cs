using Quantum;
using Quantum.Operations;
using System;
using System.Numerics;
using System.Collections.Generic;

namespace QuantumConsole
{
	public class QuantumTest
	{
		public static void Main()
		{
			int[] t = new int [4];
			for (int i = 0; i < 400; ++i) {
				QuantumComputer comp = QuantumComputer.GetInstance();
				Register x = comp.NewRegister(0, 2);
				Register root = comp.GetRootRegister(x);
				x.Hadamard(0);
				x.Hadamard(1);
				t[2*x.Measure(0) + 	x.Measure(1)] ++;
			}
			Console.WriteLine(t[0] + " " + t[1] + " " + t[2] + " " + t[3]);
		}
	}
}
