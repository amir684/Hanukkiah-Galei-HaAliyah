// Cross-section of one candle with the flame LED inside (fit check only)
include <hanukkiah.scad>
part = "none";
// candle cut in half (front half removed)
color(C_WHITE) difference() { candle(); translate([-20, -40, -1]) cube([40, 40, 60]); }
// LED: PCB tail (9 x 11 x 1) + SOIC-8 chip, seated on the slot floor
translate([0, 0, CANDLE_H - LED_PCB_H - 1]) {
    color("#c8a24a") translate([-4.5, -0.5, 0]) cube([9, 1, 11]);
    color("#222") translate([-2.5, -2.2, 3]) cube([5, 1.7, 5]);
    translate([0, 0, 11]) flame();
}
