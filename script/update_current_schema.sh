#!/bin/bash

function abort
{
	echo "$1"
	echo "Stop"
	exit 1
}

# Use a temporary file in the build directory, so if the dumping fails, we don't
# overwrite the schema

# Call from the build directory
if [ ! -e ./startkladde ]; then
	echo "Call from the build directory and build the program first"
	exit 1
fi

make || (echo "Error during make"; exit 1)

./startkladde db:ensure_empty                 || abort "Error: database is not empty - use ./startkladde db:clear"
./startkladde db:migrate                      || abort "Error during migration"
./startkladde db:dump current_schema.yaml.new || abort "Error during dump"

echo "Now move current_schema.yaml.new to src/db/migrations/current_schema.yaml in the source directory."

