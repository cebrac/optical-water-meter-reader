# optical-water-meter-reader
Arduino based device to generate pulse output from conventional water meter with a rotating disc/index.

Attach laser diode on pin 3. Make sure to add additional resistor in series if you want to drive a 3V rated
laser with 3.3V. Even small over voltage will destroy laser (it will get much darker over time).

Attach phototransistor long leg to GND, short leg to A6 and a 10K resistor between A6 and VCC to pull the
pin up.

Print enclosure so that laser is focused against a surface of periodically changing reflectivity, i.e. the
small rotating disc or index of the water meter. Usually one full rotation corresponds to 1 liter (unless
your meter uses wacky pound-ounces or whatever).

Ensure that minimal ambient light can shine onto the phototransistor. Use a suitable type that can pick up
the 650nm emitted by laser. Put phototransistor close to laser but so that no diffuse reflection from the
laser into the phototransistor will ocur. Use a dark material for enclosure.

When powered, onboard LED will blink and laser should come on (looking somewhat dim because it gets only
pulsed). Learning will commence and the further the learning process, the faster the LED blinking should
become. Once fully trained (after about 3 Liter flow) the LED should come on or remain off (depending on
location of index/rotating disc).

Pulse output on pin 10. Voltage level depending on board used.
