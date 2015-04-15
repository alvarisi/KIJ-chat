/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package kij.chat;

import java.util.Random;

/**
 *
 * @author alvarisi
 */
public class DH {
    public int getPrime() {
        int num = 0;
        Random rand = new Random(); // generate a random number
        num = rand.nextInt(1000) + 1;

        while (!isPrime(num)) {          
            num = rand.nextInt(1000) + 1;
        }
        return num;
    }

    
    private static boolean isPrime(int inputNum){
        if (inputNum <= 3 || inputNum % 2 == 0) 
            return inputNum == 2 || inputNum == 3;
        int divisor = 3;
        while ((divisor <= Math.sqrt(inputNum)) && (inputNum % divisor != 0)) 
            divisor += 2; 
        return inputNum % divisor != 0;
    }
    
        public int getPrimitiveValue(int p) {	
		// Loop through all bases.
		int cnt = 0;
		for (int a=1; a<p; a++) {
		
			// See if a is a primitive root.
			if (primitiveRoot(a, p)) {
				System.out.print(a+" ");
				cnt++;
			}
			
			if (cnt%30 == 0)
				System.out.println();
		}
		
            return cnt;
	}	
		
	// Pre-condition: p is prime and 1 < a < p.
	// Post-condition: returns true iff a is a primitive root of p.
	public boolean primitiveRoot(int a, int p) {
            int val = a, i;
            for (i=2; i<p; i++) {
                val = val*a%p;
                if (val == 1)
                    break;
            }
            return (i == p-1);
	}
    public int getPrivate(int q){
        int num = 0;
        Random rand = new Random(); 
        num = rand.nextInt(1000) + 1;
        int result;
        result = num%q;
        return result;
    }
    public int getPublicKey(int a, int xa, int q)
    {
            int Ya;
            Ya = ((int)(Math.pow(a,xa)) % q);
            return Ya;
    }
}