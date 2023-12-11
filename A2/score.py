import argparse
from math import log2
from utils import parse_results_file


parser = argparse.ArgumentParser(description="Compute NDCG scores for different Queries")

parser.add_argument('--gold_query_relevances_path', required=True, type=str,
                        help="Path to the gold query relevances file in TREC format")
parser.add_argument('--results_path', required=True, type=str,
                        help="Path to query results files in TREC format")


args = parser.parse_args()
gold_query_relevances_path = args.gold_query_relevances_path
results_path = args.results_path

gold_results = {} ## query_number -> cord_id -> score

scores = {} ## query_number -> index -> n-DCG
with open(gold_query_relevances_path, 'r') as gold_file:
    for line in gold_file.readlines():
        if line.strip():
            query_number, run, cord_id, relevance = tuple(line.split())
            query_number = int(query_number)
            relevance = int(relevance)
            if query_number not in gold_results:
                gold_results[query_number] = dict()
            gold_results[query_number][cord_id] = relevance
        
output_results = parse_results_file(results_file_path=results_path) ## query_number -> rank -> cord_id

query_numbers = list(map(int, output_results.keys()))

average_score = 0

for query_number in query_numbers:
    gold_scores = [(score, cord_id) for cord_id, score in gold_results[query_number].items()]
    sorted_gold_scores = sorted(gold_scores, key=lambda x: x[0], reverse=True)
    output_scores = {
            rank: gold_results[query_number][cord_id] if cord_id in gold_results[query_number] else 0 
            for rank, cord_id in output_results[query_number].items()
        }
    max_rank = max(output_scores.keys())
    output_scores = [output_scores[rank] for rank in range(1,max_rank+1)]
    discounted_output_scores = [score/log2(idx+2) for idx, score in enumerate(output_scores)]
    discounted_cumulative_output_scores, discounted_cumulative_score = [], 0
    for score in discounted_output_scores:
        discounted_cumulative_score += score
        discounted_cumulative_output_scores.append(discounted_cumulative_score)
    
    gold_scores = [sorted_gold_scores[rank][0] if rank < len(sorted_gold_scores) else 0 for rank in range(0, max_rank)]
    discounted_gold_scores = [score/log2(idx+2) for idx, score in enumerate(gold_scores)]
    discounted_cumulative_gold_scores, discounted_cumulative_score = [], 0
    for score in discounted_gold_scores:
        discounted_cumulative_score += score
        discounted_cumulative_gold_scores.append(discounted_cumulative_score)

    def normalized_score_at_rank(rank):
        return discounted_cumulative_output_scores[rank-1]/discounted_cumulative_gold_scores[rank-1]
    
    average_score += normalized_score_at_rank(5) + normalized_score_at_rank(10) + normalized_score_at_rank(50)
    print(f"Query: {query_number}\n\tnDCG@5: {normalized_score_at_rank(5)}\n\tnDCG@10: {normalized_score_at_rank(10)}\n\tnDCG@50: {normalized_score_at_rank(50)}")

print(f"Average Score: {average_score/(3*len(query_numbers))}")
    