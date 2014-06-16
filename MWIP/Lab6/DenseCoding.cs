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
		
			//Qbits are now entangled. 
			//Qbit 1 belongs to Alice
			//Qbit 0 belongs to Bob		
			x.Hadamard(1);
			x.CNot(target: 0, control: 1);
			
			//Now she applays  1, X, Z or XZ transformation on her Qubit
			//To send 00, 01, 10 or 11 accordingly
			//Using controled gates she can do it by setting qubits 2 and 3 insetad of putting phisical gates
			x.CNot(target: 1, control: 2);
			x.SigmaZ(1, 3);
			
			//Bob can revert the process to get initall state
			//I do not really get the point cause altough data was sent via one qbit 2 qubits was sent anyway 
			x.CNot(target: 0, control: 1);
			x.Hadamard(1);
		}
	}
}
