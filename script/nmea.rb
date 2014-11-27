module Nmea
	def Nmea.calculate_checksum(data)
		sum=0
		data.each_byte { |x| sum ^= x }
		sprintf('%02X', sum)
	end

	# ddmm.ssss or dddmm.ssss
	def Nmea.parse_angle(string, degree_digits)
		(string[0..degree_digits-1].to_i + string[degree_digits..-1].to_f/60)*Degree
	end

	def Nmea.format_angle(angle, degree_digits)
		angle=angle/Degree;

		degrees=angle.truncate
		minutes=(angle-degrees)*60
		minutes_whole=minutes.to_i
		minutes_fractional=minutes-minutes_whole

		raise 'minutes_fractional not less than 1' if minutes_fractional>=1

		degrees=sprintf("%0#{degree_digits}d", degrees)
		minutes_whole=sprintf('%02d', minutes_whole)
		minutes_fractional=sprintf('%f', minutes_fractional)

		degrees+minutes_whole+minutes_fractional[1..-1] # remove the leading zero of minutes_fractional
	end

	def Nmea.parse_latitude  (latitude ); parse_angle(latitude  , 2); end
	def Nmea.parse_longitude (longitude); parse_angle(longitude , 3); end
	def Nmea.format_latitude (latitude) ; format_angle(latitude , 2); end
	def Nmea.format_longitude(longitude); format_angle(longitude, 3); end

	def Nmea.split_sentence(line)
		if line =~ /^ \$ ([^*]*) \* (..) $/x
			payload=$1
			checksum=$2

			if checksum == calculate_checksum(payload)
				payload.split(',')
			else
				nil
			end

		else
			nil
		end
	end

	def Nmea.join_sentence(parts)
		sentence=parts.join(',')
		"$"+sentence+"*"+calculate_checksum(sentence)
	end
end

