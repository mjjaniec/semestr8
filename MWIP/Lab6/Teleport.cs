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
			Register x = comp.NewRegister(0, 3);
			
			//Qbit 2 is one we want to teleport
			//Qbits 1, 2 are helpers
			
			//First we entangle Qbits 0 and 1
			//Qbit 1 is hold by Alice
			//Qbit 0 is send to Bob 
			x.Hadamard(1);
			x.CNot(target: 0, control: 1);
			
			//Now we applay some transormations
			//Becouse qbits 0 and 1 are entangled it affects also qbit 1
			x.CNot(target: 1, control: 2);
			x.Hadamard(2);
			//After this manipulation Bob may have teleported state
			//Or may need to do some additional transormation. Measurment will say
			
			x.Measure(1);
			x.Measure(2);
			//Controlled gates allows to do appropiate manipulation if required
			x.CNot(target: 0, control: 1);
			x.SigmaZ(0, 2);
			
			//Bob has finally his teleported state
		}
	}
}
