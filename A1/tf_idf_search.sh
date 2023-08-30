#!/bin/bash

# Check the number of arguments
if [ $# -ne 4 ]; then
    echo "Usage: $0 [queryfile] [resultfile] [indexfile] [dictfile]"
    exit 1
fi

# Assign arguments to variables
queryfile=$1
resultfile=$2
indexfile=$3
dictfile=$4

# Check if index and dictionary files exist
if [ ! -f "$indexfile" ]; then
    echo "Index file '$indexfile' not found."
    exit 1
fi

if [ ! -f "$dictfile" ]; then
    echo "Dictionary file '$dictfile' not found."
    exit 1
fi

# Perform the TF-IDF search using your script or command
# Replace the following line with your actual search logic
./search "$queryfile" "$resultfile" "$indexfile" "$dictfile"

echo "Search completed. Results saved to '$resultfile'."
