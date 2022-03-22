# PiMac

[![Arduino Compile Sketches](https://github.com/Andy4495/PiMac/actions/workflows/arduino-compile-sketches.yml/badge.svg)](https://github.com/Andy4495/PiMac/actions/workflows/arduino-compile-sketches.yml)
[![Check Markdown Links](https://github.com/Andy4495/PiMac/actions/workflows/CheckMarkdownLinks.yml/badge.svg)](https://github.com/Andy4495/PiMac/actions/workflows/CheckMarkdownLinks.yml)

The "PiMac" -- converting a 2005 [iMac G5][1] (Ambient Light Sensor) into a Raspberry Pi desktop.

I had an old iMac G5 in storage which, while obsolete, still had a nice 1680x1050 LCD display that I didn't want to throw away.

I had previously converted an old [iMac G4][2] to a 1680x1050 DVI display using a [procedure][4] by Dremel Junkie, and was now looking to re-purpose my old iMac G5.

I came across this [project][3] by Kiwi, which converted an iMac G5 into a "Hackintosh" by reusing the display and power supply and replacing the system board with an X86 motherboard.

I decided to use a similar procedure to convert my iMac into a Raspberry Pi: the "PiMac".

![PiMac up and running][20]

## Procedure

The major parts of the conversion follow Kiwi's detailed [procedure][5]. However, certain steps are changed to accommodate the Raspberry Pi vs. X86 motherboard that Kiwi uses. My project also has a simpler SMB implementation, and uses what I believe is an easier method of converting the LVDS wires to an HDMI connector.

### HDMI Conversion

Adafruit has an HDMI [breakout board][6] which makes soldering the tiny LVDS video wires from the LCD much easier compared to the surface-mount connector in Kiwi's procedure. In addition, I used a separate breakout board for the power, ground, and VEDID (+5V from iMac power supply), and Hot Plug Detect circuitry.

![HDMI connector breakout][21]
![HDMI breakout board][22]

### Power Supply

As mentioned in Kiwi's procedure, it is easiest to use an ATX extension cable so that you don't need to cut the original cable coming from the Apple power supply. I used this [cable][7] -- similar cables are available from other vendors.

There are multiple power supply connections needed inside the case. I created a wiring harness from the ATX extension cable using various connectors from my spare parts bin.

![Power Supply Harness][23]

#### *Power Supply Harness Connections*

The wire colors listed below are specific to my cable harness, and can be used to trace the wires in the photo above. They are not necessarily the colors that your power supply or ATX extension cable use.

The iMac G5 uses an "ATX-style" connector and pinout. The pinout is not fully ATX compatible. The ATX pin numbers listed below assume a 22-pin connector. That is, pins 1-11 are on one side of the connector, and 12-22 are on the other side, where pin 12 is across from pin 1 and pin 11 is across from pin 22. **Note: Some of the pins on the iMac power supply do not match the function listed in the ATX spec -- the differences are noted below.**

Raspberry Pi Power (mini-USB cable) - ATX pins 4, 5:

- 5V power [pin 4]
- GND [pin 5]

Arduino Power and POWER_GOOD (3-pin JST) - ATX pins 7, 8, 9:

- Red: 5 V (always-on/Vsb power) [pin 9]
- Black: GND [pin 7]
- Yellow: POWER_GOOD signal from power supply (active high) [pin 8]

LCD Power (3-pin JST) - ATX pins 10, 11, 21:

- Red: 5 V [pin 21]
- Black: GND [pin 11 - **differs from ATX standard of +12V**]
- Yellow: 12 V [pin 10]

Inverter Power (4-pin standard Molex) - ATX pins 12, 16, 22:

- Red: 3.3 V [pin 12]
- Black: GND [pin 16]
- Yellow: 24 V [pin 22 - **differs from ATX standard of +5V**]

Power On (3-pin 0.1” mini-Molex) - ATX pins 14,15:

- Red: POWER_ON signal (active low turns on power supply outputs) [pin 15]
- Black: GND [pin 14]

AUX connector (4-pin 0.1" female header) - ATX pins 1, 6, 13, 17 (unused in current project):

- White: 3.3V [pin 1]
- Black: GND [pin 17]
- Red: 5 V [pin 6]
- Yellow: 12 V [pin 13 - differs from ATX standard of -12V]

AUX connector (2-pin 0.1” female mini Molex) - ATX pins 2, 3 (unused in current project):

- Red: 3.3V [pin 2]
- Black: GND [pin 3]

AUX connector (4-pin standard Molex) - ATX pins 18, 19, 20 (unused in current project):

- Red: 5 V [pin 20]
- Black: GND [pin 18]
- Yellow: 12 V [pin 19 - **differs from ATX standard of "reserved", formerly -5V**]

### iMac System Board

As in Kiwi's procedure, it is useful to keep a part of the iMac system board, particularly the section with the I/O ports, power connector, and front panel LED. Although my version of the system board was not quite the same as Kiwi's, I pretty much cut the board in the same way.

However, the circuit connections for the power button and front panel LED were different.

For the **power switch**, solder wires directly to pins 2 and 3 of the switch. When looking down at the top side of the PC board, the pins are numbered as follows:  

        1    2
    switch contacts
          3

For the **front panel LED**, solder wires directly to the SMD solder pads. Kiwi's procedure mentions using vias on the system board. However, my version of the system board did not have the same via connections. I therefore recommend just soldering directly to the LED pads.

Note that when putting the case back together, it is important to get the light tube (conical shaped plastic tube) mounted correctly between the system board and the iMac case so that the LED light is properly projected to the front of the case.

### System Management Board

I created a simpler System Management Board (SMB). It uses an Arduino Pro Mini (5V) to monitor the iMac power button, control the front panel LED, and turn on the power supply. It uses the "always-on" Vsb signal from the power supply and in turn controls the PS_ON signal to the power supply and monitors the Power_Good signal once the supply is turned on.

![SMB Schematic][24]
![SMB Wiring][25]

#### SMB Software

The SMB Arduino [software][10] implements a simple state machine to monitor and control the power button and power supply state. It also creates the "breathing" effect on the front panel LED while the PiMac is sleeping.

SMB state machine:  
![SMB State Machine Diagram][26]

## Putting It All Together

I mounted the circuit boards using existing mounting posts in the case, routing the cables and providing strain relief as needed.

I used short USB A-A extension cables to route the Raspberry Pi USB ports out the bottom of the iMac case. I also used a 40-pin flat ribbon cable to route the Raspberry Pi GPIO pins outside the case.

![Final Assembly][27]

## Note on Raspberry Pi Power

Since this design uses an ATX-like power supply to power the Raspberry Pi, there is a potential issue with the 5V supply to the Raspberry Pi board as discussed in this [article][13].

The iMac power supply puts out a solid 5.1V even under load. There is a slight voltage drop inherent in the USB micro cabling (even when using the shortest, low-gauge cable possible). Unfortunately, the power input circuitry on the Raspberry Pi 3 Model B introduces an additional drop of 0.3V at higher current levels. This causes the Raspbery Pi's undervoltage detection circuit to activate and the lightning bolt symbol will appear on the upper right corner of the display.

As mentioned in the [article][13], many Raspberry Pi accessory vendors sell power supplies that have nominal output voltages of 5.25V to overcome this issue, typically blaming the voltage drop on the USB cabling and connector. However, the Raspberry Pi power input circuit is the more likely culprit of the voltage drop than the cabling, particularly with the Pi 3 Model B.

## Parts

These are the parts used in my build. These are not set in stone -- alternatives can be used depending on what you may have available.

- [iMac G5][1]
  - Other models will work, with slightly different build procedures
- ATX power supply [extension cable][7]
- Adafruit [HDMI breakout][6]
- USB [extension cables][8]
- HDMI [cable][9]
- Arduino 5V [Pro Mini][11] or [Pro Mini][12]
  - Also available from other suppliers
- Raspberry Pi (various models available from numerous suppliers)
- NPN transistor: from parts bin
- Resistors: 4.7K, 1K, 230 Ohm: from parts bin
- 40-pin flat ribbon cable: from parts bin
- Internal power connectors: from parts bin
- Breakout boards: from parts bin

## References

- Kiwi's iMac to Hackintosh [article][3]
- Kiwi's detailed build [thread][5]
- Dremel Junkie [iMac G4][4]
- Raspberry Pi [Power Design Issue][13]

## License

The software and other files in this repository are released under what is commonly called the [MIT License][100]. See the file [`LICENSE`][101] in this repository.

[1]: https://everymac.com/systems/apple/imac/specs/imac_g5_2.0_20.html
[2]: https://everymac.com/systems/apple/imac/specs/imac_1.25_20_fp.html
[3]: https://tonymacx86.com/threads/kiwis-20-imac-i5.139633/
[4]: http://www.dremeljunkie.com/2011/11/all-in-one-20-imac-g4-genuine-tmds-to.html
[5]: https://www.tonymacx86.com/threads/kiwis-next-project-imac-g5.107859/
[6]: https://www.adafruit.com/product/3121
[7]: https://www.amazon.com/gp/product/B00EZ50W36/
[8]: https://www.amazon.com/gp/product/B00CJG2ZYM
[9]: https://www.amazon.com/gp/product/B00DI88XEG/
[10]: ./PiMac.ino
[11]: https://www.sparkfun.com/products/11113
[12]: https://www.adafruit.com/product/2378
[13]: http://wiki.loverpi.com/sbc:raspberry-pi-power-design-issue

[20]: jpg/PiMac.jpg
[21]: jpg/HDMI_Breakout.jpg
[22]: jpg/HDMI_Wiring.jpg
[23]: jpg/Power_Supply_Wiring_Harness.jpg
[24]: jpg/SMB_Schematic.jpg
[25]: jpg/SMB_Wiring.jpg
[26]: jpg/SMB_State_Machine.jpg
[27]: jpg/Final_Assembly.jpg

[100]: https://choosealicense.com/licenses/mit/
[101]: ./LICENSE
[200]: https://github.com/Andy4495/PiMac
