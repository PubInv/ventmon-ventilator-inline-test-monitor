#!/usr/bin/python
import time
import struct
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
import jsonpickle

import socket
import sys
import os
import traceback

# This makes it a rather short pattern.
# Up to 10,000 samples seems to work fine.
# Managing the continuity of time in the samples can be tedious, however.
SAMPLES = 3000
deqLock = threading.Lock()
my_deque = collections.deque(maxlen=SAMPLES)

app = Flask(__name__)

CORS(app)

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
      # because the measurementValue can be signed and Python tries
      # to abstract away the sign bit, we do this from:
      # https://stackoverflow.com/questions/20766813/how-to-convert-signed-to-unsigned-integer-in-python
#      v = self.measurementValue.to_bytes(4, 'big')
      packed = struct.pack('!l', self.measurementValue)  # Packing a long number.
#      print("packed = " + str(packed)+"\n")
      unpacked = struct.unpack('!l', packed)[0]  # Unpacking a packed long number to unsigned long

#      v = struct.pack('>i', self.measurementValue)
#      v = unpacked.to_bytes(4, 'big')
      struct.pack_into('>i',b,8,self.measurementValue)
#      b[8] = v[0];
#      b[9] = v[1];
#      b[10] = v[2];
#      b[11] = v[3];
#      b[3] = self.deviceLocation.to_bytes(1,'big')
      b[3] = ord(chr(self.deviceLocation))
      return b


class MeasurementEncoder(json.JSONEncoder):
    def default(self, m):
        if isinstance(m, Measurement):
            return {
                "event": "M",
                "type": m.measurementType,
                "loc": m.deviceType,
                "num": m.deviceLocation,
                "ms": m.measurementTime,
                "val": m.measurementValue,
            }
        return super(RoundTripEncoder, self).default(m)
# print json.dumps(data, cls=RoundTripEncoder, indent=2)


def terminate_with_newline(mbytes):
      b = bytearray(len(mbytes)+1)
      for i in range(len(mbytes)):
        b[i] = mbytes[i]
      b[12] = ord('\n')
      return b;

def read_from_port():
  global NUMREAD
  global my_deque
  global NUM_SENT_TO_DATA_LAKE
  global DATA_LAKE_SAMPLES_TO_SEND
  global sock
  global RUN_CNT
  global RUN_LINIT
  while True :
    if ((RUN_LIMIT > 0) and RUN_CNT >= RUN_LIMIT):
      os._exit(os.EX_OK)
      RUN_CNT = RUN_CNT + 1
    if(ser.inWaiting() <= 50):
      pass
    line = ser.readline()   # read a '\n' terminated line
    if (line[0] != 'M'):
      pass;
    NUMREAD = NUMREAD + 1;
    if (NUMREAD % REPORT_MODULUS) == 0:
      try:
        deqLock.acquire()
        print(str(len(my_deque)) + " ready!\n",sys.stderr)
      finally:
        deqLock.release()
      print(str(line) + "\n",sys.stderr)

      NUMREAD = 0;
    # performing a try except here really seems to
    # make the coding of characters all messed up,
    # I don't realy know why, it is like we get off by one character
    try:
      thisline = line.decode("utf-8")+"\n"
      # For bizarre reason I'm unable to figure out,
      # printing this here is NECESSARY!!!
      #      print(thisline)
      try:
        m = json.loads(thisline)

        if m["event"] == "M":
          # Warning! This is just a test
          # v = abs(m["val"])
          v = m["val"]
          minst = Measurement(m["type"], m["loc"], int(m["num"]),
                              m["ms"], v)
          try:
            deqLock.acquire()
            my_deque.append(minst)
          finally:
            deqLock.release();
          if  ((DATA_LAKE_SAMPLES_TO_SEND > 0) and (NUM_SENT_TO_DATA_LAKE < DATA_LAKE_SAMPLES_TO_SEND)):
            print(f'sending {NUM_SENT_TO_DATA_LAKE} {DATA_LAKE_SAMPLES_TO_SEND}')
            try:
              # Send data
              mbytes = minst.asBytes()
              # now pack with a newline!!
              bytes = terminate_with_newline(mbytes)
              print('sending "%s"' % str(bytes))

              return_value = sock.sendall(bytes)

              NUM_SENT_TO_DATA_LAKE = NUM_SENT_TO_DATA_LAKE+1
              if NUM_SENT_TO_DATA_LAKE >= DATA_LAKE_SAMPLES_TO_SEND:
                sock.close()
                os._exit(os.EX_OK)
            except OverflowError:
              print("Overflow error:", sys.exc_info()[0])
              traceback.print_exc(file=sys.stdout)
              os._exit(os.EX_OK)
            except BrokenPipeError:
              traceback.print_exc(file=sys.stdout)
              os._exit(os.EX_OK)
            except:
              traceback.print_exc(file=sys.stdout)
              print("Unexpected sending error:", sys.exc_info()[0])
      except TypeError as err:
        print("Type error: {0}".format(err),sys.stderr)
      except:
        print("Unexpected error:", sys.exc_info()[0],sys.stderr)
        traceback.print_exc(file=sys.stdout)
    except(UnicodeDecodeError):
      print("ERRRRROR\n",sys.stderr)


thread = threading.Thread(target=read_from_port)
thread.start()

def get_n_samples(n):
  global my_deque
  result = []
  print("n" + str(n)+"\n",sys.stderr)
  print(str(len(my_deque))+"ready \n",sys.stderr)
  for i in range(0,min(n,len(my_deque))):
    try:
      line = my_deque.pop()
      print(str(line) + "\n",sys.stderr)
      result.append(line);
    except IndexError:
      print("Indexerror\n",sys.stderr)
      break;
#  print("result = " + json.dumps(result, cls=MeasurementEncoder, indent=2) + "\n",sys.stderr)
# WARNING: I don't know why, but I have to print something here or it
# fails---possibly a timing issue!
  print("result!",sys.stderr)
  return result

def construct_result(samps):
  response = app.response_class(
    response=json.dumps(samps, cls=MeasurementEncoder, indent=2),
    status=200,
    mimetype='application/json'
  )
  return response

@app.route("/")
def getsamples():
  global my_deque
  try:
    deqLock.acquire()
    print("len (from full)" + str(len(my_deque))+"\n")
    samps = get_n_samples(len(my_deque));
    return construct_result(samps)
  finally:
    deqLock.release();

# Get at most the earliest n samples.
@app.route("/json")
def get_limit_samples():
  global my_deque
  try:
    deqLock.acquire()
    print("size of deque from json): " + str(len(my_deque))+"\n",sys.stderr)
    if request.args.get('n') == None:
      n = 0
    else:
      n = int(request.args.get('n'))
      print("len (from json)" + str(n) + " of " + str(len(my_deque))+"\n",sys.stderr)
      samps = get_n_samples(min(n,len(my_deque)))
      return construct_result(samps)
  finally:
    deqLock.release();

if __name__ == "__main__":
  print(f"Arguments count: {len(sys.argv)}")
  print(f"Name of the script      : {sys.argv[0]}")
  print(f"Arguments of the script : {sys.argv[1:]}")
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
  app.run(host='127.0.0.1', port=80, debug=True)
