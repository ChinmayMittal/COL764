#!/bin/bash

# Check if the correct number of arguments (4) are provided
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 [query-file] [top-100-file] [collection-dir] [output-file]"
    exit 1
fi

# Assign the arguments to meaningful variable names
query_file="$1"
top_100_file="$2"
collection_dir="$3"
output_file="$4"

# Check if the query file exists
if [ ! -f "$query_file" ]; then
    echo "Query file '$query_file' does not exist."
    exit 1
fi

# Check if the top 100 file exists
if [ ! -f "$top_100_file" ]; then
    echo "Top 100 file '$top_100_file' does not exist."
    exit 1
fi

# Check if the collection directory exists
if [ ! -d "$collection_dir" ]; then
    echo "Collection directory '$collection_dir' does not exist."
    exit 1
fi

# Perform the LM rerank operation with the provided arguments
echo "Running LM rerank with arguments:"
echo "Query file: $query_file"
echo "Top 100 file: $top_100_file"
echo "Collection directory: $collection_dir"
echo "Output file: $output_file"


python lm_rerank.py --query_file_path "$query_file" --original_rankings_path "$top_100_file" --corpus_path "$collection_dir" --output_path "$output_file"

