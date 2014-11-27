#!/usr/bin/env ruby

require 'net/http'
require 'uri'

STDOUT.sync=true

# Determine the airport and exit if it is not specified
airport=ARGV[0]
(puts "Kein Flugplatz angegeben"; exit 1) if !airport || airport.strip.empty?

# mgetmetar, mgettaf, mgetstaf
url="http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc=#{airport}"

# Get the page
puts "Rufe METAR für #{airport} ab..."
reply=Net::HTTP.get URI.parse(url)

# The relevant section of the page:
#
#<font face="courier" size = "5">
#EDDF 311920Z 26009KT 9999 FEW012 SCT019 BKN023 13/11 Q1015 NOSIG
#
#</font>
#

# Find the first line that begins with the airport code in upper case
report=reply.split(/\r?\n/).find { |line| line =~ /^#{airport.upcase}/ }

# Print the line or an error message
if report
	puts report
else
	puts "Keine Wettermeldung für #{airport} gefunden"
end

