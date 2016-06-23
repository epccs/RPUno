#!/usr/bin/env python3
# Line ending is the gift from hell, on Linux lines end with LF, on Windows most editors add a CR-LF pair 
# but that will confuse bash and/or the env function and thus most liky look for "python\r" and fail. 
# this python file also needs to be set as an executable with chmod +x load_profile.py

# Profile for Black & Decker Model NO. TO1303SB
# http://www.walmart.com/ip/22910666
# Note: Aluminum foil is taped (Kapton) to the door shiny side facing inward. Also, the door is propped open some to aid cool down (see image). 

# I am using python3 so make sure the package is for it. 
# sudo apt-get update
# sudo apt-get install python3-serial 
import json
import serial
# import io
from time import sleep

# pwm settings (uint8_t) for eeprom, each is used for two seconds.
profile_pwm = [ 255, # buzzer
                50, 127, 190, 200, 200, # 10 sec of profile run hard until flux activation 
                200, 200, 200, 200, 200, # 20 sec of profile
                200, 200, 200, 200, 200, # 30 sec of profile
                200, 200, 200, 200, 200, # 40 sec of profile
                200, 200, 200, 200, 200, # 40 sec of profile
                190, 190, 190, 190, 190, # 50 sec of profile
                190, 190, 190, 190, 190, # 60 sec of profile
                180, 180, 180, 180, 180, # 70 sec of profile
                180, 180, 180, 180, 180, # 80 sec of profile
                180, 180, 180, 180, 180, # 90 sec of profile
                180, 180, 180, 180, 180, # 100 sec of profile
                170, 160, 150, 120, 95, # 110 sec of profile
                80, 70, 60, 50, 50, # 10 sec of profile ... flux activation (about 280F or 140C for lead type solder)
                50, 50, 50, 50, 50, # 20 sec of profile
                50, 50, 50, 50, 50, # 30 sec of profile
                50, 50, 50, 50, 50, # 40 sec of profile
                50, 50, 50, 50, 50, # 50 sec of profile
                60, 60, 60, 60, 60, # 60 sec of profile
                60, 60, 60, 60, 60, # 70 sec of profile
                255, 127, 190, 200, 200, # 10 sec of profile ... flux used up (about 356F or 180C for lead type solder)
                200, 200, 200, 200, 200, # 20 sec of profile
                200, 200, 200, 200, 200, # 30 sec of profile
                200, 200, 200, 200, 200, # 40 sec of profile
                200, 200, 200, 200, 200, # 50 sec of profile
                200, 200, 200, 200, 200, # 60 sec of profile
                190, 190, 190, 190, 190, # 70 sec of profile .. peak needs to be about 428F or 220C for lead, it melts at about 374F or 190C
                170, 150, 127, 110, 95, # 80 sec of profile  .. reflow peak and some soak
                75, 65, 55, 45, 35, # 90 sec of profile  .. reflow soak
                25, 15, 5, 255, 0, # 100 sec of profile .. start cooldown
                0, 0, 0, 0, 0, # 10 sec of profile
                0, 0, 0, 0, 0, # 20 sec of profile
                0, 0, 0, 0, 0, # 30 sec of profile
                0, 0, 0, 0, 0, # 40 sec of profile
                0, 0, 0, 0, 0, # 50 sec of profile
                0, 0, 0, 0, 0, # 60 sec of profile
                0, 0, 0, 0, 0, # 70 sec of profile
                0, 0, 0, 0, 0, # 80 sec of profile
                0, 0, 0, 0, 0, # 90 sec of profile
                0, 0, 0, 0, 0, # 100 sec of profile
                0, 0, 0, 0, 0, # 110 sec of profile
                0, 0, 0, 0, 0, # 120 sec of profile
                0, 0, 0, 0, 0, # 130 sec of profile
                0, 0, 0, 0, 0, # 140 sec of profile
                0, 0, 0, 0, 0, # 150 sec of profile
                0, 0, 0, 0, 0, # 160 sec of profile
                0, 0, 0, 0, 0, # 170 sec of profile
                0, 0, 0, 0, 0, # 180 sec of profile
                0, 0, 0, 0, 0, # 190 sec of profile
                0, 0, 0, 0, 0, # 200 sec of profile
                0, 0, 0, 0, 0, # 210 sec of profile
                0, 0, 0, 0, 0, # 220 sec of profile
                0, 0, 0, 0, 0, # 230 sec of profile
                0, 255, 0, 255, 255] # 240 sec of profile ... end of profile is marked by two buzzer values

sio = serial.Serial("/dev/ttyUSB0",9600, timeout=3)

# PySerial has this example which does seem to fix some of the accii to Unicode issues, but caused other problems and ran slow for me.
#sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser), newline='', line_buffering=True, write_through=True)

reflow_echo = sio.readline().strip()
print("init: " + reflow_echo[-10:].decode("utf-8")) 

# checking if /0/reflow? command was printed 
if ( reflow_echo[-3:] == b'ow?'):
    # stop the refow that should be running after reset
    sio.write(("\n").encode('ascii'))
    sleep(0.2)
    sio.write(("/0\n").encode('ascii'))
    reflow_echo = sio.readline().strip()
    print("cmd: " + reflow_echo.decode("utf-8"))
    while (not (reflow_echo[-2:] == b"/0")) :
        sleep(0.2)
        print("miss catch of command prompt, trying again")
        sio.write(("/0\n").encode('ascii'))
        reflow_echo = sio.readline().strip()
        print("cmd: " + reflow_echo.decode("utf-8"))

    print("Profile has been Stopped ready for command")

# now we can load the eeprom
eeprom_offset = 0
for pwm in profile_pwm:

    # notice how print, sendline, and expect differ in there line ending,
    command = "/0/ee? " + str(eeprom_offset)
    sio.write((command  + "\n").encode('ascii')) 
    sleep(0.2)
    reflow_echo = sio.readline().strip()
    print("ascii cmd: ".encode('ascii') + reflow_echo)

    # print("echo match: " + str(reflow_echo == (command).encode('ascii') ))
    while (not (reflow_echo == (command).encode('ascii') ) ) :
        sleep(0.2)
        print("miss catch of read command, trying again")
        sio.write((command  + "\n").encode('ascii'))
        reflow_echo = sio.readline().strip()
        print("cmd: ".encode('ascii') + reflow_echo)
    
     # JSON line after command
    reflow_echo = sio.readline().strip()
    print ("JSON: ".encode('ascii') + reflow_echo)
    json_reply = reflow_echo.decode("utf-8")
    
    # deserialize the JSON line into a python object
    data = json.loads(json_reply) # this will cause an error if JSON is not valid
    pwm_in_eeprom = data["EE["+str(eeprom_offset)+"]"]
    
    while (int(pwm_in_eeprom) != pwm):
        command = "/0/ee " + str(eeprom_offset) + "," + str(pwm)
        sio.write((command + '\n').encode('ascii'))
        sleep(0.2)
        reflow_echo = sio.readline().strip()
        print("cmd: ".encode('ascii') + reflow_echo)
        while (not (reflow_echo == (command).encode('ascii') ) ) :
            sleep(0.2)
            print("miss catch of write command, manualy run again")
            exit()
        
         # JSON line after command
        reflow_echo = sio.readline().strip()
        print ("JSON: ".encode('ascii') + reflow_echo)
        json_reply = reflow_echo.decode("utf-8")
    
        data = json.loads(json_reply) # this will cause an error if JSON is not valid
        pwm_in_eeprom = data["EE["+str(eeprom_offset)+"]"]
    eeprom_offset += 1 

