# ventilator-test-lung
Design for test machine for rapidly doing high-quality tests, including burn-in and multi-day tests, of 
ventiators. The purpose is to produce a benchmark for the [multiple efforts](https://github.com/PubInv/covid19-vent-list) going on right now to try to develop a 
Free-libre Open Source ventilator design.

## Licensing

All code in this repo is licensed under the GNU public license. All documents and diagrams and licensed under CC0.

## Status

This is an emergeny project in response to the COVID-19 pandemic. We are in a learning mode.

## Motiavtion

As of March 18, 2020, the COVID-19 pandemic is growing quickly. The Maker, Humanitarian Enginering, and Free Sofware communities are attempting to answer the call. Public Invention is attempting to help the potential shortfall of ventilators, which is uncertain but could mean hundreds of thousands of needless deaths in a worst-case scenario.

Working with NECSI and [EndCoronavirus.org](https://www.endcoronavirus.org/), a self-organized project instigated by Bruce Fenton has arisen. It has become clear, however, that there are and should be many technical approaches.

In order to unify this community, some of us at the #response-ventilators-meta channel at the [necsi-edu.slack.com](necsi-edu.slack.com), include Nariman Poushin, have decided any practical response will have to focus on effectiveness and reliability testing, because ventilators are life-critical pieces of machinery. In effect, we need an informal certification process similar in spirit only to FDA approval, which can convince a doctor or medical decision maker to deploy unapproved ventilators as an absolute last resort if there is a shortage of ventilators.

The Free Software and Maker communities know how to form teams; our plan is to form teams for design, testing, training, building, and even fundraising. The basic process we propose is captured in the diagram below.

![Deployment Process](https://github.com/PubInv/ventilator-test-lung-analyzer/blob/master/Meta-process%20for%20Open%20Source%20Ventilator%20Deployment.svg)

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


