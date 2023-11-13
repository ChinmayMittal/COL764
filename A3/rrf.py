import argparse
from constants import RRF_K
from utils import parse_mq2008_agg_file, ouptut_formatter

parser = argparse.ArgumentParser(description='Process a collection Aggregate file for Recirpocal Rank Fusion (RRF) and write output in TREC Eval format.')
parser.add_argument('collection_file', type=str, help='Path to the input collection file (e.g., agg.txt)')
parser.add_argument('output_file', type=str, help='Path to the output file where results will be written')

# Parsing arguments
args = parser.parse_args()
collection_file_path = args.collection_file
output_file_path = args.output_file

print(f"Collection File: {collection_file_path}")
print(f"Output File: {output_file_path}")
rank_data = parse_mq2008_agg_file(collection_file_path)

def RRF_score(score):
    if score is None:
        return 0
    return (1/(RRF_K + score))

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
            document_scores[doc_id] += RRF_score(document['ranks'][rank_engine])
    document_scores = [(doc, score) for doc, score in document_scores.items()]
    document_scores.sort(reverse=True, key=lambda x: x[1])
    for rank, (document, score) in enumerate(document_scores):
        output_str = ouptut_formatter(query_number=query, document_id=document, rank=rank+1, score=score, method='rrf')
        output_file.write(f"{output_str}\n")
output_file.close()
