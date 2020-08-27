// Number of hose inputs - which is input and which is output doesn't matter but the input will be on the bottom of the print so should be the larger of the two
inputs = 1;
// Number of hose outputs
outputs = 1;
//  Measurement system for inputs - US in inches, Metric in mm
input_units = "US"; // [US,Metric]
// Measurement system for outputs
output_units = "US"; // [US,Metric]
// Input tubing inside diameter, e.g. 0.5 for 1/2 inch (US); 0.625 for 5/8, 0.75 for 3/4, etc
input_size = 0.25;
// Output tubing inside diameter, e.g. 0.17 for 1/4 inch OD (outside diameter) vinyl tubing (US); 0.25 for 1/4 inch ID (inside diameter) (US); 0.375 for 3/8 inch ID
output_size = 0.25;
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
strength = 3.0;
// Number of barbs on input - use 3 for larger tubing to reduce overall print height
input_barb_count = 3;
// Number of barbs on output - use 4 for smaller tubing
output_barb_count = 3;

/* [Hidden] */
inches_to_mm = 25.400;

make_adapter( input_units == "US" ? input_size * inches_to_mm : input_size, output_units == "US" ? output_size * inches_to_mm : output_size );

module make_adapter( input_diameter, output_diameter )
{

if (connector_type == "inline" && inputs == 1 && outputs == 1)
{
  assign ( junction_height = input_barb_count )
  {
    union() {
      input_barb( input_diameter );
      junction( input_diameter, output_diameter, junction_height );
      output_barb( input_diameter, output_diameter, junction_height );
    }
  }
}
if (connector_type == "vault" || inputs > 1 || outputs > 1)
{
  assign( center_width = max(inputs * input_diameter * vault_connector_ratio, outputs * output_diameter * vault_connector_ratio), center_height = max(input_diameter * vault_connector_ratio, output_diameter * vault_connector_ratio), junction_height = input_barb_count )
  {
   assign( center_empty_input = center_width - input_diameter * vault_connector_ratio * inputs, center_empty_output = center_width - output_diameter * vault_connector_ratio * outputs )
   {
     union() {
       //input_barb( input_diameter );
       for (i = [1:inputs]) 
	    {
		   translate( [center_empty_input / 2 + input_diameter * vault_connector_ratio / 2 + input_diameter * (i-1) * vault_connector_ratio,0,0] ) input_barb( input_diameter );
	    }
		 //echo( "inputs=", inputs, "; input_diameter=", input_diameter, "; outputs=", outputs, "; output_diameter=", output_diameter );
		 //echo( "center_width=", center_width, "; by input=", inputs * input_diameter * vault_connector_ratio, "; by output=", outputs * output_diameter * vault_connector_ratio, "; center_empty_input=", center_empty_input, "; center_empty_output=", center_empty_output );
		 difference()
		 {
			translate([-vault_wall_thickness,-vault_wall_thickness,0]) vault( input_diameter, junction_height, center_width, center_height, center_height, vault_wall_thickness );
			for (i = [1:inputs]) 
			{
				// Make holes in the vault for input entry
      		translate( [center_empty_input / 2 + input_diameter * vault_connector_ratio / 2 + input_diameter * (i-1) * vault_connector_ratio,0,input_diameter * input_barb_count * 0.9 - center_height / 3] ) cylinder( r=input_diameter * 0.36, h=center_height );
			}
			for (i = [1:outputs]) 
			{
				// Make holes in the vault for output egress
      		translate( [center_empty_output / 2 + output_diameter * vault_connector_ratio / 2 + output_diameter * (i-1) * vault_connector_ratio,0,input_diameter * input_barb_count * 0.9 + center_height / 2] ) cylinder( r=output_diameter * 0.36, h=center_height );
			}
		 } // end vault with holes
       //output_barb( input_diameter, output_diameter, 12 );
	    for (i = [1:outputs]) 
		 {
			translate( [center_empty_output / 2 + output_diameter * vault_connector_ratio / 2 + output_diameter * (i-1) * vault_connector_ratio,0,-output_diameter * 0.3] ) output_barb( input_diameter, output_diameter, center_height + 2 * vault_wall_thickness );
		 }
     }
	}
  }
}
}

/****
module tometric( unit_type, value )
{
  if (unit_type == "US") value * 25.4;
  else value;
}
***/

module barbnotch( inside_diameter )
{
  // Generate a single barb notch
  cylinder( h = inside_diameter * 1.0, r1 = inside_diameter * 0.85 / 2, r2 = inside_diameter * 1.16 / 2, $fa = 0.1, $fs = 0.1 );
}

module solidbarbstack( inside_diameter, count )
{
    union() {
      barbnotch( inside_diameter );
		for (i=[2:count]) 
		{
			translate([0,0,(i-1) * inside_diameter * 0.9]) barbnotch( inside_diameter );
		}
		/***
		if (count > 1) translate([0,0,1 * inside_diameter * 0.9]) barbnotch( inside_diameter );
		if (count > 2) translate([0,0,2 * inside_diameter * 0.9]) barbnotch( inside_diameter );
		***/
    }
}

module barb( inside_diameter, count )
{
  // Generate 4 barbs
  difference() {
    solidbarbstack( inside_diameter, count );
      // WARNING: Changing this....
//    translate([0,0,-0.3]) cylinder( h = inside_diameter * (count + 1), r = inside_diameter * 0.75 / 2, $fa = 0.5, $fs = 0.5 );
      translate([0,0,-0.3]) cylinder( h = inside_diameter * (count + 1), r = inside_diameter * 0.6 / 2, $fa = 0.1, $fs = 0.1 );
  }
}

module input_barb( input_diameter )
{
  barb( input_diameter, input_barb_count );
}

module output_barb( input_diameter, output_diameter, jheight )
{
  translate( [0,0,input_diameter * input_barb_count * 0.9 + output_diameter * output_barb_count * 0.9 + jheight + 1] ) rotate([0,180,0]) barb( output_diameter, output_barb_count );
}

module junction( input_diameter, output_diameter, jheight )
{
    holeD=input_diameter >= output_diameter ? output_diameter : input_diameter;
  junction_diameter_ratio = (inline_junction_type == "none") ? 1.1 : 1.6;
  translate( [0,0,input_diameter * jheight * 0.9] ) difference() {
	cylinder( r = max(input_diameter, output_diameter) * junction_diameter_ratio / 2, h = 5, $fa = 0.1, $fs = 0.1 );
	cylinder( r1 = input_diameter / 2, r2 = output_diameter / 2, h = (jheight + 1), $fa = 0.1, $fs = 0.1 );
//    cylinder(r=holeD/2, h=jheight*2*5, center=true);
      // WARNING!!! THIS HAS BEEN CHANGED for my specific situation
    cylinder(r=holeD/4, h=jheight*2*5, center=true,  $fn = 30);
  }
}

module vault( input_diameter, jheight, center_width, center_depth, center_vheight, wall_thickness )
{
  assign( outside_width = center_width + 2 * wall_thickness, outside_depth = center_depth + 2 * wall_thickness, outside_vheight = center_vheight + 2 * wall_thickness, vault_base = input_diameter * jheight * 0.9 )
  {
    translate( [0, -outside_depth / 2 + wall_thickness, vault_base - wall_thickness] ) union()
	 {
	   difference() 
      {
        cube( [outside_width,outside_depth,outside_vheight] );
        translate( [wall_thickness,wall_thickness,wall_thickness] ) cube( [center_width, center_depth, center_vheight] );
      }
	   // Add supports if requested
		if (vault_support == "yes")
	   {
		  assign( support_leg_width = center_width / 4, support_leg_thickness = wall_thickness / 2 )
		  {
		  translate([outside_width-support_leg_width,outside_depth-support_leg_thickness,wall_thickness-vault_base]) cube([support_leg_width, support_leg_thickness, vault_base]);
		  translate([outside_width-support_leg_width,0, wall_thickness-vault_base]) cube([support_leg_width, center_depth + wall_thickness*2, 0.6]);
		  translate([0,outside_depth-support_leg_thickness,wall_thickness-vault_base]) cube([support_leg_width,support_leg_thickness, vault_base]);
		  translate([outside_width-support_leg_width,0,wall_thickness-vault_base]) cube([support_leg_width,support_leg_thickness, vault_base]);
		  translate([0,0, wall_thickness-vault_base]) cube([support_leg_width, center_depth + wall_thickness*2, 0.6]);

		  translate([0,0,wall_thickness-vault_base]) cube([support_leg_width, support_leg_thickness, vault_base]);
		  }
		}
    }
  }
}
