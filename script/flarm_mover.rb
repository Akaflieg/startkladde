#!/usr/bin/env ruby
$: << File.dirname($0)

# Processes a Flarm NMEA file, adding artificial movement to the receiver
# position. The receiver position is moved by an offset that moves on a
# circular path. Therefore, a stationary Flarm will appear to move on a circle,
# while a Flarm moving on a straight path will appear to move on a spiral path.
# That way, the noise on the position is preserved.
# Currently, the GPRMC and PFLAA sentences are processed, so the absolute
# positions of other planes are not changed. All other sentences are not
# modified (in particular, GPGGA support is still missing).
#
# Call without arguments for usage instructions.

require 'nmea'

class Dta # Data is already defined
	attr_accessor :radius, :speed
	attr_accessor :angular_speed
	attr_accessor :angle, :relative_north, :relative_east
end

data=Dta.new

data.radius=200 # m
data.speed=10 # m/s

data.angular_speed=data.speed.to_f/data.radius

data.angle=0 # radians
data.relative_north=0 # m
data.relative_east=0 # m

NauticalMile=1852 # m
Degree=Math::PI/180;

def process_file(infile, outfile, data)
	infile.each_line { |line|
		parts = Nmea.split_sentence(line)

		if parts.nil?
			output=line
		elsif parts[0]=='GPRMC'
			# Calculate the new relative position
			data.angle+=data.angular_speed # Assume 1 second interval between GPRMC sentences
			data.angle-=2*Math::PI if data.angle>=2*Math::PI
			data.relative_north=data.radius*Math.sin(data.angle)
			data.relative_east =data.radius*Math.cos(data.angle)

			# Parse the sentence
			latitude =Nmea.parse_latitude( parts[3])
			longitude=Nmea.parse_longitude(parts[5])

			# Calculate the new absolute position
			latitude +=data.relative_north / (60*NauticalMile) * Degree
			longitude+=data.relative_east  / (60*NauticalMile) * Degree / Math.cos(latitude)

			# Format the sentence
			parts[3]=Nmea.format_latitude(latitude)
			parts[5]=Nmea.format_longitude(longitude)

			output=Nmea.join_sentence(parts)
		elsif parts[0]=='PFLAA'
			# Parse the sentence
			flarm_relative_north=parts[2].to_i
			flarm_relative_east =parts[3].to_i

			# Calculate the new relative positions
			flarm_relative_north-=data.relative_north
			flarm_relative_east -=data.relative_east

			# Format the sentence
			parts[2]=flarm_relative_north.to_i.to_s
			parts[3]=flarm_relative_east.to_i.to_s

			output=Nmea.join_sentence(parts)
		# TODO handle GPGGA sentences
		else
			output=line
		end

		outfile.puts output if outfile
	}

end

infilename=ARGV[0]
outfilename=ARGV[1]

if infilename.nil? || outfilename.nil?
	puts "Usage: #{$0} infilename outfilename"
	puts "outfilename may by \"-\" to indicate stdout or empty for no output"
	exit 1
end

infile=File.new(infilename)

if outfilename=='-'
	outfile=STDOUT
elsif outfilename==""
	outfile=nil
else
	outfile=File.new(outfilename, 'w')
end

process_file infile, outfile, data

