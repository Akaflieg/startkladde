#!/usr/bin/env ruby

$: << File.dirname($0) # Load files from the script directory

require 'weather_common'
require 'tmpdir'

begin
	navigation_page_url='http://www.wetteronline.de/include/radar_dldl_00_dwddgf.htm'

	# Download the navigation page
	output_message "Radarfilm wird heruntergeladen (1)..."
	navigation_page=download(navigation_page_url)
	raise "Fehler beim Abrufen der Radarseite" if !navigation_page

	# Extract the radar page URL (the second link before "Loop 3 Stunden")
	if navigation_page =~ /a href.*a href="\/([^"]*)".*Loop 3 Stunden/
		radar_page_url="http://www.wetteronline.de/#{$1}"
	else
		raise "Auf der Navigationsseite wurde kein Link zur Animation gefunden"
	end

	# Download the radar page
	output_message "Radarfilm wird heruntergeladen (2)..."
	radar_page=download(radar_page_url)
	raise "Fehler beim Abrufen der Radarseite" if !radar_page

	# Extract the radar image URL
	#radar_image_url=|grep gif |grep vie |sed 's/^.*src="\([^"]*\)".*$/\1/'`
	if radar_page =~ /(daten\/radar[^"]*)"/
		image_url="http://www.wetteronline.de/#{$1}"
	else
		raise "Auf der Wetterseite wurde keine Wetteranimation gefunden"
	end

	# Download the animation
	output_message "Radarfilm wird heruntergeladen (3)..."
	image=download(image_url)
	raise "Fehler beim Abrufen der Wettergrafik" if !image

	# Store the animation
	# Use a fixed name - don't clutter the directory.
	image_file_name="#{Dir::tmpdir}/regenradar_wetteronline.de_animation.gif"
	File.open(image_file_name, "wb") { |file| file.print image }

	# Speed up the animation
	output_message "Radarfilm wird konvertiert"
	system("mogrify -delay 50 #{image_file_name}")


	output_movie image_file_name
rescue RuntimeError => ex
	puts output_message ex.to_s
end

