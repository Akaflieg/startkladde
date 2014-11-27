#!/usr/bin/python3

import sys, os, atexit, signal
from multiprocessing import Process
import socketserver
import queue
import threading
import serial
import traceback
import time

if sys.platform == "linux2":
  STATUS_DIR    = "/tmp"  
  PID_FILE      = STATUS_DIR + "/serialhandler.pid"
elif sys.platform == "win32":
  STATUS_DIR    = "C:\\temp"  
  PID_FILE      = STATUS_DIR + "\\serialhandler.pid"


global queue

# @atexit.register
def unlink():
  print("unlinking", PID_FILE)
  os.unlink(PID_FILE)

def findpid():
  f = open(PID_FILE)
  p = f.read()
  f.close()
  return int(p)

def createpid():
  atexit.register (unlink)
  f = open(PID_FILE, "w")
  f.write("%d" % os.getpid())
  f.close()

def watch():
  try:
    while True:
      time.sleep (1)
  except KeyboardInterrupt:
    print('KeyboardInterrupt')
    try:
      if sys.platform == "linux2":
        os.kill(os.getppid(), signal.SIGKILL)
      elif sys.platform == "win32":
        import ctypes
        PROCESS_TERMINATE = 1
        handle = ctypes.windll.kernel32.OpenProcess(PROCESS_TERMINATE, False, os.getppid())
        ctypes.windll.kernel32.TerminateProcess(handle, -1)
        ctypes.windll.kernel32.CloseHandle(handle)
      else:
        print ("unknown operation system, could not kill parent process")
        
    except OSError: pass
    sys.exit()

class MyTCPHandler(socketserver.BaseRequestHandler):

  def handle (self):
    global queue
    print("created handle")
    while True:
      line = queue.get ()
      # print ("got line: ", line)
      if len (line) > 0:
        self.request.send(line)
        print ("sent line: %s" % line)
      queue.task_done()

class MyTCPServer (socketserver.TCPServer):

  def __init__(self, address, handler):
    self.allow_reuse_address = True
    socketserver.TCPServer.__init__(self, address, handler)

#  def handle_error(self, request, client_address):
#    print("socket error: ", request.__class__.__name__, client_address)
        
class ServerThread (threading.Thread):
    
  def run (self):
    # Create the server, binding to localhost on port 4711
    self.server = MyTCPServer(("localhost", 4711), MyTCPHandler)
    print("server created: ", self.server.server_address)
      
    while True:
      self.server.handle_request ()

class SerialThread (threading.Thread):
  def __init__ (self):
    threading.Thread.__init__(self)
    self.serial = None
    if sys.platform == "linux2":
      self.devices = ["/dev/ttyUSB0",
                      "/dev/ttyUSB1",
                      "/dev/rfcomm0",
                      "/dev/ttyS0"]
    elif sys.platform == "win32":
      self.devices = ["com0",
                      "com1"]
    else:
      self.devices = []
    
  def createSerial (self):
    for device in self.devices:
      try:
        self.serial = serial.Serial(device, 19200, timeout=1)
      except serial.serialutil.SerialException as exc:
        print ("SerialException on %s: %s" % (device, exc))
      else:
        print("serial created: %s" % device)
        return
    # no device could be opened, Wait some seconds to retry
    time.sleep (5)

  def run (self):
    global queue

    while True:
      # if the device has been destroyed, recreate. This is forced by setting it to None
      if self.serial == None:
        self.createSerial ()
        continue
        
      try:
        #line = self.serial.readline (None, '\r\n')
        line = self.serial.readline ()
        if len (line) == 0:
          # print ("empty line: %s" % line)
          # this happens after timeout; the call to getRI causes an exception if the device has been disconnected
          # we use this as a side effect to force to recreate the device after timeout
          ri = self.serial.getRI ()
          ## should the call work and the line is down, recreate
          #if not ri:
          #   print ("RI is down, try to reconnect")
          #   time.sleep (5)
          #   self.serial = None
          continue

      except serial.serialutil.SerialException as exc:
        # this happens when the cable is disconnected, but not on the Stylistic
        print("SerialException: ", exc)
        self.serial.close()
        time.sleep(1)
        self.serial = None
        continue

      except serial.portNotOpenError as exc:
        print("portNotOpenError serial exception: ", exc)
        self.serial.close()
        time.sleep(1)
        self.serial = None
        continue

      except IOError as exc:
        print("IOError serial exception: ", exc)
        self.serial.close()
        time.sleep(5)
        self.serial = None
        continue

      except Exception as exc:
        print("other serial exception: ", exc, exc.__class__.__name__)
        self.serial.close()
        time.sleep(5)
        self.serial = None
        continue
          
      # print ("put: %s" % line)
      print ("type of line: ", line.__class__.__name__)
      if queue.full ():
        print("throw away: ", queue.get())
      else:
        queue.put(line)

class FileThread (threading.Thread):
  def __init__ (self, filename):
    threading.Thread.__init__(self)
    self.serial = None
    self.filename = filename
    
  def run (self):
    global queue

    file = open (self.filename, 'r')
    while True:
      time.sleep (0.02)
      # the socket does not accept str which is unicode
      line = bytes (file.readline (), 'ascii')

      if len (line) == 0:
        continue

      # print ("put: %s" % line)
      if queue.full ():
        print("throw away: ", queue.get())
      else:
        queue.put(line)

if __name__ == "__main__":
  from optparse import OptionParser

  parser = OptionParser()
  parser.add_option("-f", "--file", dest="filename", help="read data from FILE", metavar="FILE")

  (options, args) = parser.parse_args()
  if options.filename:
    print ("Read from %s" % options.filename)
  else:
    print ("Read from serial")

  if os.path.exists(PID_FILE):
    pid = findpid()
    # pid file exists, check for real process
    if os.path.isdir ("/proc/" + str (pid)):
      # real serialhandler is running
      print("Process with PID = %d is running. Exiting" % pid)
      sys.exit (1)
    else:
      # the file was a relict. delete it and continue
      print("Stale pidfile exists with PID = %d; Removing it." % pid)
      os.unlink(PID_FILE)
                                                                                                    
  #create new pid file
  createpid()

  p = Process (target = watch)
  p.start()

  global queue
  queue = queue.Queue(10)

  if options.filename:
    serialThread = FileThread (options.filename)
  else:
    serialThread = SerialThread ()
  
  serverThread = ServerThread ()

  serialThread.start ()
  serverThread.start ()

  serialThread.join()
  serverThread.join()
  p.join()
