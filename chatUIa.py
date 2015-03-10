from Tkinter import *
import tkMessageBox

root = Tk()
root.wm_title('Welcome to chatMe!')
frame = Frame(root,width=100,height=100)

def chatMe():
    baru = Tk()
    menubar = Menu(baru)
    filemenu = Menu(menubar,tearoff=0)
    baru.wm_title('chatMe!')

    listOn = Label(baru, text='Online').grid(row=0,column=0)
    for row in range(1,5):
        on = Label(baru, text='nama').grid(row=row,column=0)

    filemenu.add_separator()
    chatbox = Label(baru, text='Chat:').grid(row=7,column=0)
    entry3 = Entry(baru,bd = 2)
    entry3.grid(row=7,column=2)
    tombol = Button(baru, text="Kirim",command=lambda: entry3.get()).grid(row=7,column=3)

    #sendMsg = Button(baru, text="Kirim", command = lambda: message()).grid(row=7,column=3)

loginButton = Button(root, text="Login", command = lambda: chatMe()).grid(row=2,columnspan=3)
#frame.bind("<Return>", chatMe('a'))
#quitButton = Button(root, text ="Quit",command = root.quit).grid(row=2,column=2)

root.mainloop(  )
