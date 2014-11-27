#!/usr/bin/env ruby

# TODO:
#   * integrate chef, the original sometimes outputs more than one line

require 'cgi'
require 'pty'
require 'tempfile'

# See http://doc.trolltech.com/qq/qq03-swedish-chef.html

##########
## Util ##
##########

class NilClass; def blank?; true  ; end; end
class String  ; def blank?; empty?; end; end

class String
	def map_lines
		self.split(/\n\r?/).map { |line| yield(line) }.join("\n")
	end
end

def copyFileContents(source, target)
	File.open(target, "w") { |file|
		file.print File.read(source)
	}
end


###############
## Outputter ##
###############

class DiffOutputter
	def output(lines); puts lines.map_lines { |line| "+++"+line }; end
	def copy  (lines); puts lines                                ; end
	def eat   (lines); puts lines.map_lines { |line| "---"+line }; end
	def close; end
end

class PlainOutputter
	def output(lines); puts lines; end
	def copy  (lines); puts lines; end
	def eat   (lines); end
	def close; end
end

class FileOutputter
	def initialize(filename)
		@file=File.open(filename, 'w')
	end

	def output(lines); @file.puts lines; end
	def copy  (lines); @file.puts lines; end
	def eat   (lines); end
	def close; @file.close; end
end

class QuietOutputter
	def output(lines); end
	def copy  (lines); end
	def eat   (lines); end
	def close; end
end


###################
## String mocker ##
###################

class UpcaseStringMocker
	def mock(string)
		string.upcase
	end

	def close
	end
end

class ProcessMocker
	def initialize(command)
		@read,@write,@pid=PTY.spawn(command)
	end

	def mock(string)
		string.lines.map { |line|
			@write.puts line
			@read.readline.chomp
			result=@read.readline.chomp
			result
		}.join("\n")
	end

	def close
		@read.close
		@write.close
	end
end

# A wrapper for another String mocker, for filtering out XML entities, HTML
# tags, %n and & mnemonics
# Currently simply does not mock strings containing &
class QtStringMocker
	def initialize(mocker)
		@mocker=mocker
	end

	def mock(string)
		string=CGI.unescapeHTML(string)

		# TODO mnemonics (e. g. File&name)
		re=/
			([^<%]+) # A text - anything not containing a < or %
		|
			(< [^>]* >) # An HTML tag
		|
			(%.) # % followed by a single character
		/x
		string=string.scan(re).map { |text, html_tag, percent|
			if text
				#puts "    Text: #{text.inspect}"
				@mocker.mock(text)
			elsif html_tag
				#puts "    HTML: #{html_tag.inspect}"
				html_tag
			elsif percent
				#puts "    Percent: #{html_tag.inspect}"
				percent
			else
				"???"
			end
		}.join

		CGI.escapeHTML(string)
	end

	def close
		@mocker.close
	end
end


#################
## File mocker ##
#################

# Parses and mocks a .ts file
class TsFileMocker
	def initialize(stringMocker, outputter)
		@stringMocker=stringMocker
		@outputter=outputter

		@sourceString=''
		@inSource=false
		@inTranslation=false
		@numerusMessage=false
	end

	def processLine(line)
		translationStart=false
		translationEnd=false

		if line =~ /^\s*<message/
			# New message - reset message state
			@sourceString=''
			@inSource=false
			@inTranslation=false
			# Determine whether it is a numerus message
			@numerusMessage=!((line =~ /numerus="yes"/).nil?)

		elsif line =~ /^\s*<source>(.*)<\/source>\s*$/
			# Single-line source string
			@sourceString=$1
		elsif line =~ /^\s*<source>(.*)$/m
			# Start of multi-line source string
			@sourceString=$1
			@inSource=true
		elsif line =~ /(.*)<\/source>\s*$/m
			# End of multi-line translation
			@sourceString+=$1
			@inSource=false
		elsif @inSource
			# Middle of multi-line source string
			@sourceString+=line

		elsif line =~ /^\s*<translation.*type="unfinished".*>.*<\/translation>\s*$/
			# Single-line translation (we're only interested in 'unfinished' ones)
			translationStart=true
			translationEnd=true
		elsif line =~ /^\s*<translation.*type="unfinished"/
			# Start of multi-line translation
			translationStart=true
			@inTranslation=true
		elsif line =~ /<\/translation>\s*$/ and @inTranslation
			# End of multi-line translation - only if we're in a translation,
			# because a translation without type=unfinished does not count as a
			# translation
			translationEnd=true
			@inTranslation=false
		end

		# We may not be in a source and in a translation simultaneously
		raise "inSource and inTranslation" if @inSource && @inTranslation

		# Output new text and determine whether to copy the line. The line is
		# not copied immediately because we want to be able to output ---line
		# if it is not copied, this is great for development.
		copyLine=false
		if translationStart
			# This covers both single and multi-line messages
			# TODO check multi-line numerus message
			if @numerusMessage
				singularGuess=@sourceString.gsub('(s)', '')
				pluralGuess  =@sourceString.gsub('(s)', 's')
				@outputter.output "        <translation type=\"unfinished\">"
				@outputter.output "            <numerusform>#{@stringMocker.mock(singularGuess)}</numerusform>"
				@outputter.output "            <numerusform>#{@stringMocker.mock(pluralGuess)}</numerusform>"
				@outputter.output "        </translation>"
			else
				@outputter.output "        <translation type=\"unfinished\">#{@stringMocker.mock(@sourceString)}</translation>"
			end
		elsif !@inTranslation && !translationEnd
			copyLine=true
		end

		# Copy the line if we determined to do so before
		if copyLine
			@outputter.copy line
		else
			@outputter.eat line
		end
	end

	def processFile(filename)
		# line includes the trailing newline, this is important for assembling
		# multi-line messages
		File.open(filename).each { |line|
			self.processLine(line)
		}
	end
end


##########
## Test ##
##########

def testMocker
	mocker=UpcaseStringMocker.new
	mocker=QtStringMocker.new(mocker)
	[
		["foo"                                  , "FOO"                                  ],
		["foo&amp;bar&amp;"                     , "FOO&amp;BAR&amp;"                     ],
		["foo&lt;font&gt;bar"                   , "FOO&lt;font&gt;BAR"                   ],
		["foo&lt;/font&gt;bar"                  , "FOO&lt;/font&gt;BAR"                  ],
		["foo&lt;font size=&quot;x&quot;&gt;bar", "FOO&lt;font size=&quot;x&quot;&gt;BAR"],
		["%n foo(s)"                            , "%n FOO(S)"                            ],
		["", ""]
	].each { |source, expected|
		actual=mocker.mock(source)
		if actual==expected
			puts "OK - #{source} - #{actual}"
		else
			puts "fail - #{source} - #{actual}, expected #{expected}"
		end
	}
end


##########
## Main ##
##########

def help
	puts "Usage:"
	puts "  #{$0} filter [output] filename.ts"
	puts "  #{$0} --test"
	puts ""
	puts "filter is one of:"
	puts "  --upcase"
	puts "    Use a simple upcase mock filter"
	puts "  --filter|-f command"
	puts "    Use the specified command as mock filter. The filter must output exactly one line for each input line."
	puts ""
	puts "output is one of:"
	puts "  --output|-o filename"
	puts "    Write to the specified file, stdout if filename is \"-\" or don't write if filename is empty."
	puts "  --diff"
	puts "    Show a unified diff"
	puts "  --inplace"
	puts "    Overwrite the file in place"
	puts "If output is not specified, the file with changes is output on stdout."
	puts ""
	puts "--test runs the integrated unit tests"
	exit 1
end


# Parse parameters
upcase =false
test   =false
diff   =false
inplace=false
filenames=[]
while arg=ARGV.shift
	case arg
	when '--test'   : test   =true
	when '--diff'   : diff   =true
	when '--inplace': inplace=true
	when '--upcase' : upcase =true
	when '-f' , '--filter': filter=ARGV.shift; if filter.nil?; puts "Filter missing"; help; end
	when '-o' , '--output': output=ARGV.shift; if output.nil?; puts "Output missing"; help; end
	when /^-/: puts "Unknown parameter #{arg}"; help
	else filenames << arg
	end
end

if test
	testMocker
	exit 0
end

if filenames.empty?
	help
elsif filenames.size>1
	puts "Too many file specified"
	exit 1
end

filename=filenames[0]
if !File.exist?(filename)
	puts "File #{filename} does not exist"
	exit 1
end

# Create the string mocker
if filter
	stringMocker=ProcessMocker.new(filter)
elsif upcase
	stringMocker=UpcaseStringMocker.new
else
	puts "No mocker specified"
	exit 1
end
stringMocker=QtStringMocker.new(stringMocker)

# Create the outputter
if diff
	# Output to temp file and show diff
	tempfile=Tempfile.new('mock')
	outputter=FileOutputter.new(tempfile.path)
elsif inplace
	# Output to temp file and overwrite original
	tempfile=Tempfile.new('mock')
	outputter=FileOutputter.new(tempfile.path)
elsif output.nil?
	# Show file with differences
	outputter=DiffOutputter.new
elsif output.empty?
	# Don't show anything
	outputter=QuietOutputter.new
elsif output=="-"
	# Show the result on standard output
	outputter=PlainOutputter.new
else
	# Write the result to the output file
	outputter=FileOutputter.new(output)
end

# Mock the file
fileMocker=TsFileMocker.new(stringMocker, outputter)
fileMocker.processFile(filename)
outputter.close

# Post processing
if diff
	system "diff", "-u", filename, tempfile.path
	tempfile.unlink
elsif inplace
	# Not using FileUtils.mv because that needs to change the permissions and
	# this may not be possible on a shared filesystem.
	copyFileContents tempfile.path, filename
	tempfile.unlink
end

