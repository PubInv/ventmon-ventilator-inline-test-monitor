# VentMon T0.1 User Manaul

This is the rapidly-evolving VentMon by Public Invention. This is for version T0.1 -- "Tester v. 0.1", which is aimed at
engineering teams needing to test rapidly manufactured pandemic ventilators.

# Introduction

The VentMon is a physical device designed to help the open-source community develop confidence in
rapidly manufactured ventilators systems. At present it serves a tester to be used on
the workbench of teams developing ventilators.  Eventually, we hope it will serve
the important function of providing a monitoring/alarming system as a module reusable
with in other designs, or as part of an emergent open design for ventilations systems.

# Basic Functionality

The VentMon plugs into an airway and measure flow and absolute and differential pressure within the
airway approximately 25 times a second. It measures oxygen, humidity, and temperature within
the airway and in the ambient air approximately once a minute.


The basic test function is to use the VentMon to log a data file over a period of time.
This log file may be viewed live (with imperceptible delay) via our software tools, allowing
an engineer to immediately observe pressures and flows. This same software computes the
clinically important parameters such as tidal volume, minute volume, and respiration rate,
and I:E ratio. If produced over a long period of time, this would be evidence that the
ventilator is performing as expected.

# Physical Connection


![Physical Configuration](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/doc/VentMon%20Overall%20Architecture.png)

The VentMon has up to 4 physical connections:

1. The air input, which is a female 22mm inner diameter airway tube. This is an international standard. The input to the VentMon comes from the ventilator or air-drive you are testing.
1. The air output, which is a standard male 22mm outer diameter port. This should be connected
to the test lung/patient.
1. A USB connection to provide 5V power to the VentMon. Additionally, the VentMon streams
[PIRDS](https://github.com/PubInv/respiration-data-standard) data on the serial port, which is useful for testing and checks on verification.
It requires additional software, however, to produce an understandable picture of ventilator
performance.
1. An ethernet connection which is used by the VentMon to stream PIRDS data via UDP.
You are welcome to change the firmware, but our basic firmware streams data to our
public data lake on port 6111 and vetmon.coslabs.com.

# Recommended Unboxing Testing

To familiarize yourself with the VentMon and "smoke test" basic operation, we recommend
the following steps:

1. Before connecting to a ventilator or a test long, simply connect the VentMon to an
Aduino IDE and start the Serial Monitor. Do not change the by uploading new code
unless you intend to so ao. The current firmward outputs 115200 baud; set your Arduino
IDE to listen at that baud rate.
1. Observe that the VentMon is streaming a series of JSON objects conformant to the
[PIRDS](https://github.com/PubInv/respiration-data-standard) data standard.

If you cannot observe PIRDS data streaming, please contact us for support.

1. Now check that flow is measured by simpling blowing into the ventmon airway tube
or using a sanitary dry source of air such as a small fan.
1. The PIRDS type code for flow is "F" and the flow sensor the sensor of type "F" named
"A" and numbered "0". A PIRDS flow event looks like:
> { "event" : "M", "type" : "F", "ms" : 12103, "loc" : "A", "num" : 0, "val" : 0  }.
1. The units of flow according to the [PIRDS standard](https://github.com/PubInv/respiration-data-standard/blob/master/PIRDS-v.0.1.md) are standard liters per minute times 1000 (or milliliters pers minute.)  You should be able to create flow of a few milliliters per minute just by waving your hand quickly at the VentMon input; a fan will produce a much higher flow.
1. Observe that you can create a measurable flow.

If you cannot observer a measurable flow, please contact us for support.

Finally, observe that the pressure differential is approximately correct.

1. The VentMon pressents pressure in the airway in absoltue terms, but more useful
is the differential pressure, which looks like this:
>  { "event" : "M", "type" : "D", "ms" : 12037, "loc" : "A", "num" : 0, "val" : -11  }
The VentMon uses two independent BME680 sensors to compute the pressure differential.
The ambient pressure is kept as a running average over a period time. The instantaneous
pressure in the airway is published as an event aobut 25 times a second, as is the
difference withe ambient air.
1. Observe that the pressure in the Aiway is of type "P" and nubmered "A" and "0".
The amBient pressure is "B" and "0". Following medical practice, these absolute
pressures are measured in cm H2O. The VentMon PIRDS standard defines the units
reported for pressure to be hundredths of cm of H2O.
1. Therefore, at close to sea level, the "val" field for these absolute pressures should be close
to 10000. The amBient pressure is reported very rarely compared to the Airway pressure; it may
be challenging to find the "B" field in the serial output.
1. Differential pressure (when not in an active breathing circuit) should be at most tens (less than one cm H2O). This represents
miscalibration in our two sensors; we do not believe this inaccuracy is clinically significant
as long as it is small.

If the pressure sensors do not seem to be working, contact us for support.






# Software Usage

Our software, however, can be used with any system that produces data in the
[Public Invention Respiratory Data Standard](https://github.com/PubInv/respiration-data-standard/blob/master/PIRDS-v.0.1.md).

# License

The VentMon is both open hardware and open software. The firmware that runs on the board and
makes sensor measurements is reduced under the [MIT License](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/LICENSE). Other libraries and files may be
released under different open source license if they are derived works of starting points
released under other licenses.

Public Invention does not apply for patents. All of the hardware designs for the VentMon
are open. The particular design documents and drawings are released under Creative Commons
licenses as indicated on the document, generally either CC0 or CC-BY-SA.

# Obtaining

The VentMon is not a commercial product; it is given away free of charge
by Public Invention to teams addressing
the COVID-19 pandemic with the generous support of the [Mozilla Opens Source Support Foundation
(MOSS)](https://www.mozilla.org/en-US/moss/) and [Protocol Labs](https://protocol.ai/).

However, the parts in the VentMon are expensive, and we may not be able to meet demand.
To request a VentMon please contact us with a description of your need and a link to
your project.

# Technical Support

For technical support, please contact <read.robert@gmail.com> by email.

# Firmware and Software Updates

As of May 1st, 2020, the VentMon is a rapidly changing system which can be
expected to have evolving firmware. Please check the [GitHub repo](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor) often for firmware updates.

# Return to Us If You Don't Use

The VentMon is offered free-of-charge; however, if you decide to stop using it, please
contact us about shipping it back to us. There may be others who want one, and the
COVID-19 pandemic has created supply chain shortages.

# Constructing Your Own

You may build your own VentMon based on our [instructions](https://docs.google.com/document/d/1n-Ja3UgbrCLcY8kx9rmTZB7b3yNJ_DnjpA1hwfMbgXk/edit#heading=h.u1fih96r90b6)
or make modifications and improvements to this design.
If you make improvements, we hope you will submit them back to use under and open-source license
so that we may publish them to the whole community.

# Using the Test Tools Without a VentMon

# Cleaning and Physical Connections
