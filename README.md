
[![DOI](https://zenodo.org/badge/248134220.svg)](https://zenodo.org/badge/latestdoi/248134220)

# VentMon T0.4 - Inline Ventilator Test Fixture and Monitor

March 9th, 2021

![20210309_173230 (1)](https://user-images.githubusercontent.com/5296671/110490948-68778a80-80b6-11eb-8cbc-c9815958531f.jpg)

The photo above shows the current version of the VentMon, the T0.4 tester/monitor, designed and manufactured by Ben Coombs. We are giving these away free of charge to worthy open source teams.

The VentMon T0.4 improves on the T0.3 mostly in that the case is self-contained (removing cables) and is quite compact. We have also improved the OLED display, although additional firmware changes will be coming.

The software infrastructure that supports the VentMon, such as [VentDisplay](https://github.com/PubInv/vent-display) and [PIRDS](https://github.com/PubInv/PIRDS-respiration-data-standard), have been improving as well.

The VentMon is basically a cloud-enable spirometer with an FiO2 sensor. It can simply be plugged into a standard 22mm airway and provides
a rich stream of pressure, flow, and Oxygen sensor, with humidity and temperature sensing as well. The primary use of the VentMon is by 
engineering teams developing pandemic response ventilators and other respiration tools. Secondairly, it is completely and transparently 
open source, so any team can adopt the design and incorporate it into their own devices to provide the same data in an integrated way.

The parts in a VentMon cost about USD$600. 

# The Serial Port

VentMon pumps data in the [PIRDS data format](https://github.com/PubInv/PIRDS-respiration-data-standard) out on the Serial Port. This is partially human readable, and a good way to tell if the VentMon is working. It is not a good way to analyze data, but you can tell a lot by reviewing this data stream.  However, the preferred way to look at moving interactive waveforms and clinical paramters is with [VentDisplay](https://github.com/PubInv/vent-display), which requires some configuration.

# Configuration

Because we are developing a modular, composable, respiration ecosystem, configuring a VentMon T0.4 is a little complicated. The diagrams below try to capture three distince approahces: using our public data lake (easiest), using Apache (requires understanding httpd.conf and fully understanding all pieces), and a Docker Image (we have not actually created this, but we plan to.)

Public Data Lake:

![VentMon Cloud Ecosystem](https://user-images.githubusercontent.com/5296671/111052623-0c986300-8422-11eb-9d41-081f517026cd.png)

Using Apache:

![VentMon Apache Ecosystem](https://user-images.githubusercontent.com/5296671/111052629-1a4de880-8422-11eb-9466-349d80afdfa5.png)

Public Invention recently created a repo that builds a Docker file image that makes all of this possible locally without running Apache/CGI.
Please visit [PIRDS-docker-local](https://github.com/PubInv/PIRDS-docker-local) for more details.

![VentMon Docker Ecosystem](https://user-images.githubusercontent.com/5296671/111052632-25087d80-8422-11eb-8d85-7e72af57ff4f.png)




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

Although a work in progress, we have a [user manaul](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/doc/UserManaul.md).


# Hacking the Firmware

If you have received a VentMon from us,
it will have VentMonFirmware.ino installed.
You are welcome to modify the VentMonFirmware.ino file as needed. To do so, you will
have to install a library of our creation which encpasulates the PIRDS standard.
Copy the libary [pirds_library](https://github.com/PubInv/PIRDS-respiration-data-standard/tree/master/pirds_library) into your Arduino library location. You may need to install the Adafruit BME680 library
and other libraries as well.

Additionally, to compile the version of the firmware, you will need to install the [SFM3X00 library](https://github.com/PubInv/SFM3X00). We wrote this library to encapsulate Sensirion flow sensors of various kinds.

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

All code in this repo is licensed under the MIT License. All documents and diagrams and licensed under CC0. All hardware is licensed under the [CERN Open Hardware Licences Version 2 - Strongly Reciprocal](https://ohwr.org/cern_ohl_s_v2.pdf).
