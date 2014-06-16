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
			int a = 17, N = 77;
			int valueMeasured;
			for (int x = 0; x < 100; ++x) 
			{
			  //wczesniej nalezy zadeklarowac i zainicjalizowac x, N, a
			  
			        // obliczamy ile bitow potrzeba na zapamiętanie N
				int ulongN = N;
				int width = (int)Math.Ceiling(Math.Log(N, 2));
			 
				// inicjalizujemy komputer kwantowy 		
				QuantumComputer comp = QuantumComputer.GetInstance();
						
			 	//inicjalizujemy rejestr wejsciowy 
				Register regX = comp.NewRegister(0, 2 * width);
						
				// inicjalizujemy rejestr wyjsciowy 
				Register regY = comp.NewRegister(1, width + 1);
			 
				// ustawiamy wartosc rejestru wejsciowego na x 
				regX.Reset((ulong)x);
			 
				// ustawiamy wartosc rejestru wyjsciowego na 1
			        // potrzebne, gdy wywolujemy w petli 
				regY.Reset(1);
			 
			        // obliczamy a^x mod N
			 	comp.ExpModulo(regX, regY, a, N);
			 
			        //mierzymy wartosc
			 	valueMeasured = (int)regY.Measure();
			 
	        	Console.WriteLine ("Dla {0} reszta to {1}",x, valueMeasured);
        	}
		}
	}
}
