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
			Register x = comp.NewRegister(0, 4);
			//Qbits 1,2,3 are input for Uf
			//Qbit 0 is output of Uf
			
			x.SigmaX(0);
			x.Hadamard(0);
			x.Hadamard(1);
			x.Hadamard(2);
			x.Hadamard(3);
			
			//This is misterious Uf
			x.CNot(target: 0, control: 3);
			x.CNot(target: 0, control: 1);
			//end of Uf
			
			x.Hadamard(0);
			x.Hadamard(1);
			x.Hadamard(2);
			x.Hadamard(3);
			
			//Now qbits 1, 2, 3 descirbes "okres" modulo 2
		}
	}
}
