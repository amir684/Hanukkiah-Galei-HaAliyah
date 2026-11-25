// =====================================================================
//  Hanukkiah "Galei HaAliyah" (Waves of Aliyah)
//  Parametric model, OpenSCAD 2021.01+
//
//  Pick a part with -D part="..." on the command line, or edit below:
//    assembly     - full preview (not for printing)
//    fan          - waves + mast, WHITE PLA, print flat on its back
//    hull         - ship hull with text, DARK PLA, print flat
//    sail_r/sail_l- sails, DARK PLA, print flat
//    candle       - candle tube, WHITE PLA, print x9 standing
//    base_shell   - base, DARK PLA, print upside-down (top on bed)
//    base_bottom  - base bottom plate, DARK PLA
//    base_window  - light diffuser with text, WHITE PLA, print text up
//    hub_cover    - cover for the wiring cavity behind the ship, DARK PLA
// =====================================================================

part = "assembly";

$fn = 48;

// ---------- main dimensions (mm) ----------
T        = 14;     // fan thickness (front-to-back)
FW       = 1.2;    // front diffuser wall over the LED strips
AG       = 4;      // air gap between strip and diffuser (softens LED dots)
ARM_W    = 9;      // width of each wave arm
Y0       = 30;     // where the arms leave the hub (hidden behind the hull)
HC       = 185;    // top of the 8 candle bosses (all equal height)
HM       = 230;    // top of the mast / shamash boss (higher than the rest)
ARM_SX   = [13, 26, 39, 52];   // arm start x (one side, mirrored)
ARM_XC   = [20, 47, 74, 101];  // candle x positions (one side, mirrored)
BEZ      = [0.5, 0.25, 0.3, 0.6]; // arm shape: [a, b, c, d] (see arm_ctrl)

STRIP_A  = 2.7;    // arm strip width (WS2812B 2020, 2.7 mm)
STRIP_M  = 5.0;    // mast strip width (WS2812B COB, 5 mm)
SOCK_D   = 10;     // candle socket diameter
PEG_D    = 9.6;    // candle peg diameter

BASE_HX  = 80;     // base: centre distance of round ends (half)
BASE_R   = 45;     // base: end radius  -> 250 x 90 footprint
BASE_H   = 55;     // base: total height
BASE_WALL= 2.4;
BASE_TOP = 3;
BOT_T    = 3;      // bottom plate thickness
FAN_Y    = -2;     // world Y of the fan front face

FONT = "Arial:style=Bold";
TXT_BASE = "נס גדול היה פה";
TXT_HULL = "ושבו בנים לגבולם";

C_DARK  = "#2d323b";
C_WHITE = "#f2efe6";
C_FLAME = "#ffa21f";
C_GLOW  = "#8fdcff";
LIT     = false;   // assembly preview: show LED light lines

// ---------- helpers ----------
function bez(p, t) = let(u = 1 - t)
    u*u*u*p[0] + 3*u*u*t*p[1] + 3*u*t*t*p[2] + t*t*t*p[3];
function bez_d(p, t) = let(u = 1 - t)
    3*u*u*(p[1]-p[0]) + 6*u*t*(p[2]-p[1]) + 3*t*t*(p[3]-p[2]);
function nrm(v) = let(l = norm(v)) [-v[1]/l, v[0]/l];

function arm_ctrl(s, c) = let(L = (HC - 8) - Y0) [
    [s, Y0],
    [s + BEZ[0]*(c - s), Y0 + BEZ[1]*L],
    [c + BEZ[2]*(c - s), Y0 + BEZ[3]*L],
    [c, HC - 8]];

// thick curve along a bezier between t0..t1
module bez_band(p, w, t0 = 0, t1 = 1, n = 60) {
    pts  = [for (i = [0:n]) let(t = t0 + (t1 - t0)*i/n) bez(p, t)];
    nn   = [for (i = [0:n]) let(t = t0 + (t1 - t0)*i/n) nrm(bez_d(p, t))];
    polygon(concat([for (i = [0:n]) pts[i] + nn[i]*w/2],
                   [for (i = [n:-1:0]) pts[i] - nn[i]*w/2]));
}

module ring_arc(r, w, a0, a1, n = 40) {
    polygon(concat(
        [for (i = [0:n]) let(a = a0 + (a1 - a0)*i/n) (r + w/2)*[cos(a), sin(a)]],
        [for (i = [n:-1:0]) let(a = a0 + (a1 - a0)*i/n) (r - w/2)*[cos(a), sin(a)]]));
}

module rrect(w, h, r) { offset(r) square([w - 2*r, h - 2*r], center = true); }

module heb(txt, size) {
    text(txt, size = size, font = FONT, halign = "center", valign = "center",
         direction = "rtl", language = "he", script = "hebrew");
}

// ---------- FAN (waves + mast) : 2D outline ----------
module crest2d(c, s) {          // s = +1 right side, -1 left side
    translate([c, HC - 8]) rrect(14, 16, 3);
    // breaking-wave curl, rolling outward
    translate([c + s*10, HC - 7])
        mirror([s < 0 ? 1 : 0, 0]) ring_arc(4.5, 3, -70, 175);
    translate([c + s*10, HC - 7]) rotate(s > 0 ? -70 : 250)
        translate([4.5, 0]) circle(d = 3.6);
}

module mast2d() {
    translate([-7, 0]) square([14, HM - 9]);
    translate([0, HM - 9]) rrect(16, 18, 3);
}

// hub follows the hull outline (3 mm inside it) so it is hidden behind the ship
module hub2d() {
    intersection() {
        offset(delta = -3) hull_body2d();
        translate([-80, 0]) square([160, 37]);
    }
}

module fan2d() {
    hub2d();
    mast2d();
    for (s = [-1, 1]) for (i = [0:3]) {
        bez_band(arm_ctrl(s*ARM_SX[i], s*ARM_XC[i]), ARM_W);
        crest2d(s*ARM_XC[i], s);
    }
}

// grooves (cut from the back face)
module arm_grooves2d(w) {
    for (s = [-1, 1]) for (i = [0:3])
        bez_band(arm_ctrl(s*ARM_SX[i], s*ARM_XC[i]), w, 0, 1);
}
module mast_groove2d(w) { translate([-w/2, 8]) square([w, HM - 9 - 8]); }
module cavity2d() {
    intersection() {
        offset(delta = -3) hub2d();
        translate([-80, 5]) square([160, 28]);
    }
}

module teardrop2d(r) {   // tip points to -y (becomes +z after rotation)
    circle(r);
    polygon([[-r*0.707, -r*0.707], [r*0.707, -r*0.707], [0.6, -r*1.25], [-0.6, -r*1.25]]);
}
module socket(x, ytop) {
    translate([x, ytop - 12, T/2]) rotate([-90, 0, 0])
        linear_extrude(13) teardrop2d(SOCK_D/2);
}

module fan() {
    slot_h = T - FW - AG;           // depth of strip slot from the back
    difference() {
        linear_extrude(T) fan2d();
        // strip slots (strip rests on the ledge, LEDs facing the front)
        translate([0, 0, -1]) linear_extrude(slot_h + 1) {
            arm_grooves2d(STRIP_A + 0.7);
            mast_groove2d(STRIP_M + 0.8);
        }
        // narrow air channel in front of the strip
        translate([0, 0, slot_h - 0.01]) linear_extrude(AG + 0.01) {
            arm_grooves2d(STRIP_A - 0.5);
            mast_groove2d(STRIP_M - 0.4);
        }
        // wiring cavity behind the ship + exit into the base
        translate([0, 0, -1]) linear_extrude(T - 2 + 1) cavity2d();
        translate([-20, -1, -1]) cube([40, 10, T - 2 + 1]);
        // candle sockets
        for (s = [-1, 1]) for (i = [0:3]) socket(s*ARM_XC[i], HC);
        socket(0, HM);
    }
    // pegs into the base
    for (s = [-1, 1]) translate([s*45 - 5, -12, 3]) cube([10, 12.01, 8]);
}

module hub_cover() {   // glued over the cavity on the back
    linear_extrude(1.6) offset(3) cavity2d();
}

// ---------- SHIP ----------
module hull_body2d() {
    polygon([[-74, 46], [-64, 32], [-56, 10], [-46, 0], [50, 0], [60, 8],
             [67, 24], [69, 42], [62, 41], [-50, 40]]);
}
module hull2d() {
    hull_body2d();
    translate([22, 38]) square([26, 11]);        // deck house
}
module ship_hull() {
    difference() {
        linear_extrude(6) hull2d();
        for (x = [-36:13:36]) translate([x, 29, 5]) cylinder(r = 2.4, h = 2);   // portholes
        for (x = [26, 38]) translate([x, 44, 5]) cube([5, 3, 2]);              // windows
        translate([-52, 33.5, 5.4]) cube([118, 1.2, 2]);                       // deck line
    }
    translate([0, 15, 6]) linear_extrude(1) heb(TXT_HULL, 7);
}

// mainsail: behind the mast (right, the ship sails to the left),
// leech bellied by the wind
module sail_r2d() {
    polygon(concat(
        [[8, 142]],
        [for (i = [0:24]) let(t = i/24) bez([[8,142],[26,124],[50,90],[48,60]], t)],
        [[8, 56]]));
}
// jib: from the mast head to the bow
module sail_l2d() {
    polygon(concat(
        [[-8, 134]],
        [for (i = [0:24]) let(t = i/24) bez([[-8,134],[-26,118],[-50,84],[-54,54]], t)],
        [[-8, 54]]));
}
module sail(which) {
    linear_extrude(4) { if (which == "r") sail_r2d(); else sail_l2d(); }
    // seams
    translate([0, 0, 4]) linear_extrude(0.8) intersection() {
        if (which == "r") sail_r2d(); else sail_l2d();
        for (y = [72:16:128]) translate([-60, y]) square([120, 1.2]);
    }
}

// ---------- CANDLE ----------
// Flame LED (DC3V / DC3-5V "candle flame COB"): 38 mm tall, flame 13.8 mm wide,
// PCB tail 9 mm wide x ~11 mm tall, ~3 mm thick incl. the SOIC-8 driver chip.
// The whole PCB tail hides inside the candle; only the flame shows above it.
LED_PCB_W = 9;     // PCB tail width
LED_PCB_H = 11;    // PCB tail height (hidden part)
LED_PCB_T = 3;     // PCB + chip thickness
CANDLE_D  = 13;    // candle body diameter (close to the 13.8 mm flame width)
CANDLE_H  = 34;    // total candle height incl. 10 mm peg

module candle() {
    slot_d = LED_PCB_H + 1;
    difference() {
        union() {
            cylinder(d = PEG_D, h = 10);                                   // peg (into socket)
            translate([0, 0, 10]) cylinder(d = 16, h = 2);                 // collar
            translate([0, 0, 12]) cylinder(d = CANDLE_D, h = CANDLE_H - 12);       // body
            translate([0, 0, CANDLE_H - 3]) cylinder(d1 = CANDLE_D, d2 = CANDLE_D + 1.4, h = 3); // drip lip
        }
        translate([0, 0, -1]) cylinder(d = 5, h = CANDLE_H);                // wire hole
        // LED board slot: flame faces the front (world -Y)
        translate([0, 0, CANDLE_H - slot_d/2 + 0.01])
            cube([LED_PCB_W + 0.6, LED_PCB_T + 0.6, slot_d], center = true);
        // room for the two solder joints + wire bends under the board
        translate([0, 0, CANDLE_H - slot_d - 5]) cylinder(d = 8, h = 5.01);
    }
}

// ---------- BASE ----------
module stadium2d(o = 0) {
    hull() for (s = [-1, 1]) translate([s*BASE_HX, 0]) circle(BASE_R + o, $fn = 96);
}
module window2d() {   // front light window with a wavy top edge
    polygon(concat([[-72, 14]],
        [for (i = [0:72]) let(x = -72 + i*2) [x, 38 + 3*sin(x*6)]],
        [[72, 14]]));
}
BOSSES = [[-80, 0], [80, 0], [20, 36]];
SHELL_H = BASE_H - BOT_T;

module base_shell() {   // modelled upright; world Z from BOT_T to BASE_H
    difference() {
        union() {
            difference() {
                linear_extrude(SHELL_H) stadium2d();
                translate([0, 0, -1]) linear_extrude(SHELL_H - BASE_TOP + 1) stadium2d(-BASE_WALL);
            }
            for (b = BOSSES) translate(b) cylinder(d = 8, h = SHELL_H);
            // rib that carries the COB strip behind the window (strip on its front face)
            translate([-74, -22, 10]) cube([148, 2, SHELL_H - 10]);
        }
        for (b = BOSSES) translate([b[0], b[1], -1]) cylinder(d = 2.6, h = SHELL_H - 4);
        // front window
        translate([0, -38, -BOT_T]) rotate([90, 0, 0]) linear_extrude(10) window2d();
        // fan pegs + wire passage
        for (s = [-1, 1]) translate([s*45 - 5.25, 0.75, SHELL_H - 10]) cube([10.5, 8.5, 20]);
        translate([-20, -1, SHELL_H - 10]) cube([40, 14, 20]);
        // touch pad: thin roof from inside + engraved ring on top
        translate([95 - 8, -6.5, -1]) cube([16, 13, SHELL_H - 1.5 + 1]);
        translate([95, 0, SHELL_H - 0.5]) difference() { cylinder(r = 9.5, h = 1); cylinder(r = 8.3, h = 1); }
        translate([95, 0, SHELL_H - 0.5]) cylinder(r = 1.8, h = 1);
        // back: USB-C power + push button (12 mm)
        translate([-46.5, 38, 5]) cube([13, 10, 6.5]);
        translate([40, 38, 25 - BOT_T]) rotate([-90, 0, 0]) cylinder(d = 12.5, h = 10);
    }
}

module base_bottom() {
    difference() {
        linear_extrude(BOT_T) stadium2d(-0.0);
        for (b = BOSSES) translate([b[0], b[1], -1]) {
            cylinder(d = 3.4, h = 10);
            translate([0, 0, 1 + BOT_T - 1.8]) cylinder(d1 = 3.4, d2 = 6.6, h = 1.81);
        }
        // cable relief notch at the back (not needed if USB-C is used)
    }
    // standoffs for a 50x70 perfboard (holes 46 x 66) holding ESP32-S3 SuperMini
    for (dx = [-33, 33], dy = [-23, 23]) translate([-25 + dx, 8 + dy, BOT_T])
        difference() { cylinder(d = 6, h = 6); cylinder(d = 2.5, h = 7); }
}

module base_window() {   // white diffuser, raised text on the front (top) face
    linear_extrude(2) offset(4) window2d();
    translate([0, 25, 2]) linear_extrude(2) heb(TXT_BASE, 12);
}

// ---------- ASSEMBLY ----------
module flame() {   // the real LED flame: 13.8 wide, ~27 tall above the candle, ~2 thick
    color(C_FLAME) rotate([90, 0, 0]) linear_extrude(2, center = true) {
        hull() { translate([0, 6]) scale([1, 1.1]) circle(6.9); translate([1.5, 27]) circle(0.6); }
        hull() { translate([-4, 8]) circle(2.5); translate([-6.2, 17]) circle(0.5); }
    }
}
module place_fan_local() {   // local (x, height, depth) -> world
    translate([0, FAN_Y + T, BASE_H]) rotate([90, 0, 0]) children();
}

module assembly() {
    color(C_DARK) translate([0, 0, BOT_T]) base_shell();
    color(C_DARK) base_bottom();
    color(C_WHITE) translate([0, -40.6, -BOT_T + BOT_T]) rotate([90, 0, 0]) base_window();
    color(C_WHITE) place_fan_local() fan();
    if (LIT) {   // preview only: where the LED light shows through the diffuser
        color(C_GLOW) place_fan_local() translate([0, 0, T]) linear_extrude(0.3)
            intersection() {
                union() { arm_grooves2d(3); mast_groove2d(4.5); }
                translate([-150, 40]) square([300, 300]);
            }
        color(C_GLOW) translate([0, -42.62, 0]) rotate([90, 0, 0])
            linear_extrude(0.3) offset(-1) window2d();
    }
    // ship in front of the fan
    color(C_DARK) translate([0, FAN_Y, BASE_H]) rotate([90, 0, 0]) ship_hull();
    color(C_DARK) translate([0, FAN_Y, BASE_H]) rotate([90, 0, 0]) sail("r");
    color(C_DARK) translate([0, FAN_Y, BASE_H]) rotate([90, 0, 0]) sail("l");
    // candles + flames
    for (p = concat([for (s = [-1, 1]) for (i = [0:3]) [s*ARM_XC[i], HC]], [[0, HM]]))
        translate([p[0], FAN_Y + T/2, BASE_H + p[1] - 10]) {
            color(C_WHITE) candle();
            translate([0, 0, CANDLE_H]) flame();
        }
}

if (part == "assembly")    assembly();
if (part == "fan")         fan();
if (part == "hull")        ship_hull();
if (part == "sail_r")      sail("r");
if (part == "sail_l")      sail("l");
if (part == "candle")      candle();
if (part == "base_shell")  rotate([180, 0, 0]) translate([0, 0, -SHELL_H]) base_shell();
if (part == "base_bottom") base_bottom();
if (part == "base_window") base_window();
if (part == "hub_cover")   hub_cover();
