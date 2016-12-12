# Description

Some lessons I learned doing RPUno.

# Table Of Contents:

6. [^5 ADC7 is .333 of PWR][#5-adc7-is-333-of-pwr]
5. [^4 Pull-down on IO2][#4-pull-down-on-io2]
4. [^2 Reduce EMI][#2-reduce-emi]
3. [^1 Reduce Current Sense Noise][#1-reduce-current-sense-noise]
2. [^1 Battery Connector Polarity][#1-battery-connector-polarity]
1. [^0 Add Reversed Battery Protection][#0-add-reversed-battery-protection]


## ^5 ADC7 is .333 of PWR

PWR node can go to 14V so swap 200k on R37 to R36 and swap the 100k on R36 to R37. Version ^4 changed from 6V to 12V SLA but I never looked at ADC7.


## ^4 Pull-down on IO2

When the ATMega328p starts its pins are in high-z mode (/w ESD diode clamps to VCC and 0V). So the gate on Q19 can float near its threshold voltage, which is what I am seeing with this first board, so I added a pull-down. 
    
Note: The RPUadpt should get power from +5V and on the RPUpi the VIN power should be used to power only the SBC. The RPUftdi can apply its USB power to the +5V line, in which case on the RPUno the VIN line gets back driven to about 4.8V and the pluggable IO is ESD protected (clamped) to that level (which seems fine). 


## ^2 Reduce EMI

While working on Irrigat7 I realized that as the current in the buck converter diode (D4) switches off and the current flow that feeds the inductor swaps over to the input capacitor (C4) it will cause magnetic fields that drive the changing current flow to the outside edge of the trace or plane. The fast changing current builds a magnetic bubble that expands as current eddies block the magnetic flux in the conductive trace or plane, which drive the current flow to the outermost edge. Put another way the cascading magnetic fields and current loops push rapidly changing current flow right to the edge of the copper, and if that copper happens to be the ground plane then that is bad news. The rapidly changing current flow on the 0V node needs to happen on a small island (copper poor) to confine, localize and minimize the EMI. The small island should be connected to the main 0V plane in a way that is not seen as an edge for the fields to push the rapidly changing current flow onto. I used some via  in the center of a copper islands (e.g. a copper poor) that connects the 0V side of C4, D4, C7, and C8. Now the fast changing current flow can happen on the island edges and not get pushed onto the main 0V plane.

![BeforIsland](./14140^2,BeforSwitchingIsland.jpg "Befor Island")
![WithIsland](./14140^3,WithSwitchingIsland.png "With Island")


## ^1 Reduce Current Sense Noise

Current sense noise is disrupting float charge cycle.

The LT3652 pin 12 is switching between the PV voltage and a diode drop bellow ground. That could be capacitivly coupling to the current sense line, but what I think is causing the problem is magnetic fields from fast changing current flows in D4, U1 pin 12 and perhaps even C1 induces voltage in the current sense path. I need to get the current sense path away from that stuff and use ground planes to shield it. 
    
![BadPath](./14140^1,CurrentSensePath.png "Bad Sense Path")
![ImprovedPath](./14140^2,CurrentSensePath.png "Improved Sense Path")

Disrupting the charge cycle, as ^1 did, will also damage the battery, so the board was scraped (execpt a few parts).

Testing shows this change worked, the current limit shifted from 0.86A (^1) to 1.32A (^2). I salvaged the LT3652 chip from the first board so it was the same device. The float charge timing is now working correctly as well.


## ^1 Battery Connector Polarity

Polarity does not match CC01 connector, it should match.


## ^0 Add Reversed Battery Protection

Push updates from another board (CC01^4) to this board, I did not build a RPUno^0 board. 

