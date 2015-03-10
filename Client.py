import socket
import select
import sys
import json
import time
import thread
import os


print "Masukkan IP Address server dan port (misal : 127.0.0.1 9999)\n"
input = sys.stdin.readline()

try:
	ipaddress, port = input.split(" ",1)
except:
	print "Input tidak valid"

server_address = (ipaddress, int(port))
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(server_address)
BUFF = 1024



ID = ''
IDtujuan = ''
STATUS = '0' # 1 = dia aktif , 2 = -, 3 = dia di room 
namaRoom = ''
sess = ''

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
#		print 'Ketik HELP untuk informasi lengkap'

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

try:
	while 1:
		socket_list = [sys.stdin, client_socket]
		# Get the list sockets which are readable
		ready_to_read, ready_to_write, in_error = select.select(socket_list , [], [])
		for sock in ready_to_read: 
			if sock == client_socket: #receive
				recvhandler(sock)
				sys.stdout.flush()
				#sys.stdout.write('>> ')
			else:	
				sys.stdout.write('>> ')	
				typehandler()

	#thread.start_new_thread(recvhandler, ())
	#thread.start_new_thread(typehandler, ())
	
except KeyboardInterrupt:
    	client_socket.close()
    	sys.exit(0)            
