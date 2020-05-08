#! /usr/bin/env python

import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# VENTMON_SERVER_PORT = 6110 # this one is refused!!
VENTMON_SERVER_PORT = 57573 # this one times out
VENTMON_HOST = 'ventmon.coslabs.com'
# VENTMON_HOST = 'localhost'
# VENTMON_SERVER_PORT = 80
# VENTMON_HOST = 'www.python.org'
server_address = (VENTMON_HOST, VENTMON_SERVER_PORT)

class Measurement:
  def __init__(self, measurementType, deviceType, deviceLocation, measurementTime, measurementValue):
      x = "local"
      self.m                = "M"
      self.measurementType  = measurementType
      self.deviceType       = deviceType
      self.deviceLocation   = deviceLocation
      self.measurementTime  = measurementTime
      self.measurementValue = measurementValue
  def asBytes(self):
      self.x = 4
      b = bytearray(12)
      b[0] = ord(self.m)
      b[1] = ord(self.measurementType)
      b[2] = ord(self.deviceType)
      x = self.measurementTime.to_bytes(4, 'big')
      b[4] = x[0];
      b[5] = x[1];
      b[6] = x[2];
      b[7] = x[3];
      v = self.measurementValue.to_bytes(4, 'big')
      b[8] = v[0];
      b[9] = v[1];
      b[10] = v[2];
      b[11] = v[3];
#      b[3] = self.deviceLocation.to_bytes(1,'big')
      b[3] = ord(chr(self.deviceLocation))
      return b

p1 = Measurement("T", "B", 25, 123456789, 1234567)

try:
    # Send data
    bytes = p1.asBytes()
    print('sending "%s"' % str(p1.deviceLocation.to_bytes(1,'big')), sys.stderr)
    print('sending "%s"' % str(bytes), sys.stderr)

    sock.connect(server_address)
    return_value = sock.sendall(bytes)
#   return_value = sock.sendall("spudboy\n".encode())
    print('sendall returned' + str(return_value), sys.stderr)
    sock.close()
    print('now seeking response ', sys.stderr)
    # Look for the response
    # amount_received = 0
    # amount_expected = 10
    # while amount_received < amount_expected:
    #     data = sock.recv(1)
    #     if (len(data) == 0):
    #       break
    #     amount_received += len(data)
    #     print('received "%s"' % data,sys.stderr)

finally:
    print('closing socket',sys.stderr)
    sock.close()
