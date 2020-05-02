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

![Physical Configuration](https://github.com/PubInv/ventmon-ventilator-inline-test-monitor/blob/master/doc/VentMon%20Overall%20Architecture.png)

The basic test function is to use the VentMon to log a data file over a period of time.
This log file may be viewed live (with imperceptible delay) via our software tools, allowing
an engineer to immediately observe pressures and flows. This same software computes the
clinically important parameters such as tidal volume, minute volume, and respiration rate,
and I:E ratio. If produced over a long period of time, this would be evidence that the
ventilator is performing as expected.

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
