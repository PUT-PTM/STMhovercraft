# -*- coding: utf-8 -*-
from Tkinter import *
import bluetooth
#import tkMessageBox
import threading
import Queue

# ---------------------------------- Zmienne globalne -----------------------------------------------

SwitchOn = False

KOD_STERUJACY = "1200000009999911"
# [serwo * 4]	[naped_tylny * 5]	[naped_dolny * 5]	[kier_tylny * 1]	[kier_dolny * 1]
UpDownStep = 10
LeftRightStep = 10
MinUpDown = 0
MaxUpDown = 65500
MinLeftRight = 700
MaxLeftRight = 1700

WallWarning = 0

# bluetooth
ADRES_PODUSZKOWCA = "20:15:12:14:51:53"
PORT = 1
# -------------------------------------------------

class Gui():
	def __init__(self):
		self.top = Tk()
		self.top.title = "PTM Hovercraft"
		self.tekst_help = Label(self.top, text="Aplikacja do testowania poduszkowca")

		self.power_on = Button(self.top, bg="green", text="ON")#, command=uruchom)
		self.power_off = Button(self.top, bg="red", text="OFF")#, command=zamknij)

		self.w_gore = Button(self.top, bg="blue",text="+", command=lambda: self.sterowanie("Up"))
		self.w_dol = Button(self.top, bg="blue",text="-", command=lambda: self.sterowanie("Down"))
		self.w_prawo = Button(self.top, bg="blue",text="->", command=lambda: self.sterowanie("Right"))
		self.w_lewo = Button(self.top, bg="blue",text="<-", command=lambda: self.sterowanie("Left"))

		self.label_skret = Label(self.top, text="lab1",relief=RIDGE)
		self.label_wiatrak = Label(self.top,text="lab2",relief=RIDGE)
		self.label_przeszkoda = Label(self.top, text="centymetry", relief=RIDGE)

		self.tekst_help.grid(column=1,row=1,columnspan=5,ipadx=50,ipady=10, sticky="N")

		self.label_skret.grid(column=2,row=4,columnspan=2, ipadx=70,ipady=10)
		self.label_wiatrak.grid(column=2,row=3,columnspan=2, ipadx=70,ipady=10)
		self.power_on.grid(column=2,row=2,ipadx=30,ipady=10)
		self.power_off.grid(column=3,row=2,ipadx=30,ipady=10)
		self.w_gore.grid(column=0,row=2,columnspan=2,ipadx=10,ipady=30)
		self.w_dol.grid(column=0,row=4,columnspan=2,ipadx=10,ipady=30)
		self.w_prawo.grid(column=1,row=3,ipadx=30,ipady=10)
		self.w_lewo.grid(column=0,row=3,ipadx=30,ipady=10)
		self.label_przeszkoda.grid(column=2,row=5,columnspan=2, ipadx=60,ipady=10)

		self.top.bind("<Up>", self.sterowanie) # tcp.bind('<Right>', lambda event: sterowanie(event,arg1,arg2))
		self.top.bind("<Down>", self.sterowanie)
		self.top.bind("<Right>", self.sterowanie)
		self.top.bind("<Left>", self.sterowanie)

		self.update_silnik()
		self.update_serwo()
		self.top.mainloop()

	def update_serwo(self):
		self.label_skret.config(text="Skręt: " + KOD_STERUJACY[0:4])
		
	def update_silnik(self):
		self.label_wiatrak.config(text="Moc: " + KOD_STERUJACY[4:9])

	def sterowanie(self,przycisk):
		global KOD_STERUJACY

		if isinstance(przycisk,str):
			klawisz = przycisk
		else:
			klawisz = str(przycisk.keysym)

		if klawisz=="Up":
			if ( int(KOD_STERUJACY[4:9])+UpDownStep <= MaxUpDown ):
				new_value = int(KOD_STERUJACY[4:9]) + UpDownStep
				KOD_STERUJACY = KOD_STERUJACY[:4] + str(new_value) + KOD_STERUJACY[9:]


		elif klawisz=="Down":
			if ( int(KOD_STERUJACY[4:9])-UpDownStep > MinUpDown ):
				new_value = int(KOD_STERUJACY[4:9]) - UpDownStep
				KOD_STERUJACY = KOD_STERUJACY[:4] + str(new_value) + KOD_STERUJACY[9:]
				

		elif klawisz=="Right":
			if ( int(KOD_STERUJACY[0:4])+LeftRightStep <= MaxLeftRight ):
				new_value = int(KOD_STERUJACY[0:4]) + LeftRightStep
				KOD_STERUJACY = str(new_value) + KOD_STERUJACY[4:]


		elif klawisz=="Left":
			if ( int(KOD_STERUJACY[0:4])-LeftRightStep > MinLeftRight ):
				new_value = int(KOD_STERUJACY[0:4]) - LeftRightStep
				KOD_STERUJACY = str(new_value) + KOD_STERUJACY[4:]

		self.update_silnik()
		self.update_serwo()

'''def komunikacja(sock):
	global SwitchOn
	global KOD_STERUJACY
	global WallWarning
	while SwitchOn:
		sock.send(KOD_STERUJACY)
		response = sock.recv(1024)
		WallWarning = int(response)'''



# INICJALIZACJA
if __name__ == '__main__':
	top = Gui()




# bluetooth
#socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
#socket.connect((ADRES_PODUSZKOWCA, PORT))

#odbiornik = threading.Thread(target=komunikacja, name=blutacz, args=(socket,))
#odbiornik.start()


