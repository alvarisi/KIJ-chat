/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package kij.chat;

/**
 *
 * @author fadri
 */
public class RC4 {
    public static void main(String[] args) throws Exception
    {
        byte[] key = "SangatRahasiaSekali".getBytes("UTF-8");
        RC4algo data = new RC4algo(key);
        byte[] plainText = "hehehe".getBytes("UTF-8");
        byte[] chiperText = data.encrypt(plainText);
        String Chiper = new String(chiperText, "UTF-8");
        System.out.println("\nChiper : " + Chiper);
        RC4algo data2 = new RC4algo(key);
        String Plain = new String(data2.decrypt(chiperText), "UTF-8");
        System.out.println("\nPlain : " + Plain);
    }
}
