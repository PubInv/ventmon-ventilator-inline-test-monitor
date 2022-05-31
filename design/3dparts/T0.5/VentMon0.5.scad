/////////////////////////////////////////
///// VentMon 0.5 Mechanical Design /////
/////     Public Invention 2022     /////
/////          Ben Coombs           /////
/////////////////////////////////////////

tol=0.01;

//////////// Housing ////////////

w = 30;
h = 55;
d = 60;
wt = 2;

case_nozzle_cutout_diameter = 22;
endcap_nozzle_cutout_diamater = 22;
airway_center_z = -12;

// Shell
union() {
    difference() {
        cube([w, d, h], center=true);

        translate([0,1.01,0])
        cube([w-2*wt, d-wt, h-2*wt], center=true);

        // Side cutout (temporary)
        translate([2.01,0,0])
        cube([w-2*wt, d-2*wt, h-2*wt], center=true);
        
        // Display cutout
        translate([0,2,h/2-wt/2])
        cube([16,35,4], center=true);
        
        // Nozzle cutout
        translate([0,-d/2+wt/2,airway_center_z])
        rotate([90,0,0])
        cylinder(h=10, d=case_nozzle_cutout_diameter, center=true);
        
    }
}

temp_endcap_move = 0;

// End cap
color("red")
translate([0,temp_endcap_move,0])
difference() {
    union()
    {
        // End cap
        translate([0,d/2+wt/2,0])
        cube([w, wt, h], center=true);
        
        // End cap inner lip
        difference() {
            translate([0,d/2-wt/2,0])
            cube([w-wt*2-0.5, wt, h-wt*2-0.5], center=true);
            translate([0,d/2-wt/2-tol,0])
            cube([w-wt*4, wt, h-wt*2-4], center=true);
        }
                
        // PCB mounts
        translate([w/2-6,27,10.5])
        cube([6,6,8], center=true);
        translate([-w/2+6,27,10.5])
        cube([6,6,8], center=true);
        
        translate([w/2-6,27,19.5])
        cube([6,6,8], center=true);
        translate([-w/2+6,27,19.5])
        cube([6,6,8], center=true);
    }
    
    // USB port cutout
    translate([0,d/2,18])
    cube([10,8,7], center=true);
        
    // Flow sensor cutout
    translate([0,d/2,airway_center_z])
    rotate([90,0,0])
    cylinder(h=6, d=endcap_nozzle_cutout_diamater, center=true);
}

//////////// Flow Sensor ////////////

color("lightgrey")
translate([0,temp_endcap_move+20,airway_center_z])
flow_sensor();

module flow_sensor() {
    id = 10;
    t = 1;
    x = 50;
        
    translate([0,-x/2,0])
    rotate([270,0,0])
    rotate_extrude()
    polygon([[id,0], [id,20],[id+t,20],[id+t,x],
            [id+2*t,x],[id+2*t,19],[id+t,19],[id+t,0]]);
    
    translate([0,0,id])
    cube([12,20,5], center=true);
}

//////////// Nozzle ////////////

color("lightblue")
translate([0,-25,airway_center_z])
nozzle();

module nozzle() {
    id = 10;
    t = 1;
    x = 40;
        
    translate([0,-x/2,0])
    rotate([270,0,0])
    rotate_extrude()
    polygon([[id,0], [id,x/2],[id+t,x/2],[id+t,x],
            [id+2*t,x],[id+2*t,x/2-t],[id+t,x/2-t],[id+t,0]]);
}

//////////// PCB ////////////

module feather_pcb() {
    pcb_length = 50.4;
    pcb_width = 22.9;
    color("green")
    cube([pcb_width,pcb_length,1], center=true);
}

module header(num_pins) {
    header_width = 2.5;
    header_height = 8.5;
    header_length = 2.54 * num_pins + 0.25;
    color("black")
    cube([header_width, header_length, header_height], center=true);
}

// PCB stack
translate([0,temp_endcap_move+4,15])
{
    // Top
    translate([0,0,9]) {
        feather_pcb();
        // OLED
        color("black")
        translate([0,-2,1])
        cube([16,35,2], center=true);
        // OLED buttons
        color("grey"){
            translate([0,20,1.5])
            cube([3,3,3], center=true);
            translate([-5,20,1.5])
            cube([3,3,3], center=true);
            translate([5,20,1.5])
            cube([3,3,3], center=true);
        }
    }
    
    // Mid
    feather_pcb();
    translate([10,-5,4.25])
    header(12);
    translate([-10,0,4.25])
    header(16);
    // USB port
    color("grey")
    translate([0,22,2])
    cube([7,7,3], center=true);
    
    // Bottom
    translate([0,0,-9])
    feather_pcb();
    translate([10,-5,4.25-9])
    header(12);
    translate([-10,0,4.25-9])
    header(16);
}

