import argparse
import numpy as np
from utils import parse_mq2008_agg_file, ouptut_formatter

parser = argparse.ArgumentParser(description='Process a collection Aggregate file for Bayes Fuse and write output in TREC Eval format.')
parser.add_argument('collection_file', type=str, help='Path to the input collection file (e.g., agg.txt)')
parser.add_argument('output_file', type=str, help='Path to the output file where results will be written')

# Parsing arguments
args = parser.parse_args()
collection_file_path = args.collection_file
output_file_path = args.output_file

print(f"Collection File: {collection_file_path}")
print(f"Output File: {output_file_path}")
rank_data = parse_mq2008_agg_file(collection_file_path)

mu = 0
sigma = 1

def gaussian(x):
    return 1 / (sigma * np.sqrt(2 * np.pi)) * np.exp(-((x - mu) ** 2) / (2 * sigma ** 2))

max_ranks = dict() ### for each search engine returns the max ranking till which it returns documents

### compute the number of candidates returned for each ranking engine
for query in rank_data.keys():
    document_data = rank_data[query]
    for document in document_data:
        rank_engines = document['ranks'].keys()
        for rank_engine in rank_engines:
            if document['ranks'][rank_engine] is not None:
                if rank_engine not in max_ranks:
                    max_ranks[rank_engine] = document['ranks'][rank_engine]
                else:
                    max_ranks[rank_engine] = max(max_ranks[rank_engine], document['ranks'][rank_engine])


relevant_probs = {
    rank_engine: [0.0001 for _ in range(max_ranks[rank_engine]+2)] for rank_engine in max_ranks.keys()
}

irrelevant_probs = {
    rank_engine: [0.0001 for _ in range(max_ranks[rank_engine]+2)] for rank_engine in max_ranks.keys()
}

relevant_prob_sum, irrelevant_prob_sum = 0, 0

for query in rank_data.keys():
    document_data = rank_data[query]
    for document in document_data:
        relevance_label = document['relevance_label']
        if relevance_label > 0:
            relevant_prob_sum += relevance_label
            for rank_engine in document['ranks'].keys():
                rank = document['ranks'][rank_engine]
                if rank is not None:
                    for i in range(max(1, rank-3), min(rank+3, max_ranks[rank_engine])+1):
                        relevant_probs[rank_engine][i] += relevance_label * gaussian(i-rank)
                else:
                    relevant_probs[rank_engine][-1] += relevance_label
        else:
            irrelevant_prob_sum += 1
            for rank_engine in document['ranks'].keys():
                rank = document['ranks'][rank_engine]
                if rank is not None:
                    for i in range(max(1, rank-5), min(rank+5, max_ranks[rank_engine])+1):
                        irrelevant_probs[rank_engine][i] += gaussian(i-rank)
                else:
                    irrelevant_probs[rank_engine][-1] += 1
                        
                
            

# print(relevant_prob_sum)
# print([float(f'{v:.2f}') for v in relevant_probs[1]])

# print(irrelevant_prob_sum)
# print([float(f'{v:.2f}') for v in irrelevant_probs[1]])

output_file = open(output_file_path, 'w')
for query in rank_data.keys():
    document_data = rank_data[query]
    document_scores = {
        doc['doc_id']: 0 for doc in document_data
    }
    rank_engines = document_data[0]['ranks'].keys()
    for document in document_data:
        doc_id = document['doc_id']
        for rank_engine in rank_engines:
            rank = document['ranks'][rank_engine]
            if rank is not None:
                document_scores[doc_id] += np.log(relevant_probs[rank_engine][rank]/relevant_prob_sum)
                document_scores[doc_id] -= np.log(irrelevant_probs[rank_engine][rank]/irrelevant_prob_sum)
    document_scores = [(doc, score) for doc, score in document_scores.items()]
    document_scores.sort(reverse=True, key=lambda x: x[1])
    for idx, (document, score) in enumerate(document_scores):
        output_str = ouptut_formatter(query_number=query, document_id=document, rank=idx+1, score=score, method='bayesfuse')
        output_file.write(f"{output_str}\n")
output_file.close()