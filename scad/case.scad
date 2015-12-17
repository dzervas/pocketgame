button_x = 6.2;
button_y = button_x;
crossins_x = 9;
crossins_y = crossins_x;
ant_x = 14;
ant_y = 2;

module outer() {
	difference() {
		cube([100, 54, 8]);
		translate([2,2,2]) cube([96, 50, 4]);
	}
}

module button(x, y) {
	translate([x, y, 5]) cube([button_x, button_y, 4]);
}

module crossb(x, y) {
	union() {
		button(x + button_x + 1, y);
		button(x, y + button_y + 1);
		button(x + button_x + 1, y);
		button(x + button_x + crossins_x + 1, y + button_y + 1);
		button(x + button_x + 1, y + button_y + crossins_y + 1);
	}
}

difference() { 
	outer();
	union() {
		crossb(16, 18.5);
		crossb(62.5, 18.5);
	}
}