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
			QuantumComputer comp = QuantumComputer.GetInstance();
			Register x = comp.NewRegister(0, 2);
			Complex[,] unitary_1_1 = new Complex[2, 2];
			unitary_1_1[0, 0] = new Complex(0.81649658092, 0);
			unitary_1_1[0, 1] = new Complex(0.57735026919, 0);
			unitary_1_1[1, 0] = new Complex(-0.57735026919, 0);
			unitary_1_1[1, 1] = new Complex(0.81649658092, 0);

			x.Gate1(unitary_1_1, 1);
			x.Hadamard(0, 1);
			x.Hadamard(0);
		}
	}
}
