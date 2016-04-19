# -*- coding: utf-8 -*-
import Tkinter
import bluetooth
#import tkMessageBox
import threading
import Queue
import time
import sys

# ZMIENNE GLOBALNE

KOD_STERUJACY = "1200000006550011"
# [serwo * 4]	[naped_tylny * 5]	[naped_dolny * 5]	[kier_tylny * 1]	[kier_dolny * 1]
WallWarning = "0"
ZAMEK = threading.Lock()
SwitchOff = True
# -------------------------------------------------

class NiebieskiZab():
	def __init__(self):
		self.ADRES_PODUSZKOWCA = "20:15:12:14:51:53"
		self.PORT = 1

		#self.socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM) # utworzenie gniazda
		#self.socket.connect((ADRES_PODUSZKOWCA, PORT))

	def komunikacja(self):
		global KOD_STERUJACY
		global ZAMEK
		global WallWarning
		global SwitchOff

		while SwitchOff:
			ZAMEK.acquire()
			message = KOD_STERUJACY
			ZAMEK.release()
			self.socket.send(message)
			response = self.socket.recv(1024)
			WallWarning = str(response)
			time.sleep(0.5)

	def test(self):
		global WallWarning
		global SwitchOff
		global KOD_STERUJACY
		for i in range(100,110):
			if not SwitchOff:
				KOD_STERUJACY = "1200000000000011"
				return
			WallWarning = str(i)
			time.sleep(1)

	def __del__(self):
		print("zakończono blutacza")
		#self.socket.close()

class Gui():
	def __init__(self):

		# ZMIENNE

		self.UpDownStep = 500
		self.LeftRightStep = 50
		self.MinUpDown = 0
		self.MaxUpDown = 65500
		self.MinLeftRight = 700
		self.MaxLeftRight = 1700

		# --------------------------------------------------------------------------------------------

		self.top = Tkinter.Tk()
		self.top.title("STM Hovercraft")
		self.tekst_help = Tkinter.Label(self.top, text="Aplikacja do testowania poduszkowca")

		self.power_on = Tkinter.Button(self.top, bg="green", text="ON", command=self.uruchom)
		self.power_off = Tkinter.Button(self.top, bg="red", text="OFF", command=self.zamknij)

		self.w_gore = Tkinter.Button(self.top, bg="blue",text="+", command=lambda: self.sterowanie("Up"))
		self.w_dol = Tkinter.Button(self.top, bg="blue",text="-", command=lambda: self.sterowanie("Down"))
		self.w_prawo = Tkinter.Button(self.top, bg="blue",text="->", command=lambda: self.sterowanie("Right"))
		self.w_lewo = Tkinter.Button(self.top, bg="blue",text="<-", command=lambda: self.sterowanie("Left"))

		self.label_skret = Tkinter.Label(self.top, text="lab1",relief=Tkinter.RIDGE)
		self.label_wiatrak = Tkinter.Label(self.top,text="lab2",relief=Tkinter.RIDGE)
		self.label_przeszkoda = Tkinter.Label(self.top, text="centymetry", relief=Tkinter.RIDGE)

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

		self.top.bind("<Up>", self.sterowanie) # nacisniety klawisz jest podawany niejawnie jako argument metody sterowanie
		self.top.bind("<Down>", self.sterowanie)
		self.top.bind("<Right>", self.sterowanie)
		self.top.bind("<Left>", self.sterowanie)

		self.update_silnik()
		self.update_serwo()

		#self.top.protocol("WM_DELETE_WINDOW", self.quit_close)
		self.refresh() # odswieza wartosci gdy nic nie jest przesylane przez Bluetooth
		self.top.mainloop()

	def quit_close(self):
		sys.exit(0)

	def uruchom(self):
		global SwitchOff
		SwitchOff = True
		self.blutacz = NiebieskiZab()
		self.watek_blutacza = threading.Thread(target=self.blutacz.test)
		self.watek_blutacza.start()

	def zamknij(self):
		global SwitchOff
		SwitchOff = False

	def refresh(self):
		self.update_silnik()
		self.update_serwo()
		self.update_WallWarning()
		self.top.after(100,self.refresh)

	def update_serwo(self):
		global KOD_STERUJACY
		self.label_skret.config(text="Skręt: " + KOD_STERUJACY[0:4])
		
	def update_silnik(self):
		global KOD_STERUJACY
		self.label_wiatrak.config(text="Moc: " + KOD_STERUJACY[4:9])

	def update_WallWarning(self):
		global WallWarning
		self.label_przeszkoda.config(text="Do przeszkody: " + WallWarning + " cm")

	def sterowanie(self,przycisk):
		global KOD_STERUJACY
		if isinstance(przycisk,str):
			klawisz = przycisk
		else:
			klawisz = str(przycisk.keysym)

		if klawisz=="Up":
			if ( int(KOD_STERUJACY[4:9])+self.UpDownStep <= self.MaxUpDown ):
				new_value = int(KOD_STERUJACY[4:9]) + self.UpDownStep
				uzupelnij = 5 - len(str(new_value)) # uzupelnienie zerami zmiennej KOD_STERUJACY
				KOD_STERUJACY = KOD_STERUJACY[:4] + uzupelnij * "0" + str(new_value) + KOD_STERUJACY[9:]


		elif klawisz=="Down":
			if ( int(KOD_STERUJACY[4:9])-self.UpDownStep >= self.MinUpDown ):
				new_value = int(KOD_STERUJACY[4:9]) - self.UpDownStep
				uzupelnij = 5 - len(str(new_value))
				KOD_STERUJACY = KOD_STERUJACY[:4] + uzupelnij * "0" + str(new_value) + KOD_STERUJACY[9:]
				

		elif klawisz=="Right":
			if ( int(KOD_STERUJACY[0:4])+self.LeftRightStep <= self.MaxLeftRight ):
				new_value = int(KOD_STERUJACY[0:4]) + self.LeftRightStep
				uzupelnij = 4 - len(str(new_value))
				KOD_STERUJACY = uzupelnij * "0" + str(new_value) + KOD_STERUJACY[4:]


		elif klawisz=="Left":
			if ( int(KOD_STERUJACY[0:4])-self.LeftRightStep >= self.MinLeftRight ):
				new_value = int(KOD_STERUJACY[0:4]) - self.LeftRightStep
				uzupelnij = 4 - len(str(new_value))
				KOD_STERUJACY = uzupelnij * "0" + str(new_value) + KOD_STERUJACY[4:]

		self.update_silnik()
		self.update_serwo()




# INICJALIZACJA
if __name__ == '__main__':
	top = Gui()
	print "siemka"



