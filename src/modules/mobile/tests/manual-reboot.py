from base64 import b64decode
import sys
import time

import serial

def send(d, s):
    print("host:  ", s)
    return d.write(s)

def recv(d, n):
    r = d.read(n)
    print("board: ", r)
    return r

def reboot(d):
    send(d, "\x05HS")
    r = recv(d, 6)
    if r != "\x06\06HS\x01\x00":
	return 1

    send(d, "\x05VH")
    r = recv(d, 8)

    send(d, "\x05VF")
    r = recv(d, 6)
    if r[:4] != "\x06\06VF":
	return 1

    send(d, "\x05IS")
    r = recv(d, 28)
    if r[:4] != "\x06\06IS":
	return 1

    send(d, "\x05rt")
    r = recv(d, 4)
    if r != "\x06\06rt":
	return 1

    return 0

if __name__ == '__main__':
    port = sys.argv[1] if len(sys.argv) > 1 else '/dev/tty.usbmodem1'
    print("Port: %r" % port)

    d = serial.Serial(port, timeout=1)
    d.flushInput()
    d.flushOutput()

    sys.exit(reboot(d))

# vim: set et sts=4 sw=4 :
