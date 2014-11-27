#!/usr/bin/env ruby

$: << File.dirname($0) # Load files from the script directory

require 'sunset_common'

begin
	puts determine_sunset.strftime("%H:%M")
rescue RuntimeError => ex
	puts ex
end

