# Ventilator Test Fixture

Since March 16th, I've been working full time on this. Along with volunteers at [EndCornaVirus.org](https://www.endcoronavirus.org/) and [Public Invention](https:\\www.pubinv.org), we have been 
[tracking and analyzing](https://github.com/PubInv/covid19-vent-list)
all [open-source ventilator projects]((https://medium.com/@RobertLeeRead/analysis-of-open-source-covid-19-pandemic-ventilator-projects-27acf9075f7e)) we can find. It has become apparent that the biggest piece
they are missing and the place Public Invention can most help is not in desining a new ventilator,
but in building a test/monitoring fixture to assist other teams.

Because monitoring is critical to keeping patients alive and so similar to the need to test, we are attempting to
make a combine tester/monitor.

At the moment, this repo is for that work, and also our overall strategic plan for a process that will give clinicians
enough confidence to actually deploy an open-source ventilator. Our goal is to certify, in a certain limited, last-resort
sense, ventilators in the case of a shortfall of commercial and professional equipment. That Stretgy is discussed lower in this document.

This includes rapidly doing high-quality tests, but also includes burn-in and multi-day tests, of 
ventilators. Perhaps we can produce a benchmark for the [multiple efforts](https://github.com/PubInv/covid19-vent-list) going on right now to try to develop a 
Free-libre Open Source ventilator design.

## Volunteering

Righ now we have several needs. People need to be experienced, have read the [essential reading](https://github.com/PubInv/covid19-vent-list) list, be able to work at
least 20 hours a week, and able to work with limited supervision.  They also must be able to tolerate the chaos
produced by the pandemic itself, and the chaos produced by my own mistakes. Finally, they must accept and 
hope for the possibility that industry will increase production and the disease will be controlled so that this 
solution is not required.

Righ now, we could use:
1. An experienced Arduino engineer to build and improve the design below
1. Someone who knows how to reliably process a time series to tell when a breath begins and respiration rate
1. A website build who can make nice graphs of pressure/volume curves show up
1. Someone to build a software test-bench to analyze 48 hours worth of data and mark anomalies.

## Licensing

All code in this repo is licensed under the GNU public license. All documents and diagrams and licensed under CC0.

## Status

This is an emergeny project in response to the COVID-19 pandemic. We are in a learning mode.

## Initial Design Idea

![InlineVentMonitor](https://github.com/PubInv/ventilator-test-lung-analyzer/blob/master/InlineVentMonitor.svg)



# Motiavtion of Strategy

As of March 18, 2020, the COVID-19 pandemic is growing quickly. The Maker, Humanitarian Enginering, and Free Sofware communities are attempting to answer the call. Public Invention is attempting to help the potential shortfall of ventilators, which is uncertain but could mean hundreds of thousands of needless deaths in a worst-case scenario.

Working with NECSI and [EndCoronavirus.org](https://www.endcoronavirus.org/), a self-organized project instigated by Bruce Fenton has arisen. It has become clear, however, that there are and should be many technical approaches.

In order to unify this community, some of us at the #response-ventilators-meta channel at the [necsi-edu.slack.com](necsi-edu.slack.com), include Nariman Poushin, have decided any practical response will have to focus on clinical effectiveness and reliability testing, because ventilators are life-critical pieces of machinery. In effect, we need an informal certification process similar in spirit only to FDA approval, which can convince a doctor or medical decision maker to deploy unapproved ventilators as an absolute last resort if there is a shortage of ventilators.

The Free Software and Maker communities know how to form teams; our plan is to form teams for design, testing, training, building, and even fundraising. The basic process we propose is captured in the diagram below.

![Deployment Process](https://github.com/PubInv/ventilator-test-lung-analyzer/blob/master/FLOSS%20Build%20Model.png)



## Necessity of Extensive Testing

To deploy a ventilator, extensive testing is *absolutely* required. This repo is an attempt to build a "test fixture" capable of running a 48-hour test on any ventilator design and collecting data on many important parameters.  We hope to create a "gold standard" test that all DIY teams can work to; but this project will proceed in parallel with that.

## Physical Test Lungs

I believe that a test lung is meant to provide a resistance to inflation which models a human lung.  An example of a purchasble device is:

[Allied Healthcare AHP300 Mechanical Test Lung](https://mfimedical.com/products/allied-healthcare-ahp300-mechanical-test-lung?variant=1189831180302&gclid=Cj0KCQjwjcfzBRCHARIsAO-1_Or1bEKy4YffCthJD3sTJYzZy1JlM-ttMZ_1h6WHfgKte8mN4AA_c_AaAkGCEALw_wcB)

[5cm H20/L/sec. Resistance Test Lung](https://www.grainger.com/product/33JV39?gclid=Cj0KCQjwjcfzBRCHARIsAO-1_Oqb0ML6Rgr4FCKePVsaIVJeHSpm4-jq89cLWnL1YsjuSJcpCzHZFsYaAnoAEALw_wcB&cm_mmc=PPC:+Google+PLA&ef_id=Cj0KCQjwjcfzBRCHARIsAO-1_Oqb0ML6Rgr4FCKePVsaIVJeHSpm4-jq89cLWnL1YsjuSJcpCzHZFsYaAnoAEALw_wcB:G:s&s_kwcid=AL!2966!3!281698276014!!!g!471328313928!)

Note the units of resistance here: Pressure/volume/sec. Presumable a test that used open air would not accurate model a ventilator.


## Certifiers

An example of a certifier, including an explanation of its functionality is here: [Alnor-4070 Certifier  Flow Analyzer](https://www.globaltestsupply.com/product/alnor-4070-certifier-flow-analyzer-system?gclid=Cj0KCQjwjcfzBRCHARIsAO-1_OpV87HtnO7sJXdt92m7W3j69KOddmV-gV0eCZjfKlHTDMWsUCqYe4kaAoaQEALw_wcB)

However, we also need to do multi-iteration reliabillity tests.  An initial design is here:

![initial design](https://github.com/PubInv/ventilator-test-lung/blob/master/VentilatorTestFixture.svg)

## Older diagram


![Deployment Process](https://github.com/PubInv/ventilator-test-lung-analyzer/blob/master/Meta-process%20for%20Open%20Source%20Ventilator%20Deployment.svg)



