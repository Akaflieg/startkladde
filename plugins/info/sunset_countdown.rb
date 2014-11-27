#!/usr/bin/env ruby

$: << File.dirname($0) # Load files from the script directory

require 'sunset_common'

STDOUT.sync=true

while true
	begin
		# Time until sunset in seconds
		dt=truncate_seconds(determine_sunset)-truncate_seconds(Time.new)

		if dt<0
			puts "<font color=\"#FF0000\">-#{format_duration(dt)}</font>"
		else
			# Use the default foreground color rather than hard-coded black 
			puts "#{format_duration(dt)}"
		end
	rescue RuntimeError => ex
		puts ex
	end

	wait_for_minute
end

