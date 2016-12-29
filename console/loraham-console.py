#!/usr/bin/env python3


"""This is a half-assed attempt at a LoRaHam GUI that was written to
learn the Urwid library.  Transmitting packets does not yet work, but
it does allow a user to conveniently bounce between viewing packets
by different filters."""

import urwid, serial;


#FIXME Callsign and port are hardwired.  
MYCALL = "KK4VCZ";
#ser = serial.Serial('/dev/ttyS0');
ser = serial.Serial('/dev/ttyUSB0');

SHOWALL=1;
SHOWMINE=2;
SHOWVERBOSE=3;
toshow=SHOWALL;
def on_showall_clicked(button):
    global toshow;
    toshow=SHOWALL;
    show_packets();
def on_showmine_clicked(button):
    global toshow;
    toshow=SHOWMINE;
    show_packets();
def on_showverbose_clicked(button):
    global toshow;
    toshow=SHOWVERBOSE;
    show_packets();
    
def show_packets():
    global toshow, allpackets, mypackets, verbloselog;
    if toshow==SHOWALL:
        received.set_text(allpackets);
    elif toshow==SHOWMINE:
        received.set_text(mypackets);
    elif toshow==SHOWVERBOSE:
        received.set_text(verboselog);

def on_packet_change(edit, new_edit_text):
    pass;

#Is my callsign mentioned in the line?
def ismentioned(line):
    for word in line.split():
        if word==MYCALL:
            return True;
    return False;
    
linestate=0;
lastline="";
def handle_line(line):
    global allpackets, mypackets, verboselog, linestate, lastline;

    #Verbose log has everything.
    verboselog=verboselog+line+'\n';

    #Empty lines delimit transmissions.
    if len(line)==0:
        linestate=0;
        return;

    #First line is the real message.
    if linestate==0 and line!=lastline:
        lastline=line;
        allpackets=allpackets+line+"\n";
        if ismentioned(line):
            mypackets=mypackets+line+"\n";
    linestate=linestate+1;
    
def ser_available():
    global ser, allpackets;
    try:
        newline=ser.readline().decode("utf-8").strip();
        handle_line(newline);
        show_packets();
    except:
        handle_line("Data lost to UTF-8 corruption.");
        show_packets();


#Some globals for our state.
allpackets="";
mypackets="";
verboselog="";

palette = [('I say', 'default,bold', 'default', 'bold'),]

packet=urwid.Edit(('I say', u"Packet to send:\n"));
received=urwid.Text(u"Packets will appear here when they arrive.");
div=urwid.Divider();
#button=urwid.Button(u'Exit');

showall=urwid.Button(u'All');
showmine=urwid.Button(u'Mine');
showverbose=urwid.Button(u'Verbose');
sendpacket=urwid.Button(u'Send');
buttons=urwid.Columns([showall,showmine,showverbose,sendpacket]);

pile=urwid.Pile([received,div,buttons,packet]);
top=urwid.Filler(pile,valign='bottom');

urwid.connect_signal(packet, 'change', on_packet_change);
urwid.connect_signal(showall, 'click', on_showall_clicked);
urwid.connect_signal(showmine, 'click', on_showmine_clicked);
urwid.connect_signal(showverbose, 'click', on_showverbose_clicked);




loop=urwid.MainLoop(top,palette)
loop.watch_file(ser, ser_available);
loop.run();

