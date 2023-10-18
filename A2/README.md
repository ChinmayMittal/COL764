
Running TREC Eval
```
./trec_eval-9.0.7/trec_eval -m ndcg -m ndcg_cut.5,10,50 ./col764-ass2-release/t40-qrels.txt ./output-lm.txt
```

Running Custom Eval

```
python score.py --gold_query_relevances_path ./col764-ass2-release/t40-qrels.txt --results_path ./output-w2v.txt
```

Running Language Model Based Reranking

```
python lm_rerank.py --query_file_path ./col764-ass2-release/covid19-topics.xml --original_rankings_path ./col764-ass2-release/t40-top-100.txt --corpus_path /Users/chinmaymittal/Downloads/2020-07-16 --output_path ./output-lm.txt
```

Running Word Embeddings Based Reranking

```
python w2v_rerank.py  --query_file_path ./col764-ass2-release/covid19-topics.xml --original_rankings_path ./col764-ass2-release/t40-top-100.txt --corpus_path /Users/chinmaymittal/Downloads/2020-07-16 --output_path ./output-w2v.txt --expansions_path ./expansions.txt --vectors_dir ./vectors
```

Generate Relevance Corpus for Each Query to Train Word2Vec

```
python generate.py --query_file_path ./col764-ass2-release/covid19-topics.xml --original_rankings_path ./col764-ass2-release/t40-top-100.txt --corpus_path /Users/chinmaymittal/Downloads/2020-07-16 --output_dir ./relevance-corpus
```

