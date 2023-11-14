import argparse
from utils import parse_mq2008_agg_file, ouptut_formatter

parser = argparse.ArgumentParser(description='Process a collection Aggregate file for Condorcet Voting and write output in TREC Eval format.')
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
### this will be used to resolve ties with board counting
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
                    
                    
output_file = open(output_file_path, 'w')
for query in rank_data.keys():
    document_data = rank_data[query]
    document_ids = [doc['doc_id'] for doc in document_data]
    document_scores = {
        doc_id: 0 for doc_id in document_ids
    }
    
    borda_scores = {
        doc_id: 0 for doc_id in document_ids
    }
    
    for i in range(len(document_ids)):
        for j in range(i+1, len(document_ids)):
            doc_i, doc_j = document_data[i],  document_data[j]
            rank_engines_i = set(doc_i['ranks'].keys())
            rank_engines_j = set(doc_j['ranks'].keys())
            rank_engines = rank_engines_i | rank_engines_j
            score = 0
            for rank_engine in rank_engines:
                doc_score_i = doc_i['ranks'][rank_engine] if rank_engine in doc_i['ranks'] else None
                doc_score_j = doc_j['ranks'][rank_engine] if rank_engine in doc_j['ranks'] else None
                if doc_score_i is not None:
                    if doc_score_j is None:
                        score += 1
                    else:
                        if doc_score_i < doc_score_j:
                            score += 1
                        else:
                            score -= 1
                else:
                    if doc_score_j is not None:
                        score -= 1
            
            if score > 0:
                document_scores[doc_i['doc_id']] += 1
            elif score < 0:
                document_scores[doc_j['doc_id']] += 1

        ## Borda Scores for Ties
        doc_id = document_ids[i]
        for rank_engine in document_data[i]['ranks'].keys():
            if  document_data[i]['ranks'][rank_engine] is not None:
                borda_scores[doc_id] += max_ranks[rank_engine] - document_data[i]['ranks'][rank_engine]

              
        
    document_scores = [(doc, score, borda_scores[doc]) for doc, score in document_scores.items()]
    document_scores.sort(reverse=True, key=lambda x: (x[1], x[2]))
    for idx, (document, score, borda_score) in enumerate(document_scores):
        output_str = ouptut_formatter(query_number=query, document_id=document, rank=idx+1, score=float(f"{score}.{borda_score}"), method='condorcet')
        output_file.write(f"{output_str}\n")
            
            
            
    
output_file.close()
