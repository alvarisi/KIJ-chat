from Tkinter import *
import tkMessageBox
import socket
import select
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
Label(root, text='Server IP:' ).grid(row=2,column=0)
Label(root, text='Port:' ).grid(row=3,column=0)
Label(root, text='Username:' ).grid(row=4,column=0)

#entry pertama untuk konek
IP = Entry(root)
IP.grid(row=1,column=1)
port = Entry(root)
port.grid(row=2,column=1)
Nama = Entry(root)
Nama.grid(row=3,column=1)

#login
loginButton = Button(root, text ="Connect", command = lambda: login(entry1.get())).grid(row=5,column=1)

#textarea
scroll = Scrollbar(root)
scroll.grid(row=0,column=5,rowspan=11)
text = Text(root,width=45,height=20, yscrollcommand=scroll.set)
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

ipaddress = 'localhost'
port = 9999

server_address = (ipaddress, int(port))
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(server_address)
size = 1024

def typehandler():
    global STATUS
    global ID
    global IDtujuan
    global namaRoom
    global sess
    req = sys.stdin.readline()
    tmp = req.split()
    if tmp[0] == 'login':
        if checkSyntax(tmp[0], tmp):
                data = json.dumps({"REQ":tmp[0],"MESSAGE":"REQ_LOGIN", "DATA" : { "SESSION_KEY":"","CONTENT": tmp[1]+":"+tmp[2], "TYPE":"AUTH"}})
                client_socket.send(data)
    elif tmp[0] == 'register':
        if checkSyntax(tmp[0], tmp):
                data = json.dumps({"REQ":tmp[0], "MESSAGE":"REQ_REGISTER", "DATA" : {"SESSION_KEY":" ","CONTENT": tmp[1]+":"+tmp[2], "TYPE":"AUTH"}})
                client_socket.send(data)
    elif tmp[0] == 'logout':
        if checkSyntax(tmp[0], tmp):
                data = json.dumps({"REQ":tmp[0], "MESSAGE":"REQ_LOGOUT",'DATA':{"SESSION_KEY": sess,"CONTENT":" ", "TYPE":"NULL"}})
                client_socket.send(data)
    elif tmp[0] == 'create':
        if checkSyntax(tmp[0], tmp):
                data = json.dumps({"REQ":tmp[0], "MESSAGE":"REQ_CREATE", "DATA":{"SESSION_KEY":sess, "CONTENT":tmp[1]+":"+tmp[2], "TYPE":"AUTH"}})
                client_socket.send(data)
    elif tmp[0] == 'list_room':
        if checkSyntax(tmp[0], tmp):
                data = json.dumps({"REQ":tmp[0], "MESSAGE":"REQ_LIST_ROOM", "DATA":{"SESSION_KEY":sess, "CONTENT":'', "TYPE":"NULL"}})
                client_socket.send(data)
    elif tmp[0] == 'join':
        if checkSyntax(tmp[0], tmp):
                data = json.dumps({"REQ":tmp[0], "MESSAGE":"REQ_JOIN", "DATA":{"SESSION_KEY":sess, "CONTENT":tmp[1]+":"+tmp[2], "TYPE":"AUTH"}})
                client_socket.send(data)
        elif STATUS == '3':
                if tmp[0] == 'leave':
                    data = json.dumps({"REQ":'leave', "MESSAGE":"LEAVE_CHAT", "DATA":{"SESSION_KEY":sess, "CONTENT":req, "TYPE":"TEXT"}})
                    STATUS = '1'
        else:
            data = json.dumps({"REQ":'chat', "MESSAGE":"IN_CHAT", "DATA":{"SESSION_KEY":sess, "CONTENT":req, "TYPE":"TEXT"}})
            client_socket.send(data)
    else:
        print '#Protokol Anda Salah\n'# ketik HELP untuk informasi lebih lengkap\n'

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
        response = json.loads(req)
        if response ["MESSAGE"] == "IN_CHAT":
            print response["DATA"]["CONTENT"]
        else:
            os.system('cls' if os.name == 'nt' else 'clear')
            if response["MESSAGE"] == "LOGIN_FAILED" or response["MESSAGE"] == "REGISTER_FAILED" or response["MESSAGE"] == "LOGOUT_SUCCESS":
                STATUS = '0'
                sess = ''
            elif response["MESSAGE"] == "LOGIN_PERMIT":
                STATUS = '1'
                sess = response["DATA"]["SESSION_KEY"]
            elif response["MESSAGE"] == "REGISTER_PERMIT":
                STATUS = '0'
            elif response["MESSAGE"] == "CREATE_SUCCESS":
                sess = response["DATA"]["SESSION_KEY"]
                STATUS = '3'
            elif response["MESSAGE"] == "CREATE_FAILED" or response["MESSAGE"] == "JOIN_FAILED": 
                STATUS = '2'
            elif response["MESSAGE"] == "JOIN_SUCCESS":
                sess = response["DATA"]["SESSION_KEY"]
                STATUS = '3'
            elif response["MESSAGE"] == "LEAVE_SUCCESS":
                STATUS = '1'
            if response["MESSAGE"] == "LIST_ROOM":
                dashboard()
                for i in range(len(response['DATA']['CONTENT'])):
                    print response['DATA']['CONTENT'][i]
                print 'Selesai'
                print '======================================'
            else:
                dashboard()
                print message[response["MESSAGE"]]

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

try:
    while True:
        socket = [client_socket]
        recvhandler(client_socket)
        msg = sys.stdin.readline()
            #sys.stdout.flush()   
            #sys.stdout.write('>> ') 
            #typehandler()
    #thread.start_new_thread(recvhandler, ())
    #thread.start_new_thread(typehandler, ())
    
except KeyboardInterrupt:
        client_socket.close()
        sys.exit(0)            

root.mainloop()
