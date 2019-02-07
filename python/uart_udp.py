import socket
import serial
from threading import Timer

# UDP server conf
UDP_IP = "127.0.0.1"
UDP_PORT = 5005

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT

sock = socket.socket(socket.AF_INET, # Internet
                      socket.SOCK_DGRAM) # UDP

packets = 0

def pps():
    Timer(1, pps).start()
    global packets
    print('Received {} pps'.format(packets))
    packets = 0

def main():
    global packets
    Timer(1, pps).start()

    uart = serial.Serial('/dev/ttyUSB3', 230400)
    first = True

    while True:
        while(uart.read() != 'M'):
            continue;
        if (uart.read() != 'V'):
            continue;

        len1 = uart.read()
        len2 = uart.read()
        length = ord(len1) + 256 * ord(len2)
        #print('Receiving {0} values'.format(length))
        received = uart.read(2*length)

        if received is not None:
            if first:
                print('...receiving')
                first = False

            # Received data, print it out.
            packets = packets + 1
            sock.sendto('MV{}{}{}'.format(len1, len2, received), (UDP_IP, UDP_PORT))
            # print('Received {0} bytes'.format(len(received)))
            # print(' '.join(received[i] for i in range(len(received))))
            #            for x in range(0, 30):
            #                s.send(received)


if __name__ == '__main__':
    main()
