from Tkinter import *
import tkMessageBox
import socket
import sys
import json
import time
import thread
import os
import ttk

root = Tk()
root.wm_title('Welcome to chatMe!')
frame = Frame(root)
BUFF=1024

#Label paling kiri
Label(root, text='Status:' ).grid(row=1,column=0)
Label(root, text='Username:' ).grid(row=2,column=0)
Label(root, text='Password:' ).grid(row=3,column=0)
#entry pertama untuk konek
Status = Entry(root)
Status.grid(row=1,column=1)
username = Entry(root)
username.grid(row=2,column=1)
password = Entry(root)
password.grid(row=3,column=1)

ipaddress = 'localhost'
port = 9999
server_address = (ipaddress, int(port))
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(server_address)
size = 1024
req = client_socket.recv(BUFF)
print req

def register(u, p):
    data = "REQ:REGISTER:"+u+":"+p+":!>"
    print data
    client_socket.send(data)
    req = client_socket.recv(BUFF)
    #resp = 

def login(u, p):
    data = "REQ:LOGIN:"+u+":"+p+":!>"
    print data
    client_socket.send(data)
    req = client_socket.recv(BUFF)
    print req

#login
loginButton = Button(root, text ="Login", command = lambda: login(username.get(),password.get())).grid(row=5,column=1)
registerButton = Button(root, text ="Daftar", command = lambda: register(username.get(),password.get())).grid(row=6,column=1)

#textarea
scroll = Scrollbar(root)
scroll.grid(row=0,column=5,rowspan=11)
text = Text(root,width=45,height=20)
text.grid(row=1,column=2,rowspan=21,columnspan=4)
scroll.config(command=text.yview)
text.config(yscrollcommand=scroll.set,state=DISABLED)
scroll.config(command=text.yview)

text.insert(INSERT,"aa")

#message
ab = Label(root,width=25)
ab.grid(row=22,column=3)
message = Entry(root,width=35)
message.grid(row=30, column=4)
SendButton = Button(root, text="Kirim",command= lambda:kirim(message.get())).grid(row=30,column=5)

#online

ol = Label(root,text="Who's Online")
ol.grid(row=0,column=6)
scroll2 = Scrollbar(root)
scroll2.grid(row=0,column=8,rowspan=11)
listnya = Listbox(root,height=10,width=25,yscrollcommand=scroll2.set)
listnya.grid(row=1,column=6,rowspan=11)
scroll2.config(command=listnya.yview)
#boh = Label(root)
#boh.grid(row=5,column=1)
#frame.bind("<Return>", chatMe('a'))
#quitButton = Button(root, text ="Quit",command = root.quit).grid(row=2,column=2)


ID = ''
IDtujuan = ''
STATUS = '0' # 1 = dia aktif , 2 = -, 3 = dia di room 
namaRoom = ''
sess = ''

def typehandler():
    global STATUS
    global ID
    global IDtujuan
    global namaRoom
    global sess
    req = sys.stdin.readline()
    client_socket.send(data)

def dashboard() :
    global STATUS
    print '================================================='
    if STATUS == '1':
        print 'Ketik create [roomname] [password] untuk membuat Room'
        print 'Ketik list_room untuk melihat room aktif'
        print 'Ketik join [roomname] [password] untuk masuk ke room aktif'
        print 'Ketik logout untuk keluar'
    elif STATUS == '3':
        print 'Ketik leave untuk keluar dari ROOM'
    elif STATUS == '0':
        print 'Selamat Datang'
        print 'REGISTER = ketik register [USERNAME] [PASSWORD]'
        print 'LOGIN = ketik login [USERNAME] [PASSWORD]'
#       print 'Ketik HELP untuk informasi lengkap'

    print '================================================='
    return True

def recvhandler(sock):
    #while True:
        global STATUS
        global ID
        global IDtujuan
        global namaRoom
        global sess
        req = sock.recv(BUFF)
        print req
        return

message = { "CONNECT_OK" : "*Koneksi berhasil!*",
            "LOGIN_PERMIT" : "*Login Berhasil!*",
            "LOGIN_FAILED" : "*Username atau password salah, coba lagi!*",
            "REGISTER_FAILED" : "*Username telah dipakai, coba lagi!*",
            "REGISTER_PERMIT" : "*Pendaftaran berhasil!*",
            "CREATE_SUCCESS"  : "*Room berhasil dibuat!*",
            "CREATE_FAILED"   : "*Nama room sama, coba nama yang lain!*",
            "LOGOUT_SUCCESS"  : "*Anda berhasil logout!*",
            "LIST_ROOM"       : "*Daftar room*",
            "JOIN_SUCCESS"    : "*Join room berhasil!*",
            "JOIN_FAILED"     : "*Join gagal, coba lagi!*",
            "LEAVE_SUCCESS"   : "*Leave room berhasil*",
            "SESSION_FAILED"  : "*User telah login*",
            "IN_CHAT"         : " "
}

def checkSyntax(types, tmp):
    if types =='login' and len(tmp) == 3:
        return True
    elif types == 'register' and len(tmp) == 3:
        return True
    elif types == 'create' and len(tmp) == 3:
        return True
    elif types == 'list_room' and len(tmp) == 1:
        return True
    elif types == 'join' and len(tmp) == 3:
        return True
    elif types == 'logout' and len(tmp) == 1:
        return True
    print '#Terjadi kesalahan syntax\n'# ketik HELP untuk informasi lebih lanjut\n'
    return False

def task():
    global client_socket
    
    #socket_list = [sys.stdin, client_socket]
       # Get the list sockets which are readable
    #ready_to_read, ready_to_write, in_error = select.select(socket_list , [], [])
    #for sock in ready_to_read: 
    #    if sock == client_socket: #recei
    #        recvhandler(sock)
    print "aaa"
    root.after(2000,task)  # reschedule event in 2 seconds
    
    
try:
    root.after(2000,task)
    root.mainloop()
    
except KeyboardInterrupt:
        client_socket.close()
        sys.exit(0)            


