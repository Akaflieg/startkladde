#!/usr/bin/env ruby

def usage
	puts "Sends one file/com port/... to another, one line at a time, with a configurable delay between lines"
	puts "The delay is specified in seconds."
	puts "Note that if you send to a com port, you must set the baud rate by some other means."
	puts "Usage: #{$0} output delay input"
	puts "Example: #{$0} COM8: 0.1 file.txt"
	exit 1
end

usage if ARGV.size<3

output=ARGV[0]
delay=ARGV[1].to_f
input=ARGV[2]

begin
	File.open(output, "w") { |p|
		p.sync=true
		File.open(input, "r").each { |line|
			puts "Sending: "+line
			p.puts line
			sleep delay
		}
	}
rescue Exception => ex
	p ex
end

