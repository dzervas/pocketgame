x = 5.3;
y = 20;
z = 6.5;
mobile_y = 16;
mobile_z = 9.5;
angle = 40;
thick = 2;

difference() {
	union() {
		cube([x, y + mobile_z + 2*thick, z + 2*thick]);
		translate([0, y + mobile_z + 2*thick, 0]) rotate([angle, 0, 0]) cube([x, mobile_y, mobile_z + 2*thick]);
	}
	union() {
		translate([-1, -1, thick]) cube([x + 2, y + 1, z]);
		translate([-1, y + mobile_z + 3*thick, (thick / cos(angle)) + thick]) rotate([angle, 0, 0]) cube([x + 2, mobile_y + 1, mobile_z]);
	}
}