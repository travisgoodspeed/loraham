# LoRaHam

This project consists of ham radio protocols and Arduino examples for
LoRa on 70cm.  The code, the protocols, and the hardware have all been
intentionally kept simple for easy experimentation, in the interest of
getting folks to build things and to put their projects on the air.

This project has only recently been made public; please join us in
`#loraham` on Freenode if you'd like to play along.

# Getting Started

You will need to buy Adafruit M0 LoRa Feather boards, which can be
programmed to act as Beacons, Gateways, and Terminals.  You should be
running the latest Arduino environment, with the Adafruit board
definitions and the RadioHead radio library.

http://www.airspayce.com/mikem/arduino/RadioHead/

# Basics of the Protocol

A Beacon is just a transmitter, and nothing else.  It doesn't receive
packets, but one every five or more minutes, it will transmit a quick
burst with a voltage, a sequence number, and maybe another sensor
reading.  A beacon's packet is addressed to the callsign "BEACON" from
its own callsign, which is followed by a suffix.  Comments can come
later.  For example, this is my solar powered beacon that won't
transmit on low voltage.

```
BEACON KK4VCZ-16 VCC=3.718945 count=437 Solar. No TX on low voltage.
```

These messages are then repeated by Gateways, which you might also
call a Digipeater.  Gateways are optionally connected to the Internet,
in order to share their packets with the wider world.  When a gateway
transmits a packet, it appends a second line addressed to `RT`
(retransmit) from its own callsign and optional observations of the
received signal strength.  In this example, `KK4VCZ-16` is the Beacon
and `KM4BBD-10` is the Gateway.

```
BEACON KK4VCZ-16 VCC=3.718945 count=437 Solar. No TX on low voltage.
RT KM4BBD-10 rssi=-46
```

Finally, you might want to use a LoRaHam network for your own personal
messages on an interactive Terminal.  In this case, you will send the
initial packet in the form of ``DESTINATION SOURCE Text``.  So when
`KK4VCZ` sends a message to `KC3BVL`, it'll look like this.

```
KC3BVL KK4VCZ Hey Jim, it's Travis.  Care for a beer?
```

The packet might be retransmitted by a gateway or two, so when Jim
(`KC3BVL`) receives the packet, it might look like the following if
repeated by `KM4BBD-10` and `AB3XL-13`.  Note that the first line is
unchanged, and also that you can graph the path that the packet took
by looking at the later lines.

```
KC3BVL KK4VCZ Hey Jim, it's Travis.  Care for a beer?
RT KM4BBD-10 rssi=-46
RT AB3XL-13 rssi=-80
```

The extra lines are added in order, so the packet went from `KK4VCZ`
to `KM4BBD-10` to `AB3XL-13` to `KC3BVL`.  Just like on the first
line, the destination is always listed first and the source listed
second.


# Hardware Designs

See the Wiki at https://github.com/travisgoodspeed/loraham/wiki for
our hardware reference designs.  In brief, we are simply powering
Adafruit LoRa Feather 434MHz boards with solar-charging motion
activated light boxes from which the lights have been torn out.

# Active Networks

The first LoRaHam network was born and raised in West Philadelphia, on
the airwaves it passes packets each day.  Other networks are planned;
please say Howdy in the IRC channel if you'd like to start one.
