/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package kij.chat;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;

/**
 *
 * @author varis
 */
public class ClientSocket implements Runnable {
    private static String hostname;
    private static int port;
    Socket sock;
    Scanner input;
    Scanner send = new Scanner(System.in);
    PrintWriter out;
    String Sess_key;
    
    
    ClientSocket(String h, int p) throws IOException
    {
        hostname = h;
        port = p;
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
            
            String msg = input.nextLine();
            printHeader(msg);
            System.out.print(msg);
            List<String> items;
            items = Arrays.asList(msg.split(":"));
            receiveHandler(items);
        }
    }
    public void receiveHandler(List<String> items)
    {
        System.out.print("Handler-In\n");
        System.out.print("nilai item" + items.get(0));

        switch(items.get(0))
        {
            case "RTR":
                System.out.print("Handler-RTR\n");
                if("SUCCESSLOGIN".equals(items.get(1)))
                {
                    System.out.print("Handler-SUCCESSLOGIN\n");
                    ChatUI.username = items.get(2);
                    ChatUI.BuildMainWindow();
                    out.println("REQ:LIST:SESSION:"+ Sess_key +":!>");
                    out.flush();
                    System.out.print("SentList\n");
                }
                else if("SUCCESSLIST".equals(items.get(1)))
                {
                    ChatUI.C_list_online.removeAllItems();
//                    List<String> userActive = new ArrayList<String>();
                    for(int i=0;i<items.size();i++)
                    {
                        if("DATA".equals(items.get(i)))
                        {
                            for(int j=i+1;j<items.size();j++)
                            {
                                if("!>".equals(items.get(j))==false)
                                {
//                                    userActive.add(items.get(j));
                                    ChatUI.C_list_online.addItem(items.get(j));
                                }
                                i++;
                            }
                        }
                    }
//                    String[] simpleArray = new String[userActive.size()];
//                    userActive.toArray(simpleArray);
//                    ChatUI.JL_online.setListData(simpleArray);
                }
                else if("SUCCESSREGISTER".equals(items.get(1)))
                {
                    System.out.print("Handler-SUCCESSREGISTER\n");
                    ChatUI.L_notiflogin.setText("Anda telah terdaftar. Silakan login");
                }
                else if("RCHAT".equals(items.get(1)))
                {
                    String sender = items.get(3);
                    String message = items.get(4);
                    printConversation(sender + " : " + message);
                }
                break;
            case "RCV":
                Sess_key = items.get(2);
                break;
            default:
                break;
        }
    }
    public void printHeader(String message)
    {
        ChatUI.TA_header.append(message);
    }
    public void printConversation(String message)
    {
        ChatUI.TA_conversation.append(message);
    }
}