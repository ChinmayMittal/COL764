from utils import parse_results_file, preprocess_meta_data_file, kl_divergence
from lm import DocumentLanguageModel, CombinedLanguageModel

query_file_path = './col764-ass2-release/covid19-topics.xml'
top_100_file_path = './col764-ass2-release/t40-top-100.txt'
meta_data_path = '/Users/chinmaymittal/Downloads/2020-07-16/metadata.csv'
document_dir_path = '/Users/chinmaymittal/Downloads/2020-07-16/'
output_file_path = './output.txt'

query_file = open(query_file_path, 'r')
output_file = open(output_file_path, 'w')    

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
    
    ### get LMs for all documents ranked for that query
    language_models = [DocumentLanguageModel(cord_uid=cord_uid, meta_data=meta_data, corpus_path=document_dir_path) for rank, cord_uid in query_results[query_number].items()]
    
    ### get background LM for that query
    combined_lm = CombinedLanguageModel()
    for lm in language_models:
        combined_lm.add_document(lm)
        
    ### add background LM to each individual model for Dirichilet smoothing
    for language_model in language_models:
        language_model.add_background_model(combined_lm)
        
    ### compute relevant model probabilities
    relevance_model_probabilities = dict()
    for word in combined_lm.word_counts.keys():
        relevance_model_probabilities[word] = 0
        for lm in language_models:
            relevance_model_probabilities[word] += lm.probability(word)
        relevance_model_probabilities[word] /= len(language_models)
    # print(sum(relevance_model_probabilities.values()))
    
    results = []
    for i in range(len(language_models)):
        results.append((i+1, 1-kl_divergence(language_models[i], relevance_model_probabilities)))
    
    results.sort(key=lambda x: x[1], reverse=True)
    for idx, result in enumerate(results):
        old_rank, score = result
        output_file.write(f"{query_number} Q0 {query_results[query_number][old_rank]} {idx+1} {score:.4f} chinmay\n")

    print(f"Processed Query Number: {query_number}\r")

output_file.close()
query_file.close()