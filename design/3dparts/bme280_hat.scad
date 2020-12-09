use <BarbGenerator-v3.scad>;
use <PressureCoupling.scad>;

outerHeight = 15;
innerHeight = 12;
outerWidth = 7.5;
innerWidth = 4.5;
outerLength = 10.5;
innerLength = 6.5;

// $fn = 30;



barbLength = 7;
barbHeight = 7;
outerBarbRadius = 1.93;
innerBarbRadius = outerBarbRadius - 1;

pegRadius = 1;
pegLength = 2;
pegOffsetX = 0;
// pegOffsetY = 3.25;
pegOffsetY = 4.25;
pegOffsetZ = -pegLength;

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


// raise the upper port a bit more 
raise_upper_port = 1;

module box()
{
    cylinder_hole_fudge = 0.6;
    union(){
        difference(){

            translate([0,0,outerHeight/2]){
                cube([outerWidth, outerLength, outerHeight], center=true);
            }
            translate([0,0,outerHeight/2 -5]) {
                cube([innerWidth, innerLength, innerHeight+10], center=true);
            }
            translate([0,0,raise_upper_port+3*outerHeight/4]) 
            {
                rotate([90,270,0])
                cylinder(barbLength, 
                    cylinder_hole_fudge*p_output_size_mm/2, 
                    cylinder_hole_fudge*p_output_size_mm/2, center=false);
            }
            translate([0,0,outerHeight/4]) 
            {
                rotate([90,279,0])
                cylinder(barbLength, 
                    cylinder_hole_fudge*output_size_mm/2, 
                    cylinder_hole_fudge*output_size_mm/2, center=false);
            }
        }

// pegs
        translate([pegOffsetX, -pegOffsetY, pegOffsetZ]){
            cylinder(pegLength, pegRadius, pegRadius, center=false);
        }
        translate([pegOffsetX, pegOffsetY, pegOffsetZ]){
            cylinder(pegLength, pegRadius, pegRadius, center=false);
        }
    }
}


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


/* [Manifold] */

// Vault wall thickness in mm - increase for higher pressure connections
vault_wall_thickness = 3.0;
// Add support under vault for printing
vault_support = "yes"; // [yes,no]
// Ratio of connector diameter to space allocated in vault - increase to allow more space for hose clamps, etc.
vault_connector_ratio = 1.8;

/* [Advanced] */

// Width of junction as ratio of tubing I.D.
inline_junction_width = 0.8;
// Strength factor - use more than 1 for thicker walls to handle higher pressure and stronger clamping
strength = 2.0;
// Number of barbs on input - use 3 for larger tubing to reduce overall print height
input_barb_count = 3;
// Number of barbs on output - use 4 for smaller tubing
output_barb_count = 4;




module simple_adapter( input_diameter, output_diameter )
{
  assign ( junction_height = input_barb_count )
  {
    union() {

      input_barb( input_diameter );
//      junction( input_diameter, output_diameter+1, 0 );
//     output_barb( input_diameter, output_diameter, junction_height );
    }
  }
}

module hat_with_barb () {



// barb();


translate([barbLength+outerLength+1,0,outerHeight/4])
rotate([0,270,0])
simple_adapter( input_units == "US" ? input_size * inches_to_mm : input_size, 
              output_units == "US" ? output_size * inches_to_mm : output_size );
    
translate([barbLength+outerLength-7.4,0,raise_upper_port+ (3*outerHeight/4)])
rotate([0,270,0])
simple_adapter( input_units == "US" ? p_input_size * inches_to_mm : p_input_size, 
              output_units == "US" ? p_output_size * inches_to_mm : p_output_size );
    
    // box
rotate([0,0,90])
box();
    
}


translate([-19,0,-10])
hat_with_barb();

echo(version=version());