import os
import sys
from utils import get_files, parse_file, simple_progress_bar
from tokenizer import SimpleTokenizer
from collections import Counter
from constants import DELIMITERS

if len(sys.argv) <= 1:
    print("Requires Training Directory as Script Argument")
    exit()

directory_path = sys.argv[1]
print(f"Inverting Collection at {directory_path}")

files = get_files(directory_path=directory_path)

postings_list = {}
document_ids_to_idx = {}
tokenizer = SimpleTokenizer(delimiters=DELIMITERS)
document_cnt = 0
vocabulary = set()

for file in simple_progress_bar(files):
    documents = parse_file(os.path.join(directory_path, file))
    for document in documents:
        document_ids_to_idx[document["DOCID"]] = document_cnt
        document_string = (document["TITLE"] + document["CONTENT"]).lower()
        unique_tokens = set(tokenizer.tokenize(document_string))
        for token in unique_tokens:
            vocabulary.add(token)
        document_cnt += 1

print(len(vocabulary))
        
        



