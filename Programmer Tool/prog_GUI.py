''' Simple GUI program that uses updi-program python script to program AVR DA family chips
    This GUI is written By Masoud Babaabasi
'''

import sys
import subprocess
from datetime import datetime
from tkinter import *
from tkinter.filedialog import askopenfilename


class Redirect():
    
    def __init__(self, widget):
        self.widget = widget

    def write(self, text):
        self.widget.insert('end', text)
        self.widget.see('end') # autoscroll

def start_prog(a , b , c , d , f ):
    action = a
    baud = b
    com = c
    device = d
    fileName = f
    #text.delete(1.0 . END)
    if( action != 'erase'):
         arg = '-a ' + action +' -b ' + baud + ' -d ' + device + ' --fuses 5:0xc9 6:0x04 7:0x00 8:0x00 2:0x00 -f ' + fileName + ' -u UART --uart ' +  com
    else:
        arg = '-a ' + action +' -b ' + baud + ' -d ' + device + ' --fuses 5:0xc9 6:0x04 7:0x00 8:0x00 2:0x00 -u UART --uart ' +  com
    command = 'python prog.py ' + arg
    proc = subprocess.Popen(command , stdout=subprocess.PIPE , universal_newlines=1)
    now = datetime.now()
    (out, err) = proc.communicate()
    print("start programming at: ", now.strftime("-- %Y/%m/%d -- %H:%M:%S") , "------------------------\n")
    print(out)
def browse_file():
    browse_filename = askopenfilename()
    fileName_entry.delete(0,END)
    fileName_entry.insert(0,browse_filename)

if __name__ == "__main__":
    master = Tk()
    master.title("AVR DA family programming tool")
    Label(master, text='Action').grid(row=0)
    Label(master, text='Baudrate').grid(row=1)
    Label(master, text='COM Port').grid(row=2)
    Label(master, text='Device').grid(row=3)
    Label(master, text='File path').grid(row=4)

    action_str = StringVar(master, value='write')
    action_entry = OptionMenu(master, action_str, "write", "read", "erase")

    bauds = ["9600", "19200", "38400" , "57600" , "115200" , "230400" , "460800" , "500000" ,"921600"]
    baud_str = StringVar(master, value='115200')
    baud_entry = OptionMenu(master , baud_str , *bauds)

    com_str = StringVar(master, value='COM6')
    coms = ['COM' + str(i) for i in range(1,25)]
    com_entry = OptionMenu(master , com_str , *coms)

    devices = ["avr32da28" , "avr32da32" , "avr32da48" , "avr32da64" ,"avr64da28" , "avr64da32" , "avr64da48" , "avr64da64"  ,"avr128da28" , "avr128da32" , "avr128da48" , "avr128da64" ]
    device_str = StringVar(master, value='avr32da48')
    device_entry = OptionMenu(master , device_str , *devices)

    fileName_str = StringVar(master, value='')
    fileName_entry = Entry(master , width=100 , textvariable = fileName_str)

    open_but = Button(master, text='...', command = lambda :browse_file())
    open_but.grid( row=4 , column=2 )
    action_entry.grid(row=0, column=1)
    baud_entry.grid(row=1, column=1)
    com_entry.grid(row=2, column=1)
    device_entry.grid(row=3, column=1)
    fileName_entry.grid(row=4, column=1)

    scroll_bar = Scrollbar(master)
    scroll_bar.grid(row=6 , column=2 )
    text = Text(master)
    text.grid(row=6,column=0, columnspan= 2)
    scroll_bar.config( command = text.yview )
    prog_but = Button(master, text='Start', width=25, command= lambda:start_prog(action_str.get() , baud_str.get() , com_str.get() , device_str.get() , fileName_str.get()))
    prog_but.grid(row = 5 , column = 0 , columnspan= 2)

    old_stdout = sys.stdout    
    sys.stdout = Redirect(text)
    mainloop()

    sys.stdout = old_stdout





