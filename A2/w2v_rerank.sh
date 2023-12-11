#!/bin/bash

# Script Name: w2v_rerank.sh

# Check if correct number of arguments is provided
if [ "$#" -ne 5 ]; then
    echo "Usage: $0 [query-file] [top-100-file] [collection-dir] [output-file] [expansions-file]"
    exit 1
fi

# Assign names to arguments to facilitate their usage
query_file="$1"
top_100_file="$2"
collection_dir="$3"
output_file="$4"
expansions_file="$5"

# Function to check if a file exists
check_file_exists() {
    if [ ! -f "$1" ]; then
        echo "Error: File $1 does not exist."
        exit 1
    fi
}

# Function to check if a directory exists
check_dir_exists() {
    if [ ! -d "$1" ]; then
        echo "Error: Directory $1 does not exist."
        exit 1
    fi
}

# Perform checks on the provided files and directories
check_file_exists "$query_file"
check_file_exists "$top_100_file"
check_dir_exists "$collection_dir"
# Note: For output file and expansions file, we're not checking if they exist,
# as they may be files that will be created later in your actual processing script.

# If all checks pass, proceed with the actual processing. 
echo "All input files and directories exist. Proceeding with re-ranking..."
mkdir ./relevance-corpus
python generate.py --query_file_path "$query_file" --original_rankings_path "$top_100_file" --corpus_path "$collection_dir" --output_dir ./relevance-corpus
echo "Created Relevance Corpus"

mkdir ./vectors
./embeddings.sh
echo "Learnt Word2Vec Embeddings"


python w2v_rerank.py --query_file_path "$query_file" --original_rankings_path "$top_100_file" --corpus_path "$collection_dir" --output_path "$output_file" --expansions_path "$expansions_file" --vectors_dir ./vectors/

echo "Re-ranking completed successfully. Results are saved in $output_file. Expansions used during the re-ranking are logged in $expansions_file."

exit 0
