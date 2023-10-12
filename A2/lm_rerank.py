from utils import parse_results_file, preprocess_meta_data_file
from lm import DocumentLanguageModel, CombinedLanguageModel

query_file_path = './col764-ass2-release/covid19-topics.xml'
top_100_file_path = './col764-ass2-release/t40-top-100.txt'
meta_data_path = '/Users/chinmaymittal/Downloads/2020-07-16/metadata.csv'
document_dir_path = '/Users/chinmaymittal/Downloads/2020-07-16/'

query_file = open(query_file_path, 'r')

## query_number -> rank -> cord_id
query_results = parse_results_file(results_file_path=top_100_file_path)
query_numbers = list(query_results.keys())

cord_uids = set()
for query_number in query_numbers:
    for cord_uid in query_results[query_number].values():
        cord_uids.add(cord_uid)
meta_data = preprocess_meta_data_file(cord_uids=list(cord_uids), meta_data_path=meta_data_path)


for query_number in query_numbers:
    ### process queries
    language_models = [DocumentLanguageModel(cord_uid=cord_uid, meta_data=meta_data, corpus_path=document_dir_path) for rank, cord_uid in query_results[query_number].items()]
    combined_lm = CombinedLanguageModel()
    for lm in language_models:
        combined_lm.add_document(lm)
    
    print(combined_lm.word_counts['virus'])
    print(combined_lm.word_counts['origin'])
    print(len(combined_lm.word_counts))
    print(combined_lm.total_count)


query_file.close()