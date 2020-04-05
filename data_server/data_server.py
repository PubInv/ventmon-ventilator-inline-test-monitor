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

# This makes it a rather short pattern.
# Up to 10,000 samples seems to work fine.
# Managing the continuity of time in the samples can be tedious, however.
SAMPLES = 1000
my_deque = collections.deque(maxlen=SAMPLES)

app = Flask(__name__)

CORS(app)

NUMREAD = 0;
REPORT_MODULUS = 500;
def read_from_port():
  global NUMREAD
  global my_deque
  while True:
    if(ser.inWaiting() == 0):
      pass
    line = ser.readline()   # read a '\n' terminated line
    NUMREAD = NUMREAD + 1;
    if (NUMREAD % REPORT_MODULUS) == 0:
      print(str(len(my_deque)) + " ready!\n",sys.stderr)
      print(str(line) + "\n",sys.stderr)
      print(line.decode("utf-8") + "\n",sys.stderr)
      NUMREAD = 0;
    my_deque.append(line.decode("utf-8"));


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
  result = get_n_samples(n);
  return result

if __name__ == "__main__":
  app.run(host='127.0.0.1', port=80, debug=True)
