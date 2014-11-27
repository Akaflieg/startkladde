#!/usr/bin/env ruby

STDOUT.sync=true

delay=1

if ARGV[0]
	delay=ARGV[0].to_f
	delay=0.2 if delay<0.2
end

while true
	puts '<font color="#FF0000">Rot</font><br><font color="#0000FF">Grün</font><br><font color="#00FF00">Blau</font>'
	sleep delay
	puts '<font color="#00FF00">Rot</font><br><font color="#FF0000">Grün</font><br><font color="#0000FF">Blau</font>'
	sleep delay
	puts '<font color="#0000FF">Rot</font><br><font color="#00FF00">Grün</font><br><font color="#FF0000">Blau</font>'
	sleep delay
end



