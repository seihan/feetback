# Example of interaction with a BLE FEETBACK device using a FEETBACK service
# implementation.
# Author: Tony DiCola
import Adafruit_BluefruitLE
from Adafruit_BluefruitLE.services import FEETBACK
import struct

import socket
from threading import Timer

# UDP server conf
UDP_IP = "127.0.0.1"
UDP_PORT = 5005

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT

sock = socket.socket(socket.AF_INET, # Internet
                      socket.SOCK_DGRAM) # UDP

packets = 0
pktsize = 0

def pps():
    Timer(1, pps).start()
    global packets
    global pktsize
    print('Received {} pps ({} avg. bytes per pkt)'.format(packets, pktsize / max(packets, 1)))
    packets = 0
    pktsize = 0


# Get the BLE provider for the current platform.
ble = Adafruit_BluefruitLE.get_provider()


# Main function implements the program logic so it can run in a background
# thread.  Most platforms require the main thread to handle GUI events and other
# asyncronous events like BLE actions.  All of the threading logic is taken care
# of automatically though and you just need to provide a main function that uses
# the BLE provider.
def main():
    # Clear any cached data because both bluez and CoreBluetooth have issues with
    # caching data and it going stale.
    ble.clear_cached_data()

    # Get the first available BLE network adapter and make sure it's powered on.
    adapter = ble.get_default_adapter()
    adapter.power_on()
    print('Using adapter: {0}'.format(adapter.name))

    # Disconnect any currently connected FEETBACK devices.  Good for cleaning up and
    # starting from a fresh state.
    print('Disconnecting any connected FEETBACK devices...')
    FEETBACK.disconnect_devices()

    # Scan for FEETBACK devices.
    print('Searching for FEETBACK device...')
    try:
        adapter.start_scan()
        # Search for the first FEETBACK device found (will time out after 60 seconds
        # but you can specify an optional timeout_sec parameter to change it).
        device = FEETBACK.find_device()
        if device is None:
            raise RuntimeError('Failed to find FEETBACK device!')
    finally:
        # Make sure scanning is stopped before exiting.
        adapter.stop_scan()

    print('Connecting to device...')
    device.connect()  # Will time out after 60 seconds, specify timeout_sec parameter
                      # to change the timeout.


    global packets
    global pktsize
    Timer(1, pps).start()

    # Once connected do everything else in a try/finally to make sure the device
    # is disconnected when done.
    try:
        # Wait for service discovery to complete for the FEETBACK service.  Will
        # time out after 60 seconds (specify timeout_sec parameter to override).
        print('Discovering services...')
        FEETBACK.discover(device)

        # Once service discovery is complete create an instance of the service
        # and start interacting with it.
        feetback = FEETBACK(device)

        # Write a string to the TX characteristic.
        #feetback.write(b'Hello world!\r\n')
        #print("Sent 'Hello world!' to the device.")

        # Now wait up to one minute to receive data from the device.
        print('Waiting up to 60 seconds to receive data from the device...')
        
        while True:
            received = feetback.read(timeout_sec=60)
            if received is not None:
                # Received data, print it out.
                packets = packets + 1
                pktsize = pktsize + len(received)
                sock.sendto('MV{}{}'.format(struct.pack('H', len(received) / 4), received), (UDP_IP, UDP_PORT))
            else:
                # Timeout waiting for data, None is returned.
                print('Received no data!')
    finally:
        # Make sure device is disconnected on exit.
        device.disconnect()


# Initialize the BLE system.  MUST be called before other BLE calls!
ble.initialize()

# Start the mainloop to process BLE events, and run the provided function in
# a background thread.  When the provided main function stops running, returns
# an integer status code, or throws an error the program will exit.
ble.run_mainloop_with(main)
