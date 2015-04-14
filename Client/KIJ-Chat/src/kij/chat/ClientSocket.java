/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package kij.chat;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.Random;
import java.util.Scanner;

/**
 *
 * @author varis
 */
public class ClientSocket implements Runnable {
    private static String hostname;
    private static int port;
    String key;
    Socket sock;
    Scanner input;
    Scanner send = new Scanner(System.in);
    PrintWriter out;
    String Sess_key;
    
    
    ClientSocket(String h, int p) throws IOException
    {
        hostname = h;
        port = p;
        key = generateKeyRandow();
        this.sock = new Socket(ClientSocket.hostname, ClientSocket.port);  
    }
    ClientSocket(Socket X) {
        // TODO Auto-generated constructor stub
        this.sock = X;
    }
    ClientSocket() {
        
    }
    
    public static void main(String[] args) throws Exception
    {
        
    }

    
    @Override
    public void run()
    {
        try{
            try {
                
                input = new Scanner(sock.getInputStream());
                out = new PrintWriter(sock.getOutputStream());
                out.flush();
                System.out.print("Run\n");
                CheckStream();
            } finally {
                sock.close();
            }
        }catch(Exception e){
            System.out.print(e);
        }
    }
    public void CheckStream() {
        while (true) {
            System.out.print("CheckStream\n");
            receive();
        }
    }
    public void receive() {
        System.out.print("Receive\n");
        if (input.hasNext()) {
            System.out.print("Receive-In\n");
            //Decrypt Sould be here
            //Start Decrypt
            
            
            //End Decrypt
            
            
            String msg = input.nextLine();
            ChatUI.receive(msg);
//            printHeader(msg);
//            System.out.print(msg);
//            List<String> items;
//            items = Arrays.asList(msg.split(":"));
//            receiveHandler(items);
        }
    }
    
    public void printHeader(String message)
    {
        ChatUI.TA_header.append(message);
    }
    
    private String generateKeyRandow()
    {
        char[] chars = "abcdefghijklmnopqrstuvwxyz1234567890".toCharArray();
        StringBuilder sb = new StringBuilder();
        Random random = new Random();
        for (int i = 0; i < 24; i++) {
            char c = chars[random.nextInt(chars.length)];
            sb.append(c);
        }
        String output = sb.toString();
        return output;
    }
}