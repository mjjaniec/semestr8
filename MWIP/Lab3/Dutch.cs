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
			
			//1 bit is input for Uf
			//2 bit is output fot Uf
			
			//At first we got superposiotn
			x.Hadamard(0);			
			
			
			x.Hadamard(1);
			x.SigmaX(0);
			
			//Here is our misterius function
			x.SigmaX(1);
			x.CNot(target: 0, control: 1);
			
			
			x.Hadamard(1);
			x.Measure(1);
			//result in bit 1. |0> means constatn |1> means variant
		}
	}
}
