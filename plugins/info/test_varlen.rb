#!/usr/bin/env ruby

STDOUT.sync=true

while true
	puts "long"
	sleep 1
	puts "long, very, very long"
	sleep 1
	puts "even longer, longer, longer, longer"
	sleep 1
	puts "so long that it will easily fit in one line with shifting, without wrapping"
	sleep 2
	puts "extremely long. in fact, this text is do long that we will have to wrap it or else the text won't fit in the column."
	sleep 2
end

