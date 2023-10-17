import os
from constants import (
            LOWERCASE,
            PUNCTUATIONS,
            DIGITS,
            STEMMING,
            STOPWORDS_ELIMINATION
        )
from utils import (
        simple_progress_bar,
        parse_results_file,
        preprocess_meta_data_file,
        get_text_from_cord_id,
        preprocess_text
    )

query_file_path = './col764-ass2-release/covid19-topics.xml'
top_100_file_path = './col764-ass2-release/t40-top-100.txt'

document_dir_path = '/Users/chinmaymittal/Downloads/2020-07-16/'
meta_data_path = os.path.join(document_dir_path, "metadata.csv")

output_dir = './relevance-corpus'

query_file = open(query_file_path, 'r')
 

## query_number -> rank -> cord_id
query_results = parse_results_file(results_file_path=top_100_file_path)
query_numbers = list(query_results.keys())

### all the cord_uids required from all the queries
cord_uids = set()
for query_number in query_numbers:
    for cord_uid in query_results[query_number].values():
        cord_uids.add(cord_uid)
meta_data = preprocess_meta_data_file(cord_uids=list(cord_uids), meta_data_path=meta_data_path)

for query_number in simple_progress_bar(query_numbers):
    ### get the text corpus, from the pseudo relevance set of this query
    
    pseudo_relevance_text_file = open(os.path.join(output_dir, f"temp-query-{query_number}.txt"), 'w')
    
    for cord_uid in query_results[query_number].values():
        text_list = get_text_from_cord_id(cord_uid=cord_uid, meta_data=meta_data, corpus_dir_path=document_dir_path)
        for text in text_list:
            pseudo_relevance_text_file.write(preprocess_text(text, lowercase=LOWERCASE, punctuations=PUNCTUATIONS, digits=DIGITS, stemming=STEMMING, stopwords_elimination=STOPWORDS_ELIMINATION) + ' ')

    pseudo_relevance_text_file.close()
query_file.close()