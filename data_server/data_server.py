#!/usr/bin/python
import time
import serial
# ser = serial.Serial('/dev/cu.usbmodem141401',57600,timeout=None)  # open serial port
ser = serial.Serial('/dev/cu.SLAB_USBtoUART',115200,timeout=None)
import time
import math
from time import sleep
from flask import Flask
from flask import request
import sys
from flask_cors import CORS
import datetime

import collections
import itertools
import threading
import json

import socket
import sys
import os

# This makes it a rather short pattern.
# Up to 10,000 samples seems to work fine.
# Managing the continuity of time in the samples can be tedious, however.
SAMPLES = 3000
my_deque = collections.deque(maxlen=SAMPLES)

app = Flask(__name__)

CORS(app)

DATA_LAKE_HOST = "ventmon.coslabs.com"
DATA_LAKE_PORT = 6110
DATA_LAKE_SAMPLES_TO_SEND = 1000
NUM_SENT_TO_DATA_LAKE = 0

NUMREAD = 0;
REPORT_MODULUS = 500;

RUN_LIMIT = 1004;
RUN_CNT = 0;


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

def read_from_port():
  global NUMREAD
  global my_deque
  global NUM_SENT_TO_DATA_LAKE
  global DATA_LAKE_SAMPLES_TO_SEND
  global sock
  global RUN_CNT
  global RUN_LINIT
  while True :
    if (RUN_CNT >= RUN_LIMIT):
      os._exit(os.EX_OK)
    RUN_CNT = RUN_CNT + 1
    if(ser.inWaiting() <= 50):
      pass
    line = ser.readline()   # read a '\n' terminated line
    NUMREAD = NUMREAD + 1;
    if (NUMREAD % REPORT_MODULUS) == 0:
      print(str(len(my_deque)) + " ready!\n",sys.stderr)
      print(str(line) + "\n",sys.stderr)

      NUMREAD = 0;
    # performing a try except here really seems to
    # make the coding of characters all messed up,
    # I don't realy know why, it is like we get off by one character
    try:
      thisline = line.decode("utf-8")+"\n"
      # For bizarre reason I'm unable to figure out,
      # printing this here is NECESSARY!!!
      print(thisline,sys.stderr)
      my_deque.append(thisline)

      if  NUM_SENT_TO_DATA_LAKE < DATA_LAKE_SAMPLES_TO_SEND:
        print(f'sending {NUM_SENT_TO_DATA_LAKE} {DATA_LAKE_SAMPLES_TO_SEND}',
              sys.stderr)
        try:
          m = json.loads(thisline);
          if m["event"] == "M":
            minst = Measurement(m["type"], m["loc"], int(m["num"]),
                                m["ms"], m["val"])
            print('MARK', sys.stderr)
            print(m["type"]+"\n", sys.stderr)
            print(m["loc"]+"\n", sys.stderr)
            print(m["num"]+"\n", sys.stderr)
            print(str(m["ms"])+"\n", sys.stderr)
            print(str(m["val"])+"\n", sys.stderr)
            try:
              # Send data
              bytes = minst.asBytes()
              print('MARK 2', sys.stderr)
              print('sending "%s"' % str(bytes), sys.stderr)
              return_value = sock.sendall(bytes)
              print('sendall returned' + str(return_value), sys.stderr)
              NUM_SENT_TO_DATA_LAKE = NUM_SENT_TO_DATA_LAKE+1
              if NUM_SENT_TO_DATA_LAKE >= DATA_LAKE_SAMPLES_TO_SEND:
                sock.close()
                os._exit(os.EX_OK)
            except:
              print("Unexpected error:", sys.exc_info()[0],sys.stderr)
              print('AAAAAAAAAAAAAA',sys.stderr)
        except:
          print('json error',sys.stderr)
    except(UnicodeDecodeError):
      print("ERRRRROR\n",sys.stderr)



thread = threading.Thread(target=read_from_port)
thread.start()


def get_n_samples(n):
  global my_deque
  n = int(request.args.get('n'))
  result = ""
  print("n" + str(n)+"\n",sys.stderr)
  print(str(len(my_deque))+"ready \n",sys.stderr)
  for i in range(0,min(n,len(my_deque))):
    try:
      line = my_deque.popleft()
      print(str(line) + "\n",sys.stderr)
      result = result + line
    except IndexError:
      print("Indexerror\n",sys.stderr)
      break;
  print("result = " + result + "\n",sys.stderr)
  return result

@app.route("/")
def getsamples():
  global my_deque
  result = ""
  print("len" + str(len(my_deque))+"\n",sys.stderr)
  for x in my_deque:
    line = x
    result = result + line
  return result

# Get at most the earliest n samples.
@app.route("/s")
def get_limit_samples():
  n = int(request.args.get('n'))
  print("len" + str(n) + " of " + str(len(my_deque))+"\n",sys.stderr)
  result = get_n_samples(n);
  return result

if __name__ == "__main__":
  print(f"Arguments count: {len(sys.argv)}")
  print(f"Name of the script      : {sys.argv[0]}")
  print(f"Arguments of the script : {sys.argv[1:]}")
  if len(sys.argv) > 1:
    DATA_LAKE_HOST = sys.argv[1]
  if len(sys.argv) > 2:
    DATA_LAKE_PORT = sys.argv[2]
  if len(sys.argv) > 3:
    DATA_LAKE_SAMPLES_TO_SEND = sys.argv[3]
  server_address = (DATA_LAKE_HOST, int(DATA_LAKE_PORT))
  sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  sock.connect(server_address)
  app.run(host='127.0.0.1', port=80, debug=True)
