# Description

This is a list of Test preformed on each RPUno after assembly.

# Table of References


# Table Of Contents:

1. [Basics](#basics)
2. [Assembly check](#assembly-check)
3. [IC Solder Test](#ic-solder-test)
4. [Reverse Battery Protection](#reverse-battery-protection)
5. [Battery Disconnect](#battery-disconnect)
6. [Power Protection](#power-protection)
7. [TPS3700 Window Comparator](#tps3700-window-comparator)
8. [LT3652 Power Up Without Battery](#lt3652-power-up-without-battery)
9. [LT3652 Load Test](#lt3652-load-test)
10. [Bias +5V](#bias-5v)
11. [+5V From OKI-78SR-5](#5v-from-oki-78sr-5)
12. [20mA Source](#current-sources)
13. [Set MCU Fuse and Install Bootloader](#set-mcu-fuse-and-install-bootloader)
14. [Self Test](#self-test)


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

Apply a current limited (&lt;30mA) supply starting at 12V to the +BAT and -BAT connector. Connect 1k ohm between U2 pin 1 and pin 2 to give the VIN latch a load. Short S2 and then release it, which will force the battery to connect. Check that PWR has been latched to the battery with TP4 and that the VIN latch has not set with U2 pin 1. Increase the supply slowly until VIN on U2 pin 1 has power, but no more than 14V. This is the voltage at which the load connects, and should be about 13.1V. Now slowly reduce the supply until the LED turns off. This is the voltage at which the battery disconnects and should be about 11.58V.

```
{ "LOAD_CONNNECT":[12.95,12.99,13.00,13.08],
  "DISCONNECT":[11.42,11.45,11.47,11.45] }
```


## LT3652 Power Up Without Battery

Connect 100 kOhm resistor to both the PV side and BAT side thermistor inputs to simulate room temperature. Connect an electronic load to the +BAT and -BAT. Connect 1k ohm between U2 pin 1 and pin 2 to give the VIN latch a load. Connect a current limited (50mA) supply to the +PV and -PV inputs turn it on and increase the voltage to about 14V. Verify no output. Next, increase the supply voltage to 19V and verify a regulated voltage (13.63V) between +BAT and -BAT* pins. 

NOTE: the LT3652 goes into fault when started into my HP6050A in CC mode, which is by design. I can start the LT3652 with the load off, or in CV mode and then switch to CC. This note is to remind me that it is an expected behavior.

```
{ "VIN@100K":[13.55,13.65,13.62,13.66,] }
```


## LT3652 Load Test

Connect 100 kOhm resistor to both the PV side and BAT side thermistor inputs to simulate room temperature. Connect an electronic load to the +BAT and -BAT. Connect 1k ohm between U2 pin 1 and pin 2 to give the VIN latch a small load. Set the electronic load voltage to 12.8V to simulate a battery. Connect +PV and -PV to a CC/CV mode supply with CC set at 50mA and  CV set at 0V. Apply power and increase the CV setting to 21V. Verify the solar power point voltage is about 16.9V, increase the supply current CC to 150mA and measure the power point voltage. Next increase the supply CC setting until its voltage increases to 21V, e.g. the supply changes from CC to CV mode, and check that the charge controller is current limiting at about 1.3A with 0R068 placed on R3, also check if U1 (LT3652) is getting hot. Note that the voltage at UUT is higher than at the load because the wires drop some voltage (the load is still running at the 12.8V). 

```
{ "PP100K@150mA&amp;12V8":[16.86,16.88,16.97,16.89,],
  "CURR_LIMIT":[1.28,1.30,1.32,1.32,] }
```

## Bias +5V

Apply a 5V current limited source (about 30mA*) to +5V (J7 pin 6 and pin 5). Check that the input current is for a blank MCU (e.g. less than 5mA). Turn off the power.

```
{ "I_IN_BLANKMCU_mA":[]}
```


## +5V From OKI-78SR-5

NOTE: U2 needs added to the board.

Connect 100 kOhm resistor to both the PV side and BAT side thermistor inputs to simulate room temperature. Connect a 12V SLA battery to the +BAT and -BAT. Connect +PV and -PV to a CC/CV mode supply with CC set at 150mA and  CV set at 0V. Apply power and increase the CV setting to 21V. After the battery has charged up to the connect voltage then VIN will connect and power U2. Measure the +5V supply at J7 pin 6 and pin 5. Turn power off.

```
{ "+5V":[5.00,4.99,4.95,4.96,] }
```


## Current Sources

The self-test checks current sources, but keep some data for now.

Connect 100 kOhm resistor to both the PV side and BAT side thermistor inputs to simulate room temperature. Connect an 12V SLA battery to the +BAT and -BAT. Connect +PV and -PV to a CC/CV mode supply with CC set at 150mA and  CV set at 0V. Connect a 10k Ohm pull-up resistor from IO9 (J10 pin 9) to +5V. Apply power and increase the CV setting to 21V. Use a DMM to measure the 22mA current source for A0 from J4.1 to J4.3 (V0) and A1 from J4.5 to J4.3. Pulse Loop 17mA source, and Pulse Pull 10mA pull up current source. Digital IO 22mA current source. 
    
```
{ "22MA_A0":[20.7,21.6,21.9,21.9,],
  "22MA_A1":[21.1,21.8,22.4,22.2.],
  "17MA_PL":[18.6,18.4,18.5,18.4],
  "10MA_PL":[10.2,10.0,10.3,10.2,],
  "22MA_IO":[21.7,22.0,22.1,22.1,]}
```

NOTE: IO9 needs to be pulled up to 5V, the pin is floating when the MCU is not programmed to control it.


## Set MCU Fuse and Install Bootloader

The MCU needs its fuses set, so a Makefile is used to do that. Apply a 5V current limited source (about 30mA*) to +5V. Check that the input current is for a blank MCU (e.g. less than 5mA).

Use the <https://github.com/epccs/RPUno/tree/master/Bootload> Makefile 

Connect the ICSP tool and run "make fuse" to program the fuses, next run "make isp" to install the bootloader.Disconnect the ICSP tool and measure the input current, wait 15 sec (or more) for the power to be settled. Turn off the power.

```
{ "I_IN_BLANKMCU_WITH_U2_mA":[4.2,4.4],
  "I_IN_16MHZ_EXT_CRYST_mA":[11.0,11.1]}
```

## Self Test

Use the <https://github.com/epccs/RPUno/tree/master/SelfTest> Firmware. Edit main.c such that "#define ADC_REF 5.0" has the correct value for the UUT.

Plug an [RPUftdi] shield with [Host2Remote] firmware onto an [RPUno] board (not the UUT but a separate board) and load [I2C-Debug] on it.

[RPUftdi]: https://github.com/epccs/RPUftdi
[Host2Remote]: https://github.com/epccs/RPUftdi/tree/master/Host2Remote
[RPUno]: https://github.com/epccs/RPUno
[I2C-Debug]: https://github.com/epccs/RPUno/tree/master/i2c-debug

Use picocom to set the bootload address on the RPUftdi shield. The RPUftdi is at address 0x30 and the UUT will be at address 0x31.

```
picocom -b 38400 /dev/ttyUSB0
...
/0/address 41
{"address":"0x29"}
/0/buffer 3,49
{"txBuffer":[{"data":"0x3"},{"data":"0x31"}]}
/0/read? 2
{"rxBuffer":[{"data":"0x3"},{"data":"0x31"}]}
```
Plug an [RPUadpt] shield with [Remote] firmware onto the UUT board. Note the RPUadpt address defaults to 0x31 when its firmware was installed.

[RPUadpt]: https://github.com/epccs/RPUadpt
[Remote]: https://github.com/epccs/RPUadpt/tree/master/Remote

Connect the Self Test [Wiring] to the UUT. Connect 100 kOhm resistor to both the PV side and BAT side thermistor inputs to simulate room temperature. Connect a 12V SLA battery to the +BAT and -BAT. Connect +PV and -PV to a CC/CV mode supply with CC set at 150mA and  CV set at 0V. Apply power and increase the CV setting to 21V. Once the UUT connects

[Harness]: https://raw.githubusercontent.com/epccs/RPUno/master/SelfTest/Setup/SelfTestWiring.png

Once the UUT has power (battery charged to > 13.1V) check that the VIN pin on the shield has power (this is not tested by the self-test so it has to be done manually).  Next, run the bootload rule in the Makefile (i.e. run "make bootload") to upload the self-test firmware to the UUT that the remote shield is mounted on. Use picocom to see the SelfTest results over its UART interface.

```
picocom -b 38400 /dev/ttyUSB0
picocom v1.7
...
Terminal ready
Self Test date: Feb 18 2017
I2C provided address 0x31 from RPU bus manager
+5V needs measured and then set as ADC_REF: 4.950 V
Charging with CURR_SOUR_EN==off: 0.108 A
PWR (Battery) at: 13.385 V
MPPT at: 16.999 V
ADC0 at: 0.000 V
ADC1 at: 0.000 V
ICP1 /w 0mA on plug termination reads: 1
CC_nFAULT measured with a weak pull-up: 1
Charging delta with CURR_SOUR_EN==on: 0.087 A
ADC0 with its own 20mA source on R1: 0.022 A
ADC1 with ICP1's 10mA on ICP1_TERM: 0.010 A
ICP1 /w 10mA on plug termination reads: 0
Dischrging at: 0.101 A
PV open circuit (LT3652 off) at: 21.242 V
ADC0 and digital curr source on R1: 0.044 A
ADC0 measure curr on R1 with DIO12 shunting: 0.028 A
ADC0 measure curr on R1 with DIO13 shunting: 0.028 A
ADC0 and ADC1 curr source on R1: 0.044 A
ADC0 measure curr on R1 with DIO10 shunting: 0.028 A
ADC0 measure curr on R1 with DIO11 shunting: 0.028 A
ICP1 10mA + 16mA curr source on ICP1_TERM: 0.028 A
ICP1 curr on ICP1_TERM with DIO4 shunting: 0.013 A
ICP1 curr on ICP1_TERM with DIO3 shunting: 0.013 A
To disconnect battery turn off the PV supply and LED should stop blinking
[PASS]
```

Before disconnecting the battery check that the VIN pin on the shield has no power, the test turns it off. Then turn off the power supply and verify disconnect of the battery.
