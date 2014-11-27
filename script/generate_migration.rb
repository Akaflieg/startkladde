#!/usr/bin/env ruby

require 'erb'

force=false
timestamp=nil
name=""

args=ARGV.dup
#ARGV.each do |arg|

while arg=args.shift
	case arg
	when "-f", "--force"     then force=true
	when "-t", "--timestamp" then timestamp=args.shift
	when /^-/                then puts "Unknown argument: #{arg}"; exit 1
	else name=arg
	end
end

if name.empty?
	puts "Usage: #{$0} [-f|--force] [-t|--timestamp timestamp] name"
	puts "  name: the name of the migration, without the version - use underscore_notation"
	puts "  timestamp: 14 digits; the current time is used if not specified"
	puts "Example: #{$0} add_towpilot"
	exit 1
end

timestamp ||= Time.now.utc.strftime "%Y%m%d%H%M%S"

unless timestamp =~ /\d\d\d\d\d\d\d\d\d\d\d\d\d\d/
	puts "Invalid timestamp #{timestamp} - must be 14 digits"
	exit 1
end


dir="src/db/migrations"

source_template="#{dir}/Migration.cpp.erb"
header_template="#{dir}/Migration.h.erb"

full_name="#{timestamp}_#{name}"
class_name="Migration_#{full_name}"

source="#{dir}/Migration_#{timestamp}_#{name}.cpp"
header="#{dir}/Migration_#{timestamp}_#{name}.h"

if !File.directory?(dir)
	puts "Directory #{dir} does not exist - this script must be called from the root"
	exit 1
end

(puts "Template #{source_template} does not exist"; exit 1) if !File.file?(source_template)
(puts "Template #{header_template} does not exist"; exit 1) if !File.file?(header_template)

if !force && !Dir["#{dir}/Migration_*_#{name}.h"].empty?
	puts "There is already a migration with the same name (#{name}) - specify --force to"
	puts "generate the migration anyway (the existing migration will not be overwritten)."
	puts "Note that the program will not compile if the migrations with the same name"
	puts "exist, even if their versions are different."
	exit 1
end

(puts "#{source} exists - specify --force to overwrite"; exit 1) if (File.exist?(source) && !force)
(puts "#{header} exists - specify --force to overwrite"; exit 1) if (File.exist?(header) && !force)

def write(template, output)
	puts "Writing #{output}"
	File.open output, "w" do |file|
		file.write ERB.new(File.read(template), nil, "%>").result
	end
end

write source_template, source
write header_template, header

puts "All done. You will now have to implement the migration and then call"
puts "'../path_to_source_directory/script/update_current_schema.sh' from the"
puts "build directory, with a configured and cleared database, to update the"
puts "schema definition."
