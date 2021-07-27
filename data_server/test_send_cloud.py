#
# Public Invention's Ventmon-Ventilator-Inline-Test-Monitor Project is an attempt
# to build a "test fixture" capable of running a 48-hour test on any ventilator
# design and collecting data on many important parameters. We hope to create a
# "gold standard" test that all DIY teams can work to; but this project will
# proceed in parallel with that. The idea is to make a standalone inline device
# plugged into the airway. It serves a dual purpose as a monitor/alarm when used
# on an actual patient, and a test device for testing prototype ventilators. It
# also allows for burnin. Copyright (C) 2021 Robert L. Read, Lauria Clarke,
# Ben Coombs, Darío Hereñú, and Geoff Mulligan.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

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
