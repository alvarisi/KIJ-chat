from Tkinter import *
import tkMessageBox

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
