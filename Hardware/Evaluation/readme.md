# Description

This shows the setup and method used for evaluation of RPUno.

# Table of References


# Table Of Contents:

8. [^5 Solenoid FW Operates K3](#5-solenoid-fw-operates-k3)
7. [^5 ADC and DIO](#5-adc-and-dio)
6. [^5 Heat No Longer Problem](#5-heat-no-longer-problem)
5. [^4 Heat Wick](#4-heat-wick)
4. [^2 Log CCtest](#2-log-cctest)
3. [^2 Log PV_IN and PWR for a day](#2-log-pv_in-and-pwr-for-a-day)
2. [^2 20mA Loop With Open Collector To ICP1](#2-20ma-loop-with-open-collector-to-icp1)
1. [^1 Mounting](#1-mounting)


## ^5 Solenoid FW Operates K3

An interactive Command Line Interface for [Solenoid][6] control is wired to LED's to see its function. In my [reset all video][7] each solenoid is cycled twice (opps), but the logic control is working. After connecting the K3 board I fixed the control program so it cycles each solenoid once as seen when [RPUno with K3 video][8] is seen. 
    
[6]: https://github.com/epccs/RPUno/tree/master/Solenoid
[7]: http://rpubus.org/Video/14140%5E5_SolenoidResetAllLogic.mp4
[8]: http://rpubus.org/Video/14140%5E5WithK3%5E0.mp4

![Solenoid Testing](./RPUno^5_SolenoidResetAllTesting.jpg "Solenoid Reset All Testing")

![RPUno With K3](./RPUno^5WithK3^0.jpg "RPUno With K3")

Update: Added more options to the [Solenoid][6] program (save/load settings to/from EEPROM) and connected some of Orbits (58874N) latching valves (which work for me with a 24V 50 mSec pulse). Have a look at the [video of RPUno (modified ^4) with K3 controlling some latching valves][9].

[9]: http://rpubus.org/Video/14140%5E4_K3%5E0_WithLatchingValves.mp4

![K3 RPUno with Valves](./RPUno^4+mod_K3^0_wLatchingValves.jpg "RPUno and K3 with Latching Valves")


## ^5 ADC and DIO

Interactive Command Line Interface for [Analog][4] updated, and for [Digital][5] created. 

[4]: https://github.com/epccs/RPUno/tree/master/Adc
[5]: https://github.com/epccs/RPUno/tree/master/Digital

![Digital and Analog](./RPUno^5_DioAdc.jpg "Digital and Analog")


## ^5 Heat No Longer Problem

Heat is no longer a problem. Testing shows it will do 1.3A all day with the thermistor (R23) placed off-board (e.g. the green-white wire), and shuts down when R23 is heated to 40 degC.


## ^4 Heat Wick

In order to run a Raspberry Pi, I will need to push the solar charge controller more than in the past. Testing shows it will do 1.28A but then gets hot and with the thermistor R23 in place shuts down after a few minutes. It starts back up after cooling down, but I need to wick more of the heat away and keep it running.

![Heat Wick](./14140^4_HeatWicking.jpg "Heat Wick")


## ^2 Log CCtest

Overview of setup: the solar PV (SLP003-12U) is on roof (that white roof coating is impossible  to photo)

![SLP003](./14140^2_WithSLP003onRoof.jpg "With SLP003 on Roof")

Enclosure with RPUno^2, RPUadpt^1 (used for point to point RS422), and CCtest^1 clipped on DIN rail

![CCtest](./14140^2_WithSLP003andCCtest.jpg "With CCtest")

CCtest^0 boad is wired to RPUadpt^1 board 

```
{Load LD[0:3]:DIO[10:13], ADC:{PV:ADC1,CHRG:ADC3,DISCHRG:ADC2}}
```

![CCtest Wiring](./14140^2_WithSLP003andCCtestWiring.jpg "CCtest Wiring")

Using an interactive command line [CCtest firmware][3] to log the charge control and load test. I'm not happy with how the test works. I started the test in the evening when the PV was bellow 18V, so I connected a 24V wall wart. I had to keep it connected to hold the input above 18V or the load stepping stops. It stopped at step 9 after I unplugged the wall wart and considered what had happened and if I needed to start over. I need to improve the test, and then everything bellow will get updated... anyway, the data is still interesting.

[3]: https://github.com/epccs/RPUno/tree/master/CCtest

 The data that follows was recorded in a terminal session:

```
/0/cctest?
{"DIS_A":"0.013","PV_V":"12.75","PWR_V":"6.52","TIME":"5","LD":"0"}
{"PV_A":"0.260","CHRG_A":"0.780","PV_V":"18.34","PWR_V":"6.67","TIME":"82119","LD":"1"}
{"DIS_A":"0.040","PV_V":"21.72","PWR_V":"6.56","TIME":"84118","LD":"2"}
{"DIS_A":"0.050","PV_V":"24.03","PWR_V":"6.51","TIME":"86119","LD":"3"}
{"DIS_A":"0.057","PV_V":"24.03","PWR_V":"6.50","TIME":"88120","LD":"4"}
{"DIS_A":"0.066","PV_V":"24.03","PWR_V":"6.50","TIME":"90121","LD":"5"}
{"DIS_A":"0.073","PV_V":"24.03","PWR_V":"6.50","TIME":"92122","LD":"6"}
{"DIS_A":"0.080","PV_V":"24.03","PWR_V":"6.50","TIME":"94124","LD":"7"}
{"DIS_A":"0.088","PV_V":"22.05","PWR_V":"6.49","TIME":"96124","LD":"8"}
{"DIS_A":"0.073","PV_V":"18.55","PWR_V":"6.48","TIME":"108335","LD":"9"}
{"DIS_A":"0.102","PV_V":"21.30","PWR_V":"6.48","TIME":"110337","LD":"10"}
{"DIS_A":"0.112","PV_V":"24.03","PWR_V":"6.47","TIME":"112337","LD":"11"}
{"DIS_A":"0.119","PV_V":"24.03","PWR_V":"6.47","TIME":"114338","LD":"12"}
{"DIS_A":"0.128","PV_V":"24.03","PWR_V":"6.47","TIME":"116339","LD":"13"}
{"DIS_A":"0.136","PV_V":"24.03","PWR_V":"6.46","TIME":"118340","LD":"14"}
{"DIS_A":"0.144","PV_V":"24.03","PWR_V":"6.46","TIME":"120341","LD":"15"}
{"DIS_A":"0.151","PV_V":"24.03","PWR_V":"6.45","TIME":"122342","LD":"15"}
{"DIS_A":"0.151","PV_V":"24.03","PWR_V":"6.45","TIME":"124343","LD":"15"}
{"DIS_A":"0.128","PV_V":"24.03","PWR_V":"6.45","TIME":"130519","LD":"15"}
{"DIS_A":"0.128","PV_V":"0.31","PWR_V":"6.39","TIME":"4983617","LD":"15"}
{"DIS_A":"0.128","PV_V":"0.26","PWR_V":"6.34","TIME":"17049416","LD":"15"}
{"DIS_A":"0.129","PV_V":"0.23","PWR_V":"6.30","TIME":"30075304","LD":"15"}
/0/cctest?
{"PV_A":"0.131","CHRG_A":"0.269","PV_V":"15.27","PWR_V":"6.41","TIME":"56850744","LD":"0"}
/0/cctest?
{"PV_A":"0.145","CHRG_A":"0.292","PV_V":"15.40","PWR_V":"6.63","TIME":"59078028","LD":"0"}
/0/cctest?
{"PV_A":"0.154","CHRG_A":"0.312","PV_V":"15.43","PWR_V":"6.67","TIME":"61687246","LD":"0"}
/0/cctest?
{"PV_A":"0.152","CHRG_A":"0.307","PV_V":"15.48","PWR_V":"6.69","TIME":"63693940","LD":"0"}
/0/cctest?
{"PV_A":"0.105","CHRG_A":"0.233","PV_V":"17.12","PWR_V":"6.68","TIME":"66580132","LD":"0"}
/0/cctest?
{"PV_A":"0.105","CHRG_A":"0.234","PV_V":"17.14","PWR_V":"6.69","TIME":"66661628","LD":"0"}
```

The time is from the millis() function. I sent a carriage return to stop the load because I want to see how it charged after that. It switches to float slowly, so I need to see that better. Loaded [Adc firmware][2].

[2]: https://github.com/epccs/RPUno/tree/master/Adc

This shows 20Sec intervals of what the ATmega328 ADC can see as the charge control  (LT3652) switches from bulk charge to float charge.

```
/0/analog? 7,6,3,2,1
{"PWR_V":"6.59","PV_V":"15.12","DISCHRG_A":"0.000","CHRG_A":"0.293","PV_A":"0.146"}
{"PWR_V":"6.64","PV_V":"15.22","DISCHRG_A":"0.000","CHRG_A":"0.293","PV_A":"0.146"}
{"PWR_V":"6.66","PV_V":"15.27","DISCHRG_A":"0.000","CHRG_A":"0.292","PV_A":"0.145"}
{"PWR_V":"6.67","PV_V":"15.30","DISCHRG_A":"0.000","CHRG_A":"0.294","PV_A":"0.146"}
{"PWR_V":"6.68","PV_V":"15.46","DISCHRG_A":"0.000","CHRG_A":"0.296","PV_A":"0.145"}
{"PWR_V":"6.69","PV_V":"16.08","DISCHRG_A":"0.000","CHRG_A":"0.279","PV_A":"0.132"}
{"PWR_V":"6.69","PV_V":"16.42","DISCHRG_A":"0.000","CHRG_A":"0.264","PV_A":"0.122"}
{"PWR_V":"6.68","PV_V":"16.68","DISCHRG_A":"0.000","CHRG_A":"0.251","PV_A":"0.115"}
{"PWR_V":"6.68","PV_V":"16.73","DISCHRG_A":"0.000","CHRG_A":"0.244","PV_A":"0.112"}
{"PWR_V":"6.68","PV_V":"16.78","DISCHRG_A":"0.000","CHRG_A":"0.240","PV_A":"0.109"}
{"PWR_V":"6.69","PV_V":"16.83","DISCHRG_A":"0.000","CHRG_A":"0.234","PV_A":"0.106"}
{"PWR_V":"6.68","PV_V":"16.91","DISCHRG_A":"0.000","CHRG_A":"0.228","PV_A":"0.103"}
{"PWR_V":"6.69","PV_V":"17.01","DISCHRG_A":"0.000","CHRG_A":"0.227","PV_A":"0.102"}
{"PWR_V":"6.68","PV_V":"17.12","DISCHRG_A":"0.000","CHRG_A":"0.223","PV_A":"0.099"}
{"PWR_V":"6.68","PV_V":"17.17","DISCHRG_A":"0.000","CHRG_A":"0.217","PV_A":"0.096"}
{"PWR_V":"6.68","PV_V":"17.17","DISCHRG_A":"0.000","CHRG_A":"0.213","PV_A":"0.095"}
```

## ^2 Log PV_IN and PWR for a day

Overview of setup: the solar PV (SLP003-12U) is on the trash container

![Setup Day Log](./14140^2_ADC60SecIntervalSetup.jpg "Log for day Setup")

Inside the box an RPUadpt board is setup for point to point and connected to a long CAT5 cable that runs to a Linux box running the terminal session. The battery used is was a 6V SLA with 7AH capacity (Power-Sonic PS-670)

![Setup Day Log Box](./14140^2_ADC60SecIntervalSetupBox.jpg "Log for day Setup Box")

Using an interactive command line program to log the solar input voltage (PV_IN) and the battery voltage (PWR). The interval between reading is 60 seconds, which was set in the [Adc Firmware][1]. The raw data was recorded in a terminal session:

[1]: https://github.com/epccs/RPUno/tree/master/Adc

```
 rsutherland@straightneck:~/Samba/RPUno/Adc$ script -f -c "picocom -b 115200 /dev/ttyUSB0" [14140^2_ADC60SecInterval.log](./14140^2_ADC60SecInterval.log)
```

Exta line feeds? the UART stdio redirect needs some more work (done), anyway they can be removed:

```
grep -v '^[[:space:]]*$' 14140^2_ADC60SecInterval.log > 14140^2_ADC60SecInterval.log.clean
```

Next use one of the JSON to CVS converters, and then it can be opened with LibreOffice Calc (version 5.0.6.3) and graphed.

![Day Graph](./14140^2_ADC60SecInterval.png "Day Graph")

In the morning, I found the solar PV just outside of the light so I moved it into the light and then the bulk charge was so fast it is barely noticeable on the graph (line 1711 in raw data). During bulk charging the PV is held at the MPPT voltage (16.8V) which decreases some with temperature. After bulk changing the battery is held at a temperature dependent absorption voltage. SLA batteries should have one to three hours per day at the absorption voltage. Missing a few days will not end the battery, but some sulfation deposits occur on those days. Sulfation is simple to understand it is like the lime deposits in a hot water heater, some of the lead sulfide dissolves (like lime) in the battery water, and then when the battery heats up in the morning it deposits in places that don't participate in the charging process. Absorption removes the lead sulfide from the battery water, and it will take some time to dissolve (and saturate the solution).


## ^2 20mA Loop With Open Collector To ICP1

The 20mA current source provides 17.5mA of current, which is acceptable (I don't want to increase it until a few more are seen). An HT^1 is shown operating on the loop in the image.

![HT Event on Loop](./14140^2,EventLoopWithHT.jpg "HT Event on Loop")

The scope shows the HT sensor in a pool of water with some ice. The important thing  is that the ICP1 pin is pulled down by an open collector when the loop current is over 6.5mA.

![HT Event on Loop with Scope](./14140^2,EventLoopWithHTscope.jpg "HT Event on Loop with Scope")

Basically this shows that this event loop is a good direction to proceed.


## ^1 Mounting

DIN rail is often used with industrial control equipment. It makes mounting very easy. Unmounting is also easy since there is a place for a flat blade screwdriver to lever the plastic mounting latch away from the DIN rail. 

![DIN Mounting](./14140^1,DINMnt.jpg "DIN Mounting")

RPUftdi, RPUadpt shield also need to mount.

![Shield Mounting](./14140^1,WithRPUftdi.jpg "Shield Mounting")

