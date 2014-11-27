require 'date'
require 'time'

def find_file(name)
	if File.exist?(name)
		File.new(name)
	elsif File.exist?("#{File.dirname($0)}/#{name}")
		File.new("#{File.dirname($0)}/#{name}")
	else
		nil
	end
end

def find_sunset(date, file)
	if file.readlines.find { |line| line =~ /^#{date}\s*(.*)/ }
		$1
	else
		nil
	end
end

# result will be regardless of argument sign
# arguments >=1day will fail
def format_duration(seconds, format="%H:%M")
	Time.at(seconds.abs).getutc.strftime(format)
end

def truncate_seconds(time)
	Time.parse("#{time.getutc.strftime("%H:%M")}Z")
end

# Wait until the next minute
def wait_for_minute
	sleep (60-Time.now.sec)+0.5
end


def determine_sunset
	# If no file name is passed on the command line, use a default
	filename=ARGV[0] || "sunsets.txt"

	# Find the file with the given name
	file=find_file(filename)
	raise "Datei #{filename} nicht gefunden" if !file

	# Determine the current date
	current_date=Date.today.strftime('%m-%d')

	# Determine the sunset time
	sunset=find_sunset(current_date, file)
	raise "Datum #{current_date} nicht in Datendatei vorhanden" if !sunset

	begin
		Time.parse("#{sunset}Z")
	rescue ArgumentError
		raise "Ungültige Zeit #{sunset}"
	end
end

