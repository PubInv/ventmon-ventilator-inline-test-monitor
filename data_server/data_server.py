#!/usr/bin/python
import time
import serial
ser = serial.Serial('/dev/cu.usbmodem141401',115200,timeout=None)  # open serial port
import time
import math
from time import sleep
from flask import Flask
from flask import request
import sys
from flask_cors import CORS
import datetime

import collections
import threading

# This makes it a rather short pattern.
# Up to 10,000 samples seems to work fine.
# Managing the continuity of time in the samples can be tedious, however.
SAMPLES = 1000
d = collections.deque(maxlen=SAMPLES)

app = Flask(__name__)

CORS(app)

def read_from_port():
  while True:
    line = ser.readline()   # read a '\n' terminated line
    print(str(line)+"\n",sys.stderr)
    d.append(line);



thread = threading.Thread(target=read_from_port)
thread.start()


# @app.route("/rec")
# def recordsamples():
#   num = min(SAMPLES,int(request.query_string))
#   print("in_waiting"+str(ser.in_waiting)+"\n",sys.stderr)
#   ser.reset_input_buffer()

#   sleep(.1)
#   print("after flush"+str(ser.in_waiting)+"\n",sys.stderr)
#   begin_t = time.time_ns()
#   for x in range(num):
#     start_time = time.time()
#     currentDT = datetime.datetime.now()
#     line = ser.readline()   # read a '\n' terminated line
#     elapsed_time = time.time() - start_time
# #    print( str(elapsed_time),sys.stderr)
# #    print(line,sys.stderr)
# # print(currentDT.strftime("%H:%M:%S"),sys.stderr)
#     current_milli_time = lambda: int(round(time.time_ns() * 1000))
#     d.append(line);
#     result = line.decode('utf-8')
#   end_t = time.time_ns()
#   total_t = (end_t - begin_t) / 1000000.0 # ms
#   print("\n")
#   print(total_t,sys.stderr)
#   print("total_t: {:f} (s) \n".format(total_t/1000.0),sys.stderr)
#   print("time per sample: {:f} (ms) \n".format(total_t/num),sys.stderr)
#   print("Samples per second: {:f} \n".format(1000 * num / total_t),sys.stderr)
#   return result

@app.route("/")
def getsamples():
  result = ""
  for x in d:
    line = x
    result = result + line.decode('utf-8')
  return result

if __name__ == "__main__":
  app.run(host='127.0.0.1', port=80, debug=True)
