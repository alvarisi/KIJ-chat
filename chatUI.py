from Tkinter import *
import tkMessageBox
import socket
import select
import sys
import json
import time
import thread
import os
server_address = (ipaddress, int(port))
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(server_address)
BUFF = 1024

try:
	while 1:
		socket_list = [sys.stdin, client_socket]
		# Get the list sockets which are readable
		ready_to_read, ready_to_write, in_error = select.select(socket_list , [], [])
		for sock in ready_to_read: 
                        if sock == client_socket: #receive
				#recvhandler(sock)
				#sys.stdout.flush()
				sys.stdout.write('>> ')
			else:	
				sys.stdout.write('>> ')	
				#typehandler()

root = Tk(  )

def chatMe(nama):
    tkMessageBox.showinfo( "chatMe", "Hello " + nama)

uname = Label(root, text='Username',borderwidth=1 ).grid(row=0,column=0)
entry1 = Entry(root, bd = 2)
entry1.grid(row=0,column=1)
passw = Label(root, text='Password',borderwidth=1 ).grid(row=1,column=0)
entry2 = Entry(root, bd = 2, show='*')
entry2.grid(row=1,column=1)


loginButton = Button(root, text ="Login", command = lambda: chatMe(entry1.get())).grid(row=2,column=1)
quitButton = Button(root, text ="Quit",command = root.quit).grid(row=2,column=2)

root.mainloop(  )
