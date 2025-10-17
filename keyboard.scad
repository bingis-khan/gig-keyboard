$fa = 1;
$fs = 0.4;

print = false;  // may be redefined at command line


// right now, let's make a printable "slab" where I can insert the keys and put on keycaps to see if they fit.

// spacing: https://null-src.com/posts/keyboard-design-cheatsheet/
// apparently, 1u = 19.05mm

key_size = 14;
unit = 19.05;
key_pin = 3.3;  // from ref.
key_body = 5;
key_underside = key_pin + key_body;
key_off = (unit - key_size) / 2;

max_key_with_keycap_exact_height = 18;  // in mm. based on OEM num keys.
pressed_key_height = 11;  // close enough.
exposed_switch_height = 6;  // close enough
slump = exposed_switch_height;  // not sure how much to lower the keyboard plane. this variable tracks it.

// 2 parts. these numbers refer to one part.
keys_in_row = 6;
keys_in_column = 4;  // NOTE: last column is different.

use_width = keys_in_row * unit;
use_height = keys_in_column * unit;

cable_zone_height = 7;

off = 0;  // random offset, so that the part "sticks"

// if i decide to make the keyboard angled, this would be it.
key_plane_angle = 0;

avg_depth = 5;
cut_depth = 2;  // cutting shit


function sum(l, i = 0) =
    i < len(l)
        ? l[i] + sum(l, i + 1)
        : 0;

function slice(l, from, to) =
    to != undef  // HACK: if to is undefined, then we assume 'from' is actually 'to' and from is 0.
        ? to >= from  // when range is empty, make empty list yo.
            ? [for (i = [from : to]) l[i]]
            : []
        : slice(l, 0, from);

module key_plane_cutouts(edge_off, last_line) {
    key_begin = (unit - key_size) / 2;
    for (x = [key_begin : unit : use_width]) {
        // ignore closest key line
        for (y = [key_begin + unit : unit : use_height]) {
            translate([x + edge_off, y + edge_off, -cut_depth/2]) cube([key_size, key_size, avg_depth + cut_depth]);
        }
    }

    // add later lines
    last_line_units = [for (s = last_line) s * unit];
    for (i = [0 : len(last_line_units) - 1]) {
        spacing_until_now = sum(slice(last_line_units, i - 1));
        spacing = last_line_units[i];

        key_off = (spacing - key_size) / 2;

        x = edge_off + spacing_until_now + key_off;
        y = edge_off + key_begin;
        translate([x, y, -cut_depth/2]) cube([key_size, key_size, avg_depth + cut_depth]);
    }
}

module test_key_grid() {
    key_begin = (unit - key_size) / 2;
    for (x = [key_begin : unit : use_width]) {
        for (y = [key_begin : unit : use_height]) {
            translate([x + off, y + off, -cut_depth/2]) cube([key_size, key_size, avg_depth + cut_depth]);
        }
    }
    
}

module key_plane(edge_off, last_line) {
    part_width = use_width + 2*edge_off;
    height = use_height + 2*edge_off + cable_zone_height;
    difference() {
        cube([part_width, height, avg_depth]);
        
        key_plane_cutouts(edge_off, last_line);
        translate([key_off, height - cable_zone_height - conn_off, -conn_off]) cube([use_width - 2*key_off, cable_zone_height, avg_depth]);

        // right part.
        // key_plane([1.75, 1.25, 1, 1, 1, 1]);
    }
}


module flip_z() {
    scale([1, 1, -1]) children();
}

module flip_y() {
    scale([1, -1, 1]) children();
}

module flip_x() {
    scale([-1, 1, 1]) children();
}

module id() {
    children();
}

// half function half comment. used to mean that we cut FAR.
function farcut(x) =
    x != undef
        ? x + 1000
        : 1000;


rp_width = 24;
rp_height = 18;
rp_more = 3;  // elongate the thing, so you can put in the rpi easier.
usbc_width = 7;
usbc_height = 9 + 1;  // measured + error
usbc_z = 3 + 1;
rp_off = 1.5;

module rp2040() {
	cube([rp_width, rp_height, 1]);
	translate([-2, (rp_height  - 9) / 2, 1]) cube([usbc_width, usbc_height, usbc_z]);
}

wall_thickness = 10;
plane2ground = key_underside + 5;  // 5mm added to approximate the actual size of this keyboard.
front_back_wall_height = max_key_with_keycap_exact_height - slump;
left_right_wall_height = front_back_wall_height * 2;
max_height = plane2ground + left_right_wall_height;  // for now! we're not including the bottom layer yet.
echo("max height: ", max_height);
connection_off = 0;  // for connections with surrounding wall. not needed now.
extra = 1;
wall_down = key_underside - key_body + extra;

module part() {
			// keys
	    translate([0, 0, wall_down]) {
				children();

				// wallz
				flip_z() difference() {
					cube([use_width, use_height + cable_zone_height, wall_down]);

					translate([key_off, key_off, -.1]) cube([use_width - 2*key_off, use_height + cable_zone_height - 2*key_off, farcut()]);
				}
			}
}


pin = 3;  // got by measuring
conn_width = 5 * pin;
conn_height = 2 * pin;
conn_min_depth = 12;
conn_off = 1.5;

all_height = wall_down + 2*key_off;  // NOTE: does not make sense at all. it's my shitty coding, because I didn't bother to look through the code now.
thing_width = conn_width + 2 * conn_off;
thing_height = conn_height + 2 * conn_off;
thing_z = conn_min_depth + 5;

magnet_length = 28 + 1;
magnet_thickness = 2 + .1;  // minimal error - we are operating on scraps. very thin walls.
magnet_height = 9 + 1;

extra_off = 0;

module left_part() {
		difference() {
			id () {
		    part() key_plane(connection_off, [1.25, 1.25, 1.25, 2.25]);

				// magnet thing
				// magnet_thing_length = thing_z + cable_zone_height + key_off;
				// translate([use_width - extra_off, use_height - key_off, 0]) {
				// 	cube([extra_off, magnet_thing_length, all_height + 5]);
				// }


				// connector part
				translate([use_width - thing_width - extra_off, use_height + cable_zone_height, 0]) {
					difference() {
						cube([thing_width, thing_z, max(thing_height, all_height)]);

						// main shaft
						translate([conn_off, 0, conn_off]) cube([thing_width - 2*key_off, thing_z - conn_off, farcut()]);

						// pinout
						translate([conn_off, 0, conn_off]) cube([thing_width - 2*conn_off, farcut(), thing_height - 2*conn_off]);
					}
				}

				// arduino part
				rpp_width = rp_height + 2*rp_off;
				rpp_height = rp_width + rp_more + rp_off;
				translate([use_width - thing_width - rpp_width + rp_off - extra_off, use_height + cable_zone_height, 0]) {
					difference() {
						id() {
							cube([rpp_width, rpp_height, max(usbc_z + rp_off + 2 + rp_off, all_height)]);
						}

						// top shaft
						translate([rp_off, -.1, rp_off]) cube([rpp_width - 2*rp_off, rpp_height - rp_off, farcut()]);

						// usbc cutout
						translate([(rpp_width - usbc_height) / 2, 10, rp_off + 2]) cube([usbc_height, farcut(), usbc_z]);

						// main shaft
						// translate([key_off, 0, key_off]) cube([rpp_width - 2*key_off, rpp_height - key_off, farcut()]);

						// pinout
						// translate([key_off, 2*key_off, key_off]) cube([thing_width - 2*key_off, thing_z - key_off, thing_height - 2*key_off]);
					}
				}
			}

			// cut between wire connector and keyboard
			translate([use_width - thing_width + conn_off - extra_off, use_height + cable_zone_height - 2*conn_off - 4, conn_off - conn_off/2]) cube([key_size, thing_z - conn_off, thing_height - 2*conn_off + conn_off/2]);

			// rp - keyboard
			translate([use_width - thing_width - key_off/2 + key_off - .2 - unit - extra_off, use_height + cable_zone_height - 2*key_off - 3, key_off - key_off/2]) cube([key_size - key_off, thing_z - key_off, thing_height - 2*key_off + key_off/2]);

			// magnet hollow outs
			// NOTE: the magnet is as tall as the keyboard, so I'll do a vertical cut.
			// the magnet should be glued to it.
			translate([use_width - extra_off - magnet_thickness + 0.01, use_height - magnet_length, -0.01]) {
				cube([magnet_thickness, magnet_length, magnet_height]);
			}

			translate([use_width - extra_off - magnet_thickness + 0.01, key_off + key_off/2, -0.01]) {
				cube([magnet_thickness, magnet_length, magnet_height]);
			}
		}
}

module right_part() {
    flip_x() {
		difference() {
			id() {
		 part() key_plane(connection_off, [1, 1, 1, 1, 1, 1]);  // funny hack. i simply reversed the last line keys in a list and it works :3
		  
				// connector part
				translate([use_width - thing_width, use_height + cable_zone_height, 0]) {
					difference() {
						cube([thing_width, thing_z, max(thing_height, all_height)]);

						// main shaft
						translate([conn_off, 0, conn_off]) cube([thing_width - 2*key_off, thing_z - conn_off, farcut()]);

						// pinout
						translate([conn_off, 0, conn_off]) cube([thing_width - 2*conn_off, farcut(), thing_height - 2*conn_off]);
					}
				}
			}

			// cut between wire connector and keyboard
			translate([use_width - thing_width + conn_off, use_height + cable_zone_height - 2*key_off - 3, key_off - key_off/2]) cube([key_size, thing_z - key_off, thing_height - 2*key_off + key_off/2]);

			
			// magnet hollow outs
			// NOTE: the magnet is as tall as the keyboard, so I'll do a vertical cut.
			// the magnet should be glued to it.
			translate([use_width - extra_off - magnet_thickness + 0.01, use_height - magnet_length, -0.01]) {
				cube([magnet_thickness, magnet_length, magnet_height]);
			}

			translate([use_width - extra_off - magnet_thickness + 0.01, key_off + key_off/2, -0.01]) {
				cube([magnet_thickness, magnet_length, magnet_height]);
			}

		}

	}
}


// BOB_SCAD_PRINT left_part right_part
if (!print) {
    left_part();
    translate([300, 0, 0]) right_part();
}
