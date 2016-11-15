# Description

This is a list of Test preformed on each RPUno after assembly.

# Table of References


# Table Of Contents:

    1. Basics
    2. Assembly check
    3. IC Solder Test
    4. Reverse Battery Protection
    5. Battery Disconnect
    6. Power Protection
    7. TPS3700 Window Comparator
    8. LT3652 Power Up Without Battery
    9. LT3652 Load Test
    10. +5V From OKI-78SR-5
    11. 20mA Source
    12. Set MCU Fuse and Install Bootloader


## Basics

These tests are for an assembled RPUno board 14140^5 which may be referred to as a Unit Under Test (UUT). If the UUT fails and can be reworked then do so, otherwise it needs to be scraped. 

**Warning: never use a soldering iron to rework ceramic capacitors due to the thermal shock.**
    
Items used for test.

![ItemsUsedForTest](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/Testing/14140,ItemsUsedForTest.jpg "RPUno Items Used For Test")


## Assembly check

After assembly check the circuit carefully to make sure all parts are soldered and correct, note that the device making is labeled on the schematic and assembly drawing.
    
NOTE: U2 is not yet on the board, so everything with +5V will not have power.


## IC Solder Test

The bottom pad on U1 dissipates heat as well as providing a ground, it is not connected to any other pin. The 0V plane is ground for this test. Check that a diode drop is present from each pin to the ground by measuring with a DMM's diode test between each pin and the 0V plane (reversed polarity). U1 pins 12, 10 and 9 are connected to L1 and R3 and read with a low (.13V) value. U2 not pop yet. U3 and U4 give a diode value on pins 1 and 5, while pins 3 and 4 are connected to R3 and L1 and read with a low value. U5 and U6 gives normal diode values where expected. U5 pin 2 and U6 pins 3, 5, 21 are connectd to ground and should have a short to 0V.


## Reverse Battery Protection

Apply a current limited (&lt;20mA) supply set with 14V to the +BAT and -BAT connector in reverse and verify that the voltage does not get through to TP4 (^4 PWR). Note some voltage (.2V) will be seen on TP4 because of how the shutdown through Q3 works.
    

## Battery Disconnect

Apply a current limited (&lt;20mA) supply set with 14V to the +BAT and -BAT connector and verify that voltage does not get through to TP4. 


## Power Protection

Apply a current limited (20mA) supply to the PV input with reverse polarity and ramp the voltage up to 30V, verify that no current flows.


## TPS3700 Window Comparator 

Connect a 20mA rated LED between J4.1 and J4.3. Apply a current limited (&lt;50mA) supply starting at 12V to the +BAT and -BAT connector. Short S2 and then release it, which will force the battery to connect. Check that TP4 has been latched e.g. has voltage from supply. Increase the supply slowly until the LED turns on, but no more than 14V. This is the voltage at which the load connects, and should be about 13.1V. Now slowly reduce the supply until the LED turns off. This is the voltage at which the battery disconnects and should be about 11.58V. The parts used determine the actual voltages. 

```
        TODO:  some data from unit(s)
            { "LOAD_CONNNECT":[12.95,12.99,],
               "DISCONNECT":[11.42,11.45,] }
```


## LT3652 Power Up Without Battery

Connect 100 kOhm resistor to both the PV side and BAT side thermistor inputs to simulate room temperature. Connect an LED with 1k Ohm series resistor to the +LD and -LD. Apply a current limited (50mA) supply to the +PV and -PV inputs and adjust it to about 14V. Verify no output.  Next adjust the supply to 19V and verify a regulated voltage (13.63V) between +BAT and -BAT* pins. 

NOTE: the LT3652 goes into fault when started into my HP6050A in CC mode, which is by design. I can start the LT3652 with the load off, or in CV mode and then switch to CC. This note is to remind me that it is an expected behavior.

```
        TODO:  some data from unit(s)
            { "VOUT100K":[13.55,13.65,] }
```


## LT3652 Load Test

NOTE: this test is likely beyond the scope of the DIY, but anyone making more than a few of these should load test the converter and make sure it can dissipate heat.

Connect 100 kOhm resistor to both the PV side and BAT side thermistor inputs to simulate room temperature. Connect a 20mA rated LED between J4.1 and J4.3. A voltage mode electronic load is used for this test, however, a partly discharged battery may also give results. Connect the electronic load between  the +BAT and -BAT pins. Set the electronic load voltage to 12.8V to simulate a battery. Connect +PV and -PV to a CC/CV mode supply with CC set at 150mA and CV set at 21V, apply the power. Verify the solar power point voltage is about 16.9V with the supply current CC at 150mA. Increase the supply CC setting until its voltage increases to 21V, e.g. the supply changes from CC to CV mode, and check that the charge controller is current limiting at about 1A with 0R068 placed on R3, also check if U1 (LT3652) is getting hot. Note that the voltage at UUT is higher than at the load because the wires drop some voltage (the load is still running at the 12.8V). 

```
        TODO:  some data from unit(s)
            { "PP100K@150mA&amp;12V8":[16.86,16.88],
               "CURR_LIMIT":[1.28,1.30,] }
```


## +5V From OKI-78SR-5

NOTE: U2 needs added to the board.

Power up like durring the load test and check the +5V supply at the ICSP header (e.g. between IOREF and 0V)

```
        TODO:  some data from unit(s)
            { "+5V":[5.00,4.99] }
```

## Current Sources

Use a DMM to measure the 22mA current source for A0 from J4.1 to J4.3 (V0) and A1 from J4.5 to J4.3. Pulse Loop 17mA source, and Pulse Pull 10mA pull up current source. Digital IO 22mA current source. 
    
NOTE: 17MA_PL and 10MA_PL need IO9 pulled up to IOREF (the pin is floating).

```
        TODO:  some data from unit(s)
            { "22MA_A0":[20.7,21.6,],
               "22MA_A1":[21.1,21.8,],
               "17MA_PL":[18.6,18.4,],
               "10MA_PL":[10.2,10.0,],
               "22MA_IO":[21.7,22.0,]}
```


## Set MCU Fuse and Install Bootloader

The MCU needs its fuses set, so a Makefile is used to do that. Apply a 5V current limited source (about 30mA*) to +5V. Check that the input current is for a blank MCU (e.g. less than 5mA).

Use the <https://github.com/epccs/RPUadpt/tree/master/Bootload> Makefile 

Connect the ICSP tool and run "make fuse" to program the fuses, next run "make isp" to install the bootloader.Disconnect the ICSP tool and measure the input current. Turn off power.

```
        TODO:  some data from unit(s)
            { "I_IN_BLANKMCU_mA":[4.2,],
            "I_IN_16MHZ_EXT_CRYST_mA":[11.0,]}
```

