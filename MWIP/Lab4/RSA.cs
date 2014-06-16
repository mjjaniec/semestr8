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
			
			QuantumComputer comp = QuantumComputer.GetInstance();
			
			//input register
			Register regX = comp.NewRegister(0, 2 * width);
			
			// output register (must contain 1):
			Register regX1 = comp.NewRegister(1, width + 1);
			
			// perform Walsh-Hadamard transform on the input register
			// input register can contains N^2 so it is 2*width long
			comp.Walsh(regX);
			
			// perform exp_mod_N
			comp.ExpModulo(regX, regX1, a, N);
			
			// output register is no longer needed
			regX1.Measure();
			
			// perform Quantum Fourier Transform on the input register
			comp.QFT(regX);
			
			comp.Reverse(regX);
			
			// getting the input register
			int Q = (int)(1 << 2 * width);
			int inputMeasured = (int)regX.Measure();
			
			
			Tuple<int, int> result = FractionalApproximation(inputMeasured, Q, 2 * width - 1);
 
			
			int period = result.Item2;
			
			if(BigInteger.ModPow(a, period, N) == 1) {
				return period;
			}
			
			int maxMult = (int)(Math.Sqrt(N)) + 1;
			int mult = 2;
			while(mult < maxMult) 
			{
				period = result.Item2 * mult;
				if(BigInteger.ModPow(a, period, N) == 1) 
				{
					return period;
				}
				else 
				{		
					mult++;
				}
			}
			
			return -1;
		}
		
		public static int FindPeriodBardzo(int N, int a) 
		{
			int result;
			for (int i = 0; i<100; ++i)
			{
				result = FindPeriod(N,a);
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
				int gcd = GCD(N, a);
				if (gcd != 1) 
				{
					return gcd;
				}
				
				r = FindPeriodBardzo(N, a);
				if (r % 2 == 1) 
				{
					continue;
				}
				r /= 2;
				
				pow = (int) BigInteger.ModPow(a, r, N);
				if (pow == N - 1)
				{
					continue;
				}
				
				return GCD(pow+1, N);
				
			}
			return 0;
		}
		
		static int modInverse(int a, int n) 
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
			int p = 3, q = 7;
			int N = p * q;
			int M = (p - 1) * (q - 1);
			int message = 4;
			int c = 7;
			int d = modInverse(c, M);
			int encoded = (int)  BigInteger.ModPow(message, c, N);
			Console.WriteLine("p: {0}, q: {1}, c: {2}, message: {3}, encodedMessage: {4}", p, q, c, message, encoded);
			Console.WriteLine("private key: {0}", d);
			if(message != (int) BigInteger.ModPow(encoded, d, N)) {
				Console.WriteLine("Bad!!!");
			}
			int divisor = Shor(N);				
			int second = N / divisor;
			int M2 = (divisor -1 )* (second -1);
			int d2 = modInverse(d, M);
			Console.WriteLine("broken private key: {0}", d2);
			int message2 = (int) BigInteger.ModPow(encoded, d2, N);
			Console.WriteLine("broken message: {0}", message2);
		}
	}
}