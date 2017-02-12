# SMS-lights
Control strip of LED lights by SMS using a particle electron.

Particle electron is connected to a strip of 150 addressable LEDs which can then be controlled by SMS. 

The SMS specifies colours, repeats and effect. Possible effects are blend, flash, cycle, fade, chase.

Examples:

"Red 2 green" - lights are red, red, green, red, red, green, ... along the length of the strip.

"Blue pink yellow fade"  - lights start blue, gradually change to pink, then yellow then back to blue. 

"Cyan 9 green 1 chase" - Every tenth light is green, these move along the strip.

"red orange yellow cycle" - all lights start red, chnage to orange, change to yellow, back to red.

Effects continue until a new message is received. Unparsable messages have no effect. 
Colours can also be specfied as six hexadecimal digits in RGB form, eg FF0000 for red.
