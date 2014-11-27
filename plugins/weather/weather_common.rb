require 'tmpdir'
require 'net/http'
require 'uri'

STDOUT.sync=true

def temp_file_name(base, extension)
	filename="#{Dir::tmpdir}/#{base}.#{extension}"
	num=0

	while File.exist? filename
		filename="#{Dir::tmpdir}/#{base}-#{num}.#{extension}"
		num+=1
	end

	filename
end

def download(url)
	Net::HTTP.get URI.parse(url)
end

def output_message(message)
	puts "[MSG] [#{message}]"
end

def output_image(file_name)
	puts "[IMG] [#{file_name}]"
end

def output_movie(file_name)
	puts "[MOV] [#{file_name}]"
end


