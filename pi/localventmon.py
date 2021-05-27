#!/usr/bin/env python

import os
import subprocess
import serial
import docker

# constants 
serialFd = "/dev/ttyUSB0"
serialBaudrate = 500000

# docker stuff
port_bindings = {'8081/tcp' : 80, '6111/udp' : 6111}
volume_binding = {'/home/pi/VentMon/PIRDS-docker-local/logger_src/data' : {'bind': '/data', 'mode': 'rw'}}


# top menu    
menuStrings = ["START local VentMon", "STOP local VentMon", "change serial port info", "check for updates", "quit"]
def print_menu():      
    print 30 * "-" , "VENTMON LOCAL" , 30 * "-"
    print "1. " , menuStrings[0] 
    print "2. " , menuStrings[1]
    print "3. " , menuStrings[2]
    print "4. " , menuStrings[3]
    print 67 * "-" 

# loop for menu
def menu_loop():
    while True:          
        print_menu()    
        choice = input("please select from the above menu [1-4]: ")
     
        if choice==1:     
            print "\n", 5 * "-", menuStrings[choice - 1], 5 * "-"  
            start_ventmon()
        
        elif choice==2:
            print "\n", 5 * "-", menuStrings[choice - 1], 5 * "-"  
            ## You can add your code or functions here
        elif choice==3:
            print "\n", 5 * "-", menuStrings[choice - 1], 5 * "-"  
            print "Menu 3 has been selected"
            ## You can add your code or functions here
        elif choice==4:
            print "\n", 5 * "-", menuStrings[choice - 1], 5 * "-"  
            print "Menu 3 has been selected"
            ## You can add your code or functions here
        elif choice==5:
            print "\n", 5 * "-", menuStrings[choice - 1], 5 * "-"  
            ## You can add your code or functions here
            break;
        else:
            # Any integer inputs other than values 1-5 we print an error message
            raw_input("Wrong option selection. Enter any key to try again..")


# kick off ventmon process
def start_ventmon():
    if check_serial() == True:
        start_docker()
        start_javascript()
       # start_browser()
    else:
        print "ERROR: no VentMon connected"


# check whether ventmont is active on expected serial port
def check_serial():
    print "checking serial port: " , serialFd
    ser = serial.Serial()
    ser.baudrate = serialBaudrate
    ser.port = serialFd
    # print ser
    try:
        ser.open()
    except:
        print "FIXME!"
        print ser.is_open
    return ser.is_open


# start docker 
def start_docker():
    dockerClient = docker.from_env()

    # check to see if we have any containers running (only expect one to be running)
    if len(dockerClient.containers.list()) == 0:
        print "starting docker image..."
        dockerClient.images.build(path="/home/pi/VentMon/PIRDS-docker-local", tag="pirds-logger")
        dockerClient.containers.run(name='logger', image='pirds-logger', detach='True', remove='True', ports=port_bindings, volumes=volume_binding) 
    else:
        # get the container with the logger image and check whether it's running
        container = dockerClient.containers.get('logger')
        containerState = container.attrs['State']
        if container.attrs['State']['Status'] == "running":
            print "docker image is already running..."
        else:
            # if the contaier has been run, but is not currently running stop and re-start it
            print "re-starting docker image..."
            container.stop()
            dockerClient.images.build(path="/home/pi/VentMon/PIRDS-docker-local/", tag="pirds-logger")
            dockerClient.containers.run(name='logger', image='pirds-logger', detach='True', remove='True', ports=port_bindings, volumes=volume_binding)  

   # container = dockerClient.containers.get('logger')
   # print container.attrs['Config']['Image']
   # containerState = container.attrs['State']
   # print containerState['Status']
   # print container.attrs['State']['Status']


   # containers = dockerClient.containers.list()
   # print "docker containers: ", containers

   # containers = dockerClient.containers.list(filters={'name': 'logger'})
   # print "docker containers: ", containers

   # container = dockerClient.containers.get('logger')
   # print container.attrs['Config']['Image']
   # containerState = container.attrs['State']
   # print containerState['Status']
   # print container.attrs['State']['Status']

   # container.stop()


# start vent display js
def start_javascript():

    # FIXME need to check whether node is running
    # thsi only works with python 3.5....need a better solution 
    pidOutput = subprocess.run(["pidof", "node"], capture_output=True, text=True).stdout
    print(pidOutput)
    
    #print subprocess.check_output(["pidof", "node"], stderr=subprocess.STDOUT)
    os.system("cd ~/VentMon/vent-display")
    os.system("node serialserver.js --uaddress=127.0.0.1 --sport=/dev/ttyUSB0 --uport=6111")
    os.system("cd ../")
    #print subprocess.check_output(["pidof", "node"])
    pidOutput = subprocess.run(["pidof", "node"], capture_output=True, text=True).stdout
    print(pidOutput)



# open browser 
def start_browser():
    os.system("chromium-browser http://localhost:8081 &")


# main
def main():
    menu_loop()

if __name__ == "__main__":
    main()






def run(*popenargs, **kwargs):
    input = kwargs.pop("input", None)
    check = kwargs.pop("handle", False)

    if input is not None:
        if 'stdin' in kwargs:
            raise ValueError('stdin and input arguments may not both be used.')
        kwargs['stdin'] = subprocess.PIPE

    process = subprocess.Popen(*popenargs, **kwargs)
    try:
        stdout, stderr = process.communicate(input)
    except:
        process.kill()
        process.wait()
        raise
    retcode = process.poll()
    if check and retcode:
        raise subprocess.CalledProcessError(
            retcode, process.args, output=stdout, stderr=stderr)
    return retcode, stdout, stderr






