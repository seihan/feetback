import serial
import matplotlib.pyplot as plt
import sys

def main():
	if len(sys.argv) != 2:
		print('Invalid Numbers of Arguments. Script will be terminated.')
	else:
		arg = str(sys.argv[1])
		print(arg)
	
	uart = serial.Serial(arg, 230400) # Port, Baudrate WINDOWS: 'COM1'
	#uart = serial.Serial('/dev/ttyUSB0', 230400) # Port, Baudrate WINDOWS: 'COM1'
	uart.write('r') # restart the MCU
	first = True
	x = []
	y0 = []
	y1 = []
	y2 = []
	y3 = []
	y4 = []
	y5 = []
	y6 = []
	y7 = []
	f = open('data_file.csv','w+') # Neue Datei wird erstellt
	f.write('Timestamp, AIN0, AIN1, AIN2, AIN3, AIN4, AIN5, AIN6, AIN7\n') # Kopfzeile
	f.close()
	while True:
		while(uart.read() != 'M'):
			continue;
		if (uart.read() != 'V'):
			continue;

		time1 = uart.read()
		time2 = uart.read()
		timestamp = ord(time1) + 256 * ord(time2)
		#print('Receiving {0} values'.format(length))
		received = uart.read(2*8) # 2 bytes per measurement

		if received is not None:
			if first:
				print('...receiving')
				first = False
			idx = 0
			x.append(timestamp)
			f = open('data_file.csv','a+') # Vorhandene Datei wird zum anhaengen geoeffnet
			f.write(str(timestamp))
			while idx < len(received):
				values = ord(received[idx]) + 256 * ord(received[idx + 1])
				if (idx == 0):
					y0.append(values)
				if (idx == 2):
					y1.append(values)
				if (idx == 4):
					y2.append(values)
				if (idx == 6):
					y3.append(values)
				if (idx == 8):
					y4.append(values)
				if (idx == 10):
					y5.append(values)
				if (idx == 12):
					y6.append(values)
				if (idx == 14):
					y7.append(values)
				idx += 2
				#time.append(offset)
				f.write(',') # Trennzeichen
				f.write(str(values)) # Wert
			f.write('\n') # Neue Zeile
		if (timestamp > 14242):
			plt.plot(x, y0)
			plt.plot(x, y1)
			plt.plot(x, y2)
			plt.plot(x, y3)
			plt.plot(x, y4)
			plt.plot(x, y5)
			plt.plot(x, y6)
			plt.plot(x, y7)
			plt.show()		
			f.close()
if __name__== "__main__":
	main()
