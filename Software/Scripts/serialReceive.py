import serial
import time
import os

serialPort = serial.Serial(port = "COM4", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
serialPort.flushInput()

serialString = ""                           # Used to hold data coming over UART
file = "enc_comp.txt"
#compileCMD = 'gcc rsa_decrypted.c' 
#runCMD = 'a.exe d test.txt out.txt'

while(1):
    # Wait until there is data waiting in the serial buffer
    if(serialPort.in_waiting > 0):
            # Read data out of the buffer until a carraige return / new line is found
        serialString = serialPort.readline()
            # Print the contents of the serial data
        with open(file,"ab") as f:
            f.write(serialString)
            print(serialString.decode('Ascii'))
            f.close()
       # os.system(compileCMD)
       # os.system(runCMD)
       # print("complete")