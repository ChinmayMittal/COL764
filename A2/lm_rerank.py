import os
import argparse
from utils import parse_results_file, preprocess_meta_data_file, kl_divergence, preprocess_text
from lm import DocumentLanguageModel, CombinedLanguageModel
import xml.etree.ElementTree as ET
from constants import (
            UNK_PERCENTAGE,
            LOWERCASE,
            PUNCTUATIONS,
            DIGITS,
            STEMMING,
            STOPWORDS_ELIMINATION
        )

parser = argparse.ArgumentParser(description="Pseudo Relevance Based Language Modelling Reranking (Based on Lavrenko and Croft)")
parser.add_argument('--query_file_path', required=True, type=str,
                        help="Path to the a file containing topics in XML")
parser.add_argument('--original_rankings_path', required=True, type=str,
                    help='Path to the original Rankings for each query in TREC format')
parser.add_argument('--corpus_path', required=True, type=str,
                    help='Path to the Corpus of all Documents')
parser.add_argument('--output_path', required=True, type=str,
                    help='Path to the Output file, which will contain Reranked Documents')

args = parser.parse_args()
query_file_path = args.query_file_path
top_100_file_path = args.original_rankings_path
document_dir_path = args.corpus_path
output_file_path = args.output_path
meta_data_path = os.path.join(document_dir_path, 'metadata.csv')


query_file = open(query_file_path, 'r')
output_file = open(output_file_path, 'w')    


tree = ET.parse(query_file_path)
root = tree.getroot()
topics = dict()
for topic in root.findall('topic'):
    topic_number = topic.get('number')
    query = topic.find('query').text
    question = topic.find('question').text
    narrative = topic.find('narrative').text
    
    # Print or process the extracted information as needed
    topics[int(topic_number)] = {
        "query": query,
        "question": question,
        "narrative": narrative
    }


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
    query = (topics[int(query_number)]['query'])
    query = preprocess_text(query, lowercase=LOWERCASE, punctuations=PUNCTUATIONS, digits=DIGITS, stemming=STEMMING, stopwords_elimination=STOPWORDS_ELIMINATION )
    
    ### get LMs for all documents ranked for that query
    language_models = [DocumentLanguageModel(cord_uid=cord_uid, meta_data=meta_data, corpus_path=document_dir_path) for rank, cord_uid in query_results[query_number].items()]
    
    ### get background LM for that query
    combined_lm = CombinedLanguageModel()
    for lm in language_models:
        combined_lm.add_document(lm)
    combined_lm.add_unk(percentage=UNK_PERCENTAGE)
    query_terms = [query_term if query_term in combined_lm.word_counts.keys() else '<UNK>' for query_term in query.split()]
    print(query_terms)
        
    ### add background LM to each individual model for Dirichilet smoothing
    for language_model in language_models:
        language_model.add_background_model(combined_lm)
        
   
    query_probability_total = 0.0
    for lm in language_models:
       query_probability = 1
       for query_term in query_terms:
           query_probability *= lm.probability(query_term)
       query_probability_total += query_probability
    query_probability_total /= len(language_models)

    ### compute relevant model probabilities
    relevance_model_probabilities = dict()
    for word in combined_lm.word_counts.keys():
        relevance_model_probabilities[word] = 0
        for lm in language_models:            
            accumulator = lm.probability(word)
            for query_term in query_terms:
                accumulator *= lm.probability(query_term)
            relevance_model_probabilities[word] += accumulator
        relevance_model_probabilities[word] /= len(language_models)
        relevance_model_probabilities[word] /= query_probability_total
    print(sum(relevance_model_probabilities.values()))
    
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