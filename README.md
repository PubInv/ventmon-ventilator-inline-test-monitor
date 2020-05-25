# VentMon 0.1 - Inline Ventilator Test Fixture and Monitor

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

# User Manual

Although a work in progress, we have a [user manaul](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/doc/UserManaul.md).


# Hacking the Firmware

If you have received a VentMon from us,
it will have VentMonFirmware.ino installed.
You are welcome to modify the VentMonFirmware.ino file as needed. To do so, you will
have to install a library of our creation which encpasulates the PIRDS standard.
Copy the libary [pirds_library](https://github.com/PubInv/PIRDS-respiration-data-standard/tree/master/pirds_library) into your Arudino library location. You may need to install the Adafruit BME680 library
and other libaries as well.


## Design and Dependencies


![SystemDiagram](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/images/VentMonSystemDiagram.png)


The idea is to make a standalone inline device plugged into the airway. It serves a dual purpose as a monitor/alarm when used on an actual patient, and a test devices for testing prototype ventilators. It also allows for burnin.

VentMon depends on the following Arduino library for communication with Sensirion flow sensors: [SFM3X00](https://github.com/PubInv/SFM3x00)

Assembly instructions can be found here: [Assembly Instructions](https://docs.google.com/document/d/1n-Ja3UgbrCLcY8kx9rmTZB7b3yNJ_DnjpA1hwfMbgXk/edit#)

Initial BOM can be found here: [BOM](https://docs.google.com/document/d/1n-Ja3UgbrCLcY8kx9rmTZB7b3yNJ_DnjpA1hwfMbgXk/edit#)


## Status

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
1. Someone who knows how to reliably process a time series to tell when a breath begins and respiration rate
1. A website build who can make nice graphs of pressure/volume curves show up
1. Someone to build a software test-bench to analyze 48 hours worth of data and mark anomalies.
1. Someone to compare our Javascript analys algorithms to [BreathMetrics](https://github.com/zelanolab/breathmetrics) in MatLab.
1. Someone to design the User Interface of the "monitor and alarm" feature. Would have to be or become familiar with existing ventilators to make a similar interface. This is likely to be a small LCD or OLED text screen, 4 physical buttons, and a small speaker or buzzer.



# Request for Donations to Public Invention

[Public Invention](https://www.pubinv.org/) is a small, all-volunteer 501(c)3 US public charity. Your [small donation](https://www.pubinv.org/donate/) is a stamp of approval for our work that will really help us.


# Licensing

All code in this repo is licensed under the MIT License. All documents and diagrams and licensed under CC0.
