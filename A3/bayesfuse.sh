#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 [collection-file] [output-file]"
    exit 1
fi

# Assign arguments to variables
collection_file=$1
output_file=$2

# Check if the collection file exists
if [ ! -f "$collection_file" ]; then
    echo "Error: Collection file '$collection_file' does not exist."
    exit 1
fi

python bayes-fuse.py "$collection_file" "$output_file"
