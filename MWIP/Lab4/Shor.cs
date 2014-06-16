using Quantum;
using Quantum.Operations;
using System;
using System.Numerics;
using System.Collections.Generic;

namespace QuantumConsole
{
	public class QuantumTest
	{	
		public static Tuple<int, int> FractionalApproximation(int a, int b, int width)
        {
            double f = (double)a / (double)b;
            double g = f;
            int i, num2 = 0, den2 = 1, num1 = 1, den1 = 0, num = 0, den = 0;
            int max = 1 << width;

            do
            {
                i = (int)g;  // integer part
                g = 1.0 / (g - i);  // reciprocal of the fractional part

                if (i * den1 + den2 > max) // if denominator is too big
                {
                    break;
                }

                // new numerator and denominator
                num = i * num1 + num2;
                den = i * den1 + den2;

                // previous nominators and denominators are memorized
                num2 = num1;
                den2 = den1;
                num1 = num;
                den1 = den;

            }
            while (Math.Abs(((double)num / (double)den) - f) > 1.0 / (2 * max));
            // this condition is from Shor algorithm

            return new Tuple<int, int>(num, den);
        }
        
        public static int FindPeriod(int N, int a) {
			ulong ulongN = (ulong)N;
			int width = (int)Math.Ceiling(Math.Log(N, 2));
 
			Console.WriteLine("Width for N: {0}", width);
			Console.WriteLine("Total register width (7 * w + 2) : {0}", 7 * width + 2);
			
			QuantumComputer comp = QuantumComputer.GetInstance();
			
			//input register
			Register regX = comp.NewRegister(0, 2 * width);
			
			// output register (must contain 1):
			Register regX1 = comp.NewRegister(1, width + 1);
			
			// perform Walsh-Hadamard transform on the input register
			// input register can contains N^2 so it is 2*width long
			Console.WriteLine("Applying Walsh-Hadamard transform on the input register...");
			comp.Walsh(regX);
			
			// perform exp_mod_N
			Console.WriteLine("Applying f(x) = a^x mod N ...");
			comp.ExpModulo(regX, regX1, a, N);
			
			// output register is no longer needed
			regX1.Measure();
			
			// perform Quantum Fourier Transform on the input register
			Console.WriteLine("Applying QFT on the input register...");
			comp.QFT(regX);
			
			comp.Reverse(regX);
			
			// getting the input register
			int Q = (int)(1 << 2 * width);
			int inputMeasured = (int)regX.Measure();
			Console.WriteLine("Input measured = {0}", inputMeasured);
			Console.WriteLine("Q = {0}", Q);
			
			Tuple<int, int> result = FractionalApproximation(inputMeasured, Q, 2 * width - 1);
 
			Console.WriteLine("Fractional approximation:  {0} / {1}", result.Item1, result.Item2);
			
			int period = result.Item2;
			
			if(BigInteger.ModPow(a, period, N) == 1) {
				Console.WriteLine("Success !!!    period = {0}", period);
				return period;
			}
			
			int maxMult = (int)(Math.Sqrt(N)) + 1;
			int mult = 2;
			while(mult < maxMult) 
			{
				Console.WriteLine("Trying multiply by {0} ...", mult);
				period = result.Item2 * mult;
				if(BigInteger.ModPow(a, period, N) == 1) 
				{
					Console.WriteLine("Success !!!    period = {0}", period);
					return period;
				}
				else 
				{		
					mult++;
				}
			}
			
			Console.WriteLine("Failure !!!    Period not found, try again.");
			return -1;
		}
		
		public static int FindPeriodBardzo(int N, int a) 
		{
			int result;
			for (int i = 0; i<100; ++i)
			{
				result = FindPeriod(N,a);
				Console.WriteLine("result period: " + result);
				if (result != -1) 
				{
					return result;
				}
			}
			return 0;
		}
		
		public static int Shor(int N) 
		{
			Random random = new Random();
			int a;
			int r=0;
			int pow;
			
			for (int i = 0; i<10; ++i)
			{
				a = random.Next(1, N);
				Console.WriteLine("lets try for: a: " + a + " N: " + N);
				int gcd = GCD(N, a);
				if (gcd != 1) 
				{
					Console.WriteLine("Succes random divisior");
					return gcd;
				}
				
				r = FindPeriodBardzo(N, a);
				Console.WriteLine("period: " + r);
				if (r % 2 == 1) 
				{
					Console.WriteLine("Odd period");
					continue;
				}
				r /= 2;
				
				pow = (int) BigInteger.ModPow(a, r, N);
				if (pow == N - 1)
				{
					Console.WriteLine("Bad period");
					continue;
				}
				
				return GCD(pow+1, N);
				
				Console.ReadLine();
			}
			return 0;
		}
		
		int modInverse(int a, int n) 
		{
		    int i = n, v = 0, d = 1;
		    while (a > 0) {
		        int t = i/a, x = a;
		        a = i % x;
		        i = x;
		        x = d;
		        d = v - t*x;
		        v = x;
		    }
		    v %= n;
		    if (v<0) v = (v+n)%n;
		    return v;
		}
		
		public static int GCD(int p, int q)
		{
		    if(q == 0)
		    {
		         return p;
		    }
		
		    int r = p % q;
		
		    return GCD(q, r);
		}

		
		public static void Main()
		{
			int N = 21;
			Console.WriteLine(Shor(N));				
		}
	}
}