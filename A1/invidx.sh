#!/bin/bash

# Check the number of arguments
if [ $# -ne 4 ]; then
    echo "Usage: $0 [coll-path] [indexfile] Compression:{0|1} Tokenization:{0|1}"
    exit 1
fi

# Assign arguments to variables
coll_path=$1
indexfile=$2
compression=$3
tokenization=$4

# Check if arguments 3 and 4 are valid (0 or 1)
if [[ "$compression" != "0" && "$compression" != "1" ]]; then
    echo "Invalid argument: $compression"
    exit 1
fi

if [[ "$tokenization" != "0" && "$tokenization" != "1" ]]; then
    echo "Invalid argument: $tokenization"
    exit 1
fi

echo "Creating Inverted Index with the following arguments:"
echo "Collection Path: $coll_path"
echo "Index File: $indexfile"
echo "Compression Type: $compression"
echo "Tokenization: $tokenization"

#### create inverted index
./inverted_index "$coll_path" "$indexfile" "$compression" "$tokenization"


