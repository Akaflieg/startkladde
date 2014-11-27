#!/usr/bin/env ruby

require 'timeout'
require 'thread'

host="localhost"
port=8905
timeout=10

command="netcat #{host} #{port}"

class NilClass
	def round
		nil
	end
end

# Extrahiert einen Float-Wert aus einer Zeile der Form
# …KEYxxx.yyy…. Gibt nil zurück, wenn die Zeile keinen
# solchen Wert enthält.
def extract(line, key)
	result=($1 if line =~ /#{key}([^,]*),/)
	result=result.to_f if result
	result
end

# Gibt die Daten einer Zeile in formatierter Form aus
# Eingabe: 17:48:36, 13.02.08, TE5.32, DR1018.96, WR45.33, FE78.68, WG1.61, WS2.85, WD2.16, WC5.32, WV45.45,
# Ausgabe: 45°, 3kt
def handle_line(line)

	# Geschwindigkeiten in m/s
	wind_direction=extract(line, "WR")
	wind_velocity =extract(line, "WG")
	#wind_gust     =extract(line, "WS")

	# Geschwindigkeiten in Knoten umrechnen
	wind_velocity_kt=(wind_velocity*2 if wind_velocity)
	#wind_gust_kt    =(wind_gust    *2 if wind_gust    )

	# Ausgabe formatieren
	display_direction=(  "#{wind_direction  .round}°"  if wind_direction)    || "?"
	display_velocity =(  "#{wind_velocity_kt.round}kt" if wind_velocity_kt ) || "?"
	#display_gust     =(" G#{wind_gust_kt    .round}kt" if wind_gust_kt     ) || ""

	#puts "#{display_direction}/#{display_velocity}#{display_gust}"
	puts "#{display_direction}, #{display_velocity}"
end


line          =nil
line_mutex    =Mutex.new
line_received =ConditionVariable.new


# Verarbeitungs-Thread
# Wartet darauf, dass eine Zeile empfangen wird und ruft handle_line auf. Wird
# für (timeout) Sekunden keine Zeile empfangen, wird eine entsprechende Meldung
# ausgegeben.
format_thread=Thread.start {
	line_mutex.synchronize {
		while true
			begin
				Timeout::timeout(timeout) {
					line_received.wait(line_mutex)

					# Synchronisiert, damit bei Verbindungsabbruch nicht noch
					# etwas ausgegeben wird
					handle_line(line)
				}
			rescue Timeout::Error
				puts "Keine Daten empfangen"
			end
		end
	}
}

puts "Verbindung zur Wetterstation wird aufgebaut..."

begin
	# Zeilen von dem angegebenen Kommando lesen und an den Empfänger-Thread senden
	connected=false
	IO.popen(command) { |io|
		while !io.eof?
			l=io.readline

			#  Es wurden Daten empfangen
			connected=true

			line_mutex.synchronize {
				line=l
				line_received.signal
			}
		end
	}

	line_mutex.synchronize {
		format_thread.exit

		# Meldung, je nachdem, ob schon Daten empfangen wurden
		if connected
			puts "Verbindung zur Wetterstation abgebrochen"
		else
			puts "Keine Verbindung zur Wetterstation"
		end

		exit 1
	}
rescue Interrupt
	puts "Verbindung zur Wetterstation abgebrochen"
end

