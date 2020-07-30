
outerHeight = 5;
innerHeight = 3;
outerWidth = 6.5;
innerWidth = 4.5;
outerLength = 8.5;
innerLength = 4.5;

$fn = 30;

barbLength = 7;
barbHeight = 7;
outerBarbRadius = 1.93;
innerBarbRadius = outerBarbRadius - 1;

pegRadius = 1;
pegLength = 2;
pegOffsetX = 0;
pegOffsetY = 3.25;
pegOffsetZ = -pegLength;


// box
rotate([0,0,90])
{
union(){
difference(){
    translate([0,0,outerHeight/2]){
    cube([outerWidth, outerLength, outerHeight], center=true);
    }
    translate([0,0,outerHeight/2 - 1]) {
        cube([innerWidth, innerLength, innerHeight], center=true);
    }
    translate([0,0,innerHeight]) {
        cylinder(barbLength, innerBarbRadius, innerBarbRadius, center=false);
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

// barb
translate([0,0,innerHeight])
{
difference()
{
  union()  {
    cylinder(barbHeight, outerBarbRadius, outerBarbRadius, center = false);
    translate([0, 0, barbHeight]) sphere(outerBarbRadius);
    translate([0, 0, barbHeight]) rotate([0, 90, 0]) cylinder(barbLength, outerBarbRadius, outerBarbRadius, center = false);
  }  
  union()  {
    translate([0, 0, -0.5]) cylinder(barbHeight - 1, innerBarbRadius, innerBarbRadius, center = false);
    translate([0, 0, barbHeight]) sphere(innerBarbRadius);
    translate([1.5, 0, barbHeight]) rotate([0, 90, 0]) cylinder(barbLength - 1, innerBarbRadius, innerBarbRadius, center = false);    
  }
}
}









echo(version=version());