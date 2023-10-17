import nltk
nltk.download('stopwords')

from constants import (
            LOWERCASE,
            PUNCTUATIONS,
            DIGITS,
            STEMMING,
            STOPWORDS_ELIMINATION,
            W2V_LAMBDA
        )
import numpy as np
from collections import Counter
import xml.etree.ElementTree as ET
from nltk.corpus import stopwords
from constants import TOP_K
from lm import DocumentLanguageModel, CombinedLanguageModel
from utils import parse_results_file, preprocess_meta_data_file, kl_divergence_reverse, preprocess_text


expansions_file_path = "./expansions.txt"
top_100_file_path = './col764-ass2-release/t40-top-100.txt'
meta_data_path = '/Users/chinmaymittal/Downloads/2020-07-16/metadata.csv'
document_dir_path = '/Users/chinmaymittal/Downloads/2020-07-16/'
output_file_path = "./output-w2v.txt"

stop_words = set(stopwords.words('english'))

tree = ET.parse('./col764-ass2-release/covid19-topics.xml')
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

def parse_embedding_file(file_path):
    word_to_index = {}
    embeddings = []
    index_to_word = {}
    
    with open(file_path, 'r') as file:
        num_words, num_dimensions = map(int, file.readline().split())
        try:
            for i, line in enumerate(file):
                parts = line.split()
                word = parts[0]
                embedding = np.array(list(map(float, parts[1:])))
                word_to_index[word] = i
                index_to_word[i] = word
                embeddings.append(embedding/np.sqrt(np.sum(embedding**2))) ### normalize embeddings
        except Exception as e:
            print(file_path)
            exit()
    embeddings_array = np.array(embeddings)    
    return embeddings_array, word_to_index, index_to_word

## query_number -> rank -> cord_id
query_results = parse_results_file(results_file_path=top_100_file_path)
query_numbers = list(query_results.keys())

cord_uids = set()
for query_number in query_numbers:
    for cord_uid in query_results[query_number].values():
        cord_uids.add(cord_uid)
meta_data = preprocess_meta_data_file(cord_uids=list(cord_uids), meta_data_path=meta_data_path)

expansions_file = open(expansions_file_path, 'w')
output_file = open(output_file_path, 'w')
for topic_idx, topic in topics.items():
        expansions_file.write(f"{topic_idx}: ")
        embeddings, word_to_index, index_to_word = parse_embedding_file(f"./vectors/vectors-{topic_idx}.text")
        V, k = embeddings.shape ### embeddings is |V| * |k| where k is the dimension of the word embeddings, |V| is vocabulary size
        # query = ' '.join([word for word in (topic['query']).split() if word not in stop_words])
        query = preprocess_text(topic['query'], lowercase=LOWERCASE, punctuations=PUNCTUATIONS, digits=DIGITS, stemming=STEMMING, stopwords_elimination=STOPWORDS_ELIMINATION)
        print(query)

        query_vector = np.zeros(shape=(V,1)) ### binary vector representing terms present in query
        for vocab_term, idx in word_to_index.items():
            if vocab_term.lower() in query.lower().split():
                query_vector[idx, 0] = 1
        # print(query_vector, np.sum(query_vector))
        
        similarity_scores = np.matmul(embeddings, embeddings.T)
        query_similarity_scores = np.matmul(similarity_scores, query_vector)
        # print(query_similarity_scores) ### |V| * 1, indicating similarity scores for each vocab term to query

        flattened_simlarity_matrix = query_similarity_scores.flatten()
        sorted_indices = np.argsort(flattened_simlarity_matrix)[::-1]

        ## Get the top indices
        top_indices = [(idx // query_similarity_scores.shape[1], idx % query_similarity_scores.shape[1]) for idx in sorted_indices[:TOP_K]]

        original_query_terms = query.lower().split()
        expanded_query_terms = []
        for top_index in top_indices:
            similar_term, _ = top_index
            expansions_file.write(f"{index_to_word[similar_term]} ")
            expanded_query_terms.append(index_to_word[similar_term])
        
        expanded_query_term_counter = Counter(expanded_query_terms)
        original_query_terms_counter = Counter(original_query_terms) 

        ### get LMs for all documents ranked for that query
        language_models = [DocumentLanguageModel(cord_uid=cord_uid, meta_data=meta_data, corpus_path=document_dir_path) for rank, cord_uid in query_results[int(topic_idx)].items()]
        ### get background LM for that query
        combined_lm = CombinedLanguageModel()
        for lm in language_models:
            combined_lm.add_document(lm)
        combined_lm.add_unk(percentage=0.5)
            
        ### add background LM to each individual model for Dirichilet smoothing
        for language_model in language_models:
            language_model.add_background_model(combined_lm)

        new_original_query_term_counter = dict()
        for k, v in original_query_terms_counter.items():
            key = k if k in combined_lm.word_counts.keys() else '<UNK>'
            if key not in new_original_query_term_counter:
                new_original_query_term_counter[key] = 0
            new_original_query_term_counter[key] += v
        original_query_terms_counter = new_original_query_term_counter

        new_expaneded_query_terms_counter = dict()
        for k, v in expanded_query_term_counter.items():
            key = k if k in combined_lm.word_counts.keys() else '<UNK>'
            if key not in new_expaneded_query_terms_counter:
                new_expaneded_query_terms_counter[key] = 0
            new_expaneded_query_terms_counter[key] += v
        expanded_query_term_counter = new_expaneded_query_terms_counter
            
        print(expanded_query_term_counter)
        ### compute relevant model probabilities (expanded query model)
        ### some query terms are not in global vocab
        relevance_model_probabilities = dict()
        for word in combined_lm.word_counts.keys():
            relevance_model_probabilities[word] = 0
            relevance_model_probabilities[word] += (W2V_LAMBDA)*(expanded_query_term_counter.get(word,0)/sum(expanded_query_term_counter.values()))
            relevance_model_probabilities[word] += (1-W2V_LAMBDA)*(original_query_terms_counter.get(word,0)/sum(original_query_terms_counter.values()))
        
        print(sum(relevance_model_probabilities.values())) ## doesn't sum to 1, because some query_terms are not in vocab

        results = []
        for i in range(len(language_models)):
            results.append((i+1, 1-kl_divergence_reverse(language_models[i], relevance_model_probabilities)))
        
        results.sort(key=lambda x: x[1], reverse=True)
        for idx, result in enumerate(results):
            old_rank, score = result
            output_file.write(f"{topic_idx} Q0 {query_results[topic_idx][old_rank]} {idx+1} {score:.4f} chinmay\n")
        
        
        expansions_file.write("\n")

expansions_file.close()
output_file.close()