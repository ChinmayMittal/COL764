import argparse
from utils import parse_mq2008_agg_file, ouptut_formatter

parser = argparse.ArgumentParser(description='Process a collection Aggregate file for BordaCount Fusion and write output in TREC Eval format.')
parser.add_argument('collection_file', type=str, help='Path to the input collection file (e.g., agg.txt)')
parser.add_argument('output_file', type=str, help='Path to the output file where results will be written')

# Parsing arguments
args = parser.parse_args()
collection_file_path = args.collection_file
output_file_path = args.output_file

print(f"Collection File: {collection_file_path}")
print(f"Output File: {output_file_path}")
rank_data = parse_mq2008_agg_file(collection_file_path)

max_ranks = dict() ### for each search engine returns the max ranking

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

### Uniform Weighting
# weights = {
#     rank_engine: 1/len(max_ranks) for rank_engine in max_ranks.keys()
# }

### Statistics Based Weighting
weights = {
    rank_engine: 0 for rank_engine in max_ranks.keys()
}

for query in rank_data.keys():
    document_data = rank_data[query]
    for document in document_data:
        relevance_label = document['relevance_label']
        for rank_engine in document['ranks'].keys():
            if document['ranks'][rank_engine] is not None:
                # weights[rank_engine] += relevance_label / (60 + document['ranks'][rank_engine])
                weights[rank_engine] += relevance_label / (60 + document['ranks'][rank_engine])
                

normalization_factor = sum([v for k, v in weights.items()])
weights = {
    rank_engine: weight/normalization_factor for rank_engine, weight in weights.items()
}
print(weights)

output_file = open(output_file_path, 'w')
for query in rank_data.keys():
    document_data = rank_data[query]
    document_scores = {
        doc['doc_id']: 0 for doc in document_data
    }
    for document in document_data:
        doc_id = document['doc_id']
        for rank_engine in document['ranks'].keys():
            if document['ranks'][rank_engine] is not None:
                document_scores[doc_id] += weights[rank_engine] *  (max_ranks[rank_engine] - document['ranks'][rank_engine])
    
    document_scores = [(doc, score) for doc, score in document_scores.items()]
    document_scores.sort(reverse=True, key=lambda x: x[1])
    for idx, (document, score) in enumerate(document_scores):
        output_str = ouptut_formatter(query_number=query, document_id=document, rank=idx+1, score=score, method='bordacount')
        output_file.write(f"{output_str}\n")

output_file.close()
print(max_ranks)