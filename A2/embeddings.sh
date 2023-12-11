#!/bin/bash

# Define the folder path
folder_path="./relevance-corpus"

echo "Generating Relevance Corpus"

# Loop through files with the pattern "temp-query-i.txt"
for file in "$folder_path"/temp-query-*.txt; do
  if [ -f "$file" ]; then
    # Extract the 'i' value from the file name
    i=$(basename "$file" | sed 's/temp-query-\(.*\)\.txt/\1/')
    
    echo "Processing $file"

    time ./word2vec/word2vec -train "$file" -output "./vectors/temp-vectors-$i.txt" -cbow 1 -size 100 -window 8 -negative 25 -hs 0 -sample 1e-4 -threads 20 -binary 0 -iter 15
    sed 's/[^[:print:]]/-/g' "./vectors/temp-vectors-$i.txt" > "./vectors/vectors-$i.txt" ## remove non-printable characters if any
    rm "./vectors/temp-vectors-$i.txt"
  fi
done
