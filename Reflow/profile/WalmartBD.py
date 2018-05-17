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

# address on the RPU_BUS to load oven pwm settings
rpu_addr = '1'

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

# Opening the port will cause nDTR and nRTS to be set active which will then run the bootloader.
# On an RPUno with an RPUadpt this is controled using the management pair (e.g. DTR)  whicih needs to 
# reset (e.g. send the bootload address) the device to upload these oven pwm settings to.
sio = serial.Serial("/dev/ttyUSB0",38400, timeout=3)


# the bootloader takes about 2 seconds and a timeout of 3 seconds is provided
reflow_echo = sio.readline().strip()

# should be an empty buffer to show user
print("init: " + reflow_echo[-10:].decode("utf-8")) 


# stop the running command (if any)
sio.write(("\n").encode('ascii'))
sleep(0.2)

# checking id
command = "/"+rpu_addr+"/id?"
sio.write((command + "\n").encode('ascii'))

# the command is sent back
reflow_echo = sio.readline().strip()
print("cmd echo: ".encode('ascii') + reflow_echo)

# print("echo match: " + str(reflow_echo == (command).encode('ascii') ))
while (not (reflow_echo == (command).encode('ascii') ) ) :
    sleep(0.2)
    print("miss catch of read command, trying again")
    sio.write((command  + "\n").encode('ascii'))
    reflow_echo = sio.readline().strip()
    print("cmd echo: ".encode('ascii') + reflow_echo)

 # JSON line after id command
reflow_echo = sio.readline().strip()
print ("JSON: ".encode('ascii') + reflow_echo)
json_reply = reflow_echo.decode("utf-8")

# deserialize the JSON (e.g. {"id":{"name":"Reflow"}} ) line into a python object
data = json.loads(json_reply)
id_name = data["id"]["name"]
    
if ( id_name[-6:] == 'Reflow'):
    print("Reflow fw found ready for command")
else:
    print("I did not see a /reflow? command running")
    exit()

# now we can load the eeprom pwm values starting at address 42
eeprom_offset = 42
for pwm in profile_pwm:

    command = "/"+rpu_addr+"/ee? " + str(eeprom_offset)
    sio.write((command  + "\n").encode('ascii')) 
    sleep(0.2)
    reflow_echo = sio.readline().strip()
    print("cmd echo: ".encode('ascii') + reflow_echo)

    # print("echo match: " + str(reflow_echo == (command).encode('ascii') ))
    while (not (reflow_echo == (command).encode('ascii') ) ) :
        sleep(0.2)
        print("miss catch of read command, trying again")
        sio.write((command  + "\n").encode('ascii'))
        reflow_echo = sio.readline().strip()
        print("cmd echo: ".encode('ascii') + reflow_echo)
    
     # JSON line after command
    reflow_echo = sio.readline().strip()
    print ("JSON: ".encode('ascii') + reflow_echo)
    json_reply = reflow_echo.decode("utf-8")
    
    # deserialize the JSON (e.g. {"EE[42]":{"r":"255"}} ) line into a python object
    data = json.loads(json_reply)
    pwm_in_eeprom = data["EE["+str(eeprom_offset)+"]"]["r"]
    
    # don't write the same data again, note the /ee comand will check also
    while (int(pwm_in_eeprom) != pwm):
        command = "/"+rpu_addr+"/ee " + str(eeprom_offset) + "," + str(pwm)
        sio.write((command + '\n').encode('ascii'))
        sleep(0.2)
        reflow_echo = sio.readline().strip()
        print("cmd echo: ".encode('ascii') + reflow_echo)
        while (not (reflow_echo == (command).encode('ascii') ) ) :
            sleep(0.2)
            print("miss catch of write command, manualy run again")
            exit()
        
         # JSON line after command
        reflow_echo = sio.readline().strip()
        print ("JSON: ".encode('ascii') + reflow_echo)
        json_reply = reflow_echo.decode("utf-8")
        
        # deserialize the JSON (e.g. {"EE[42]":{"byte":"255","r":"255"}} ) line into a python object
        # byte: is what the mcu was sent
        # r: is the reading taken from EEPROM after writing was done, if it is different then EEPROM may have failed
        data = json.loads(json_reply)
        pwm_in_eeprom = data["EE["+str(eeprom_offset)+"]"]["r"]
    eeprom_offset += 1 

