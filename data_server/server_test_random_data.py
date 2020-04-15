#! /usr/bin/env python
#################################################################################
#     File Name           :     server_test_random_data.py
#     Created By          :     lauriaclarke
#     Creation Date       :     [2020-04-15 13:39]
#     Last Modified       :     [2020-04-15 13:40]
#     Description         :
#################################################################################

import time
import struct
import random
import time
import math
from time import sleep
import sys
import datetime

import socket
import sys
import os
import traceback

# This makes it a rather short pattern.
# Up to 10,000 samples seems to work fine.
# Managing the continuity of time in the samples can be tedious, however.
SAMPLES = 3000

DATA_LAKE_HOST = "ventmon.coslabs.com"
DATA_LAKE_PORT = 6110
DATA_LAKE_SAMPLES_TO_SEND = 0
NUM_SENT_TO_DATA_LAKE = 0

NUMREAD = 0;
REPORT_MODULUS = 500;

RUN_LIMIT = 0;
RUN_CNT = 0;


class Measurement:
  def __init__(self, measurementType, deviceType, deviceLocation, measurementTime, measurementValue):
      self.m                = "M"
      self.measurementType  = measurementType
      self.deviceType       = deviceType
      self.deviceLocation   = deviceLocation
      self.measurementTime  = measurementTime
      self.measurementValue = measurementValue


  def asBytes(self):

      b = bytearray(12)

      b[0] = ord(self.m)
      b[1] = ord(self.measurementType)
      b[2] = ord(self.deviceType)
      b[3] = ord(chr(self.deviceLocation))

      x = self.measurementTime.to_bytes(4, 'big')
      b[4] = x[0];
      b[5] = x[1];
      b[6] = x[2];
      b[7] = x[3];

      y = self.measurementValue.to_bytes(4, 'big', signed=True)
      b[8]  = y[0];
      b[9]  = y[1];
      b[10] = y[2];
      b[11] = y[3];


      # debugging
      # test we can convert to a signed int
      #p = int.from_bytes(b'\xff\xff\xff\xff', byteorder='big', signed=True)
      #print("%d" %p)

      # print all bytes in b
      #for x in b:
        #print("%X" %x)

      return b

  def printMeasurement(self):
      print("measurement to send: ", self.m, self.measurementType, self.deviceType, self.deviceLocation, self.measurementTime, self.measurementValue)



def terminate_with_newline(mbytes):
      b = bytearray(len(mbytes)+1)
      for i in range(len(mbytes)):
        b[i] = mbytes[i]
      b[12] = ord('\n')
      return b;


# pass socket from main
def read_from_port(sock):
  global NUMREAD
  global my_deque
  global NUM_SENT_TO_DATA_LAKE
  global DATA_LAKE_SAMPLES_TO_SEND
  global RUN_CNT
  global RUN_LINIT

  while True :
    if ((RUN_LIMIT > 0) and RUN_CNT >= RUN_LIMIT):
      os._exit(os.EX_OK)
    RUN_CNT = RUN_CNT + 1

    # generate randomized values and types
    measurementValue = random.randrange(-10, 10, 1)
    measurementTime = int(round(time.time() * 1000)) - startTime
    deviceLocation = random.randrange(0, 10, 1)
    deviceType = random.choice(['B', 'A', 'M', 'D'])
    measurementType = random.choice(['T', 'P', 'D', 'F', 'O', 'H', 'V', 'B', 'G', 'A'])
    # assign them ot measurement
    m = Measurement(measurementType, deviceType, deviceLocation, measurementTime, measurementValue)

    if  ((DATA_LAKE_SAMPLES_TO_SEND > 0) and (NUM_SENT_TO_DATA_LAKE < DATA_LAKE_SAMPLES_TO_SEND)):
      print(f'sending {NUM_SENT_TO_DATA_LAKE} / {DATA_LAKE_SAMPLES_TO_SEND}')

      try:
        mbytes = m.asBytes()
        bytes = terminate_with_newline(mbytes)
        m.printMeasurement()
        print('sending "%s" \n' % str(bytes))

        sock.sendall(bytes)

        NUM_SENT_TO_DATA_LAKE = NUM_SENT_TO_DATA_LAKE + 1

        if NUM_SENT_TO_DATA_LAKE >= DATA_LAKE_SAMPLES_TO_SEND:
          print("\nclosing socket")
          print(f'sent: {NUM_SENT_TO_DATA_LAKE} / {DATA_LAKE_SAMPLES_TO_SEND}')
          sock.close()
          os._exit(os.EX_OK)

      except OverflowError:
        print("Overflow error:", sys.exc_info()[0])
        traceback.print_exc(file=sys.stdout)
        os._exit(os.EX_OK)
      except BrokenPipeError:
        traceback.print_exc(file=sys.stdout)
        os._exit(os.EX_OK)



if __name__ == "__main__":
  startTime = int(round(time.time() * 1000))
  print(f"Arguments count: {len(sys.argv)}")
  print(f"Name of the script      : {sys.argv[0]}")
  print(f"Arguments of the script : {sys.argv[1:]}\n")
  if len(sys.argv) > 1:
    DATA_LAKE_HOST = sys.argv[1]
  if len(sys.argv) > 2:
    DATA_LAKE_PORT = sys.argv[2]
  if len(sys.argv) > 3:
    DATA_LAKE_SAMPLES_TO_SEND = int(sys.argv[3])
  server_address = (DATA_LAKE_HOST, int(DATA_LAKE_PORT))
  try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(server_address)
  except (ConnectionRefusedError):
   print(f"ConnectionRefuesd{sys.exc_info()[0]}")


thread = threading.Thread(target=read_from_port(sock))
thread.start()
