


[![DOI](https://zenodo.org/badge/248134220.svg)](https://zenodo.org/badge/latestdoi/248134220)

Note: The previous version, the VentMon T0.3, has been published in [HardwareX](https://www.sciencedirect.com/science/article/pii/S2468067221000249) with extensive build instructions.

![IMG_4102 2](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/assets/5296671/b96dfc75-58e7-41bd-a88e-ea28e00b7dd8)


# VentMon T0.5 - Use without Needing Internet

The VentMon T0.5 is being roughly designed in June 2021. The version will self-contained---that is, it will not require a separate computer and internet access.
This version is especially motivated by the work of Mr. Joe Leier in Guatemala repairing ventilators. The idea is to have a self-contained unit
which allows for the tesitng and repair of units in the field.
It will, however, still be WiFi enabled, and still publish data to a the public data lake.

For now, our ideas and work are published in the directory called [pi](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/tree/master/pi).



# VentMon T0.4 - Inline Ventilator Test Fixture and Monitor

March 9th, 2021

![VentMonT0 4](https://user-images.githubusercontent.com/5296671/113649478-1e9da800-9654-11eb-934b-7e8a9a5c5bfe.JPG)

The photo above shows the current version of the VentMon, the T0.4 tester/monitor, designed and manufactured by Ben Coombs. We are giving these away free of charge to worthy open source teams.

The VentMon T0.4 improves on the T0.3 mostly in that the case is self-contained (removing cables) and is quite compact. We have also improved the OLED display, although additional firmware changes will be coming.

The software infrastructure that supports the VentMon, such as [VentDisplay](https://github.com/PubInv/vent-display) and [PIRDS](https://github.com/PubInv/PIRDS-respiration-data-standard), have been improving as well.

The VentMon is basically a cloud-enable spirometer with an FiO2 sensor. It can simply be plugged into a standard 22mm airway and provides
a rich stream of pressure, flow, and Oxygen sensor, with humidity and temperature sensing as well. The primary use of the VentMon is by 
engineering teams developing pandemic response ventilators and other respiration tools. Secondarily, it is completely and transparently 
open source, so any team can adopt the design and incorporate it into their own devices to provide the same data in an integrated way.

The parts in a VentMon cost about USD$600. 

# Usage

## Basics

The VentMon T0.4 can be powered with any micro-USB cable. It has an RJ45 ethernet port, and supports WiFi.  It has a male airway connector and 
a female airway connector in the international standard 22mm adult airway sizes. It comes with an oxygen sensor, which you may have to install
by opening the case. It is not pre-installed by the Oxygen sensors have a limited shelf life and some users don't need an oxygen sensor. The case can 
be opened with a small jeweler's Phillips head screwdriver.

The VentMon has a small OLED display which gives some startup information and then displays the momentary flow and pressure and produces a tiny 
graph of airway pressure. This is not intended for clinical use, but is useful for letting you know that the device is powered on and working.

## The Serial Port

The main targe for the VentMon is engineers working on ventilators and respiration support devices. Most such engineers will know
how to use the Arudino IDE to see the serial port. The baudrate is 500000 (500,000).
A simple human-readable output is printed to the serial port at start up time
showing some basic start up information.  After searching for the an ethernet connection and a WiFi connection, the VentMon will
begin printing dynamic data on the serial port. 
The output is in the [PIRDS data format](https://github.com/PubInv/PIRDS-respiration-data-standard). 
This is useful for verifying the devices is working physically correctly.
It is not a good way to analyze data, but you can tell a lot by reviewing this data stream.  However, the preferred way to look at moving interactive waveforms and clinical paramters is with [VentDisplay](https://github.com/PubInv/vent-display), which requires some configuration.


## Configuring a UDP conncetion

The easiest way to use VentDisplay is to use our public IoT Data Lake at [http://ventmon.coslabs.com/](http://ventmon.coslabs.com/). 
We name your log file by your ip address. The webpage there looks like this:
![Screen Shot 2021-03-25 at 8 38 57 PM](https://user-images.githubusercontent.com/5296671/112564937-22a30d80-8daa-11eb-8bb9-277bd82a6f5a.png)
These files are sorted by modification timestamp, so if you use one of the methods to place your data there, your data will likely be near the top!
The "BreathPlot" links take you to the VentDisplay for that log file, whether it is is static or "live" (that is, being dynamically added to from a
running VentMon.)

By entering a "c" character followed by a newline (probably in the Arduino IDE serial monitor tool), the VentMon will stop sending data 
to the serial port and write out a small menu of instructions. You can follow these instructions to enable or disable either the ethernet
connection, the WiFi connection, or both.  This menu allows you to enter your SSID for your WiFi network and your password, which will be
stored in the EEPROM of the device.

If you choose to use the Ethernet connection, you will probably plug an ethernet CAT5 cable directly into your router and the port on the VentMon.

Both Ethernet and WiFi attempt to send UDP packets to port 6111 at ventmon.coslabs.com.  There our [PIRDS-logger](https://github.com/PubInv/PIRDS-logger)
connection automatic timestamps and logs your UDP packets, and allows them to rendered for your viewing and analysis pleasure.

## Configuration

Because we are developing a modular, composable, respiration ecosystem, configuring a VentMon T0.4 is a little complicated. The diagrams below try to capture three distince approahces: using our public data lake (easiest), using Apache (requires understanding httpd.conf and fully understanding all pieces), and a Docker Image (we have not actually created this, but we plan to.)

Public Data Lake:

![VentMon Cloud Ecosystem](https://user-images.githubusercontent.com/5296671/111052623-0c986300-8422-11eb-9d41-081f517026cd.png)

Using Apache:

![VentMon Apache Ecosystem](https://user-images.githubusercontent.com/5296671/111052629-1a4de880-8422-11eb-9466-349d80afdfa5.png)

Public Invention recently created a repo that builds a Docker file image that makes all of this possible locally without running Apache/CGI.
Please visit [PIRDS-docker-local](https://github.com/PubInv/PIRDS-docker-local) for more details. A of March 25th, the Docker system is working 
on one computer (the authors), and another user has reported a problem with it; this is in an early rease stage.

![VentMon Docker Ecosystem](https://user-images.githubusercontent.com/5296671/111052632-25087d80-8422-11eb-8d85-7e72af57ff4f.png)


## Compiling and updating the Firmware

If you have received a VentMon from us,
it will have VentMonFirmware.ino installed.
The VentMon firmware is a work in progress, and we are making active improvements.
Therefore, you may want to update it from time to time. This requires you to build it with the Arduino IDE on your system.
You are welcome to modify the VentMonFirmware.ino file as needed. To do so, you will
have to install a library of our creation which encpasulates the PIRDS standard.
Copy the libary [pirds_library](https://github.com/PubInv/PIRDS-respiration-data-standard/tree/master/pirds_library) into your Arduino library location. You may need to install the Adafruit BME680 library
and other libraries as well.

Additionally, to compile the version of the firmware, you will need to install the [SFM3X00 library](https://github.com/PubInv/SFM3X00). We wrote this library to encapsulate Sensirion flow sensors of various kinds.

This process is a little clunky; please contact us if you need help with that.



# VentMon 0.3

Note: The important file "breath_plot.html" which reneders respiration traces and calculated values from PIRDS 
data was moved to its own [repo](https://github.com/PubInv/vent-display) on Thursday, June 18, 2020.

Since March 16th, I've been working full time on this. Along with volunteers at [EndCornaVirus.org](https://www.endcoronavirus.org/) and [Public Invention](https:\\www.pubinv.org), we have been
[tracking and analyzing](https://github.com/PubInv/covid19-vent-list)
all [open-source ventilator projects]((https://medium.com/@RobertLeeRead/analysis-of-open-source-covid-19-pandemic-ventilator-projects-27acf9075f7e)) we can find. It has become apparent that the biggest piece
they are missing and the place Public Invention can most help is not in designing a new ventilator,
but in building a test/monitoring fixture to assist other teams.

Because monitoring is critical to keeping patients alive and so similar to the need to test, we are attempting to
make a combine tester/monitor.

At the moment, this repo is for that work, and also our overall strategic plan for a process that will give clinicians
enough confidence to actually deploy an open-source ventilator. Our goal is to certify, in a certain limited, last-resort
sense, ventilators in the case of a shortfall of commercial and professional equipment. That Strategy is discussed lower in this document.

This includes rapidly doing high-quality tests, but also includes burn-in and multi-day tests, of
ventilators. Perhaps we can produce a benchmark for the [multiple efforts](https://github.com/PubInv/covid19-vent-list) going on right now to try to develop a
Free-libre Open Source ventilator design.

For further explanation of design motivation and approach please refer to the following document -
[Motivation of Strategy](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/Motivation.md)

For a hands on demonstration and in-depth explanation of VentMon watch our screen cast!  
[VentMon ScreenCast](https://www.youtube.com/watch?v=OV9MrMjVOCI)

# User Manual

Although a work in progress, we have a [user manaul v0.3T](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/doc/UserManaul.md).
This User Manual is basically for the VentMon.



## Design and Dependencies


![SystemDiagram](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/images/VentMonSystemDiagram.png)


The idea is to make a standalone inline device plugged into the airway. It serves a dual purpose as a monitor/alarm when used on an actual patient, and a test devices for testing prototype ventilators. It also allows for burnin.

VentMon depends on the following Arduino library for communication with Sensirion flow sensors: [SFM3X00](https://github.com/PubInv/SFM3x00)

Assembly instructions can be found here: [Assembly Instructions](https://docs.google.com/document/d/1n-Ja3UgbrCLcY8kx9rmTZB7b3yNJ_DnjpA1hwfMbgXk/edit#)

Initial BOM can be found here: [BOM](https://docs.google.com/document/d/1n-Ja3UgbrCLcY8kx9rmTZB7b3yNJ_DnjpA1hwfMbgXk/edit#)


## Status

** October 10th **

We have now shipped 20 units of the VentMons to open source teams all over the world. Some of them are
using them successfully. We order 15 printed circuit boards and 15 3D printed sensors ports and I built them
by hand. There was an error in the PCB which has since been corrected; on the current PCBs I fix it by 
soldering a wire in place. The current design of the case does not do cable management as well as we should; 
the flow meter cable tends to fall out. We will improve this in future desgins.

** August 20th **

We have received 15 populated boards and desigend a 3D printed hat. We are beginning final assembly/manufacture of the 15 units that people have requested to be sent out free-of-charge.

** June 18th ***

Lauria plans to ship 5 more VentMons on the 23rd. She has added FiO2 sensing, and important feature. Rob Giseburt has create an VentMon-like device using a Raspberry PI; we may switch to that display. The VentMon has been used internationally by several teams to gain important insights into their prototype pandemic ventilators. One team may be constructiong their own VentMon from our Open Source designs.

** May 25th **

We have now shipped 4 VentMons. Most recent modifications involved using the "pirds_library" more universally.

** May 7th **

We sent the first VentMon to Artemio Mendoza of the 1MillionVents team, and he made this great video:

[![VentMon testing an A.R.M.E.E. Device](https://user-images.githubusercontent.com/5296671/81298650-b0fb8d00-903a-11ea-9d14-260d6de92e55.png)](https://youtu.be/nViBK8d_pbM)


**May 4th:**

May the Fourth be with you.

We have shipped two VentMons free of charge to open source ventilators team that will use them for volume and flow measurements.

A [user manaul](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/doc/UserManaul.md) is progressing.


**April 27th:**

Assembly instructions can be found here: [Assembly Instructions](https://docs.google.com/document/d/1n-Ja3UgbrCLcY8kx9rmTZB7b3yNJ_DnjpA1hwfMbgXk/edit#)


**April 21st:**

Initial prototype is fully assembled!

![Prototype](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/images/prototype_0.JPG)

**April 20th:**

1) Have a working ESP32 board with pressure sensors and Sensirion flow sensors.
1) This code outputs a stream of PIRDS respiratory standard events on the serial port.
1) These are read by data_server.py which presents a web server for retrieving this data.
1) test_tools/breath_plot.html makes AJAX calls to this server and renders the results dynamically.


## Volunteering

This project is co-led by Robert L. Read <read.robert@gmail.com> and Geoff Mulligan.

Right now we have several needs. People need to be experienced, have read the [essential reading](https://github.com/PubInv/covid19-vent-list) list, be able to work at
least 20 hours a week, and able to work with limited supervision.  They also must be able to tolerate the chaos
produced by the pandemic itself, and the chaos produced by my own mistakes. Finally, they must accept and
hope for the possibility that industry will increase production and the disease will be controlled so that this
solution is not required.

Righ now, we could use:
1. An experienced Arduino engineer to build and improve the design below
1. A JavaScript programmer to add the ability to scroll-through all time to [https://github.com/PubInv/vent-diplay](vent-display) code.
1. Someone to design the User Interface of the "monitor and alarm" feature. Would have to be or become familiar with existing ventilators to make a similar interface. This is likely to be a small LCD or OLED text screen, 4 physical buttons, and a small speaker or buzzer.



# Request for Donations to Public Invention

[Public Invention](https://www.pubinv.org/) is a small, all-volunteer 501(c)3 US public charity. Your [small donation](https://www.pubinv.org/donate/) is a stamp of approval for our work that will really help us.


# Licensing

All code in this repo is licensed under the GPL License. All documents and diagrams and licensed under CC0. All hardware is licensed under the [CERN Open Hardware Licences Version 2 - Strongly Reciprocal](https://ohwr.org/cern_ohl_s_v2.pdf).
