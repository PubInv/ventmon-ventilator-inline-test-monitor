// This file was origininally written by Stephen Glow and has been
// extensively modified by Robert L. Read.

use <BarbGenerator-v3.scad>;

// This dile describes the geometry of the freeflow venturi tube.
// I have tried to document the various parameters, most of which can be
// changed to adjust tube geometry.
//
// These are just openscad special variables related to rendering.
// They define how many fascets in an arc.  I have them set for pretty high 
// resolution (slow) rendering.  Making these bigger produces faster but 
// rougher renderings
$fa = 1;      // Minimum angle fragment
$fs = .1;     // Minimum size of a fragment

// M for margin.  I use this to make things overlap slightly for better rendering
M = .01;

// These are the X axis offsets of the two pressure sensors from the end of the pcb.
// The sensors are both centered in the Y axis, so I don't include that here.
// These values are correct for the rev-2 freeflow board.
pcb_tap_outer  = in2mm(0.3);
pcb_tap_center = in2mm(1.6);

// Tubing that connects to the venturi outer & inner diameter
tube_od = 22;
tube_id = 18;

// Amount that my venturi will overlap the tube on the enterence.
// I'm using a female enterence and male exit
tube_overlap = 10;

// Thickness of the wall of the overlapping section
overlap_thickness = 1.25;

// The outer diameter of the venturi tube enterence will be the 
// same as the tube_id.  The throat diameter is the inner diameter
// of the constricted throat of the venturi.  Smaller gives larger
// readings, especially at low flow rates, but also causes more
// pressure drop through the tube.
throat_d = tube_id * 0.4;
echo( "throat_d ", throat_d );

// These angles define how quickly the venturi will narrow on 
// both entry and exit.  Angles are relative to the center line
// running through the tube.  These angles are pretty typical for
// venturi tubes according to my research.
angle_entry = 12;
angle_exit = 7;

// The exit of the venturi will have the same inner/outer diameter 
// as the tube that attaches to the entry.  I'm assuming that a fitting
// sized for that tube will be connected to the exit end
exit_id = tube_id;
exit_od = tube_od;

// Find the entry and exit distance based on the change of diameter
// and the angles.  
Lnarrow = (tube_id - throat_d)/2 / tan(angle_entry);
Lexpand  = (exit_id - throat_d)/2 / tan(angle_exit);

// This is the length of the exit section.  The fitting attached to the 
// end of the venturi will overlap by up to this much
exit_overlap = 10;

// The length of the throat section is customarily the same as the
// throat diameter
// Hack:
// Lthroat = throat_d;
Lthroat = 0;

// The length of the enterence to the venturi (before it starts to narrow)
// The outer tap is located in this area, so this is somewhat dictated by the PCB dimensions

pcb_dtap = pcb_tap_center - pcb_tap_outer;

Lentry = pcb_dtap - Lnarrow - Lthroat/2 + tube_id/2;

// There will be two holes in the side of the tube where we will sense
// pressure.  This is the diameter of those holes
tap_d = 2.7;

module CouplerWTap() {
    difference() {
        Coupler();
        TapHole();
    }
}



// This module is the venturi tube itself without the board mount
module Coupler()
{
   // This is the Z offset of the bottom of the PCB relative to the bottom of the tube
   pcb_zoff = tube_overlap+Lentry+Lnarrow+Lthroat/2-pcb_tap_center;

   // OK, build the venturi tube.
   difference()
   {
      // This section is the outer shell of the venturi tube.
      union()
      {

         // This first section is the area in which the venturi tube overlaps
         // the tubing that connects to it.
         cylinder( d=tube_od+2*overlap_thickness, h=tube_overlap );

          L = Lentry/2;
         translate( [0,0,tube_overlap-M] )
            cylinder( d1=tube_od+2*overlap_thickness, d2=exit_od, h=L+M );

         // This last section is the outer part of the section which will fit
         // into the fitting 
         translate( [0,0,tube_overlap+L-M] )
            cylinder( d=exit_od, h=exit_overlap+M );
      }

      // Everything after here is removed from that outer shell.

      // This is the area in which the external tube will enter
      translate( [0,0,-M] ) 
         cylinder( d=tube_od, h=tube_overlap+M );

      // This is the outer section of the venturi.
      // The outer tap will end up in the center of this section.
      translate( [0,0,tube_overlap-M] ) 
         cylinder( d=tube_id, h=Lentry+M );

      // This is the narrowing entry part
      translate( [0,0,tube_overlap+Lentry-M] ) 
         cylinder( d1=tube_id, d2=tube_id, h=Lnarrow+M );

   }
}

module TapHole() 
{  
   //pcb_zoff = tube_overlap+Lentry+Lnarrow+Lthroat/2-pcb_tap_center;
          // Add a hole for the outer pressure sensor tap
    L = Lentry/2;
    z = tube_overlap +L/2;
      translate( [0,0,z] )
         rotate( [0,90,0] ) cylinder( d=tap_d, h=tube_od );

}

function in2mm(x) = x*25.4;

CouplerWTap();


/* [Hidden] */
inches_to_mm = 25.400;

// Input tubing inside diameter, e.g. 0.5 for 1/2 inch (US); 0.625 for 5/8, 0.75 for 3/4, etc
input_size = 3/16;
// Output tubing inside diameter, e.g. 0.17 for 1/4 inch OD (outside diameter) vinyl tubing (US); 0.25 for 1/4 inch ID (inside diameter) (US); 0.375 for 3/8 inch ID
output_size = input_size;

output_size_mm = output_size*25.4;

//p_input_size = 0.06889764;
//p_output_size = 0.06889764;

p_input_size = 0.06889764;
p_output_size = p_input_size;

p_output_size_mm = p_output_size*25.4;

barbLength = 7;
barbHeight = 7;
outerBarbRadius = 1.93;
innerBarbRadius = outerBarbRadius - 1;

// Number of hose inputs - which is input and which is output doesn't matter but the input will be on the bottom of the print so should be the larger of the two
inputs = 1;
// Number of hose outputs
outputs = 1;
//  Measurement system for inputs - US in inches, Metric in mm
input_units = "US"; // [US,Metric]
// Measurement system for outputs
output_units = "US"; // [US,Metric]
// Input tubing inside diameter, e.g. 0.5 for 1/2 inch (US); 0.625 for 5/8, 0.75 for 3/4, etc
input_size = 3/16;
// Output tubing inside diameter, e.g. 0.17 for 1/4 inch OD (outside diameter) vinyl tubing (US); 0.25 for 1/4 inch ID (inside diameter) (US); 0.375 for 3/8 inch ID
output_size = 3/16;
// Connector type - Use inline for a single input and single output
connector_type = "inline"; // [inline,vault]
// Inline junction type - currently only round is supported
inline_junction_type = "round"; // [round,hex,none]

translate([tube_od+2.5,0,14.1])
rotate([0,270,0])
input_barb( input_units == "US" ? input_size * inches_to_mm : input_size );
    
