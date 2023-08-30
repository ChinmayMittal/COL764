#!/bin/bash

# Define the URL of the tar.gz file
url="https://github.com/zeux/pugixml/releases/download/v1.13/pugixml-1.13.tar.gz"

# Define the target filename
filename="pugixml-1.13.tar.gz"

# Define the target directory where you want to extract the contents
extract_dir="pugixml-1.13"

# Download the tar.gz file
echo "Downloading $filename..."
curl -LO "$url"

# Check if the download was successful
if [ $? -ne 0 ]; then
    echo "Download failed. Exiting..."
    exit 1
fi

# Extract the contents of the tar.gz file
echo "Extracting $filename..."
tar -xzvf "$filename"

# Check if the extraction was successful
if [ $? -ne 0 ]; then
    echo "Extraction failed. Exiting..."
    exit 1
fi

# Move files from the 'src' folder to the script's directory
echo "Moving files from 'src' folder..."
mv "$extract_dir"/src/* .

rm -r "$extract_dir"
rm "$filename"

echo "Extraction and moving completed."
make my_index
make my_search

