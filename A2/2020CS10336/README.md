Build Script (*build.sh*)
This file will be used to build the source code for word2vec and TREC eval tool
```
bash build.sh
```

Running Task-1 (Pseudo Relevance Language Modelling)
```
bash lm_rerank.sh [query-file] [top-100-file] [collection-dir] [output-file]
```
Eg.
```
bash lm_rerank.sh ./col764-ass2-release/covid19-topics.xml ./col764-ass2-release/t40-top-100.txt /home/baadalvm/COL764/2020-07-16/ ./output-lm.txt
```


Running TREC Eval
```
./trec_eval-9.0.7/trec_eval -m ndcg -m ndcg_cut.5,10,50 <Query-Results-File> <Output-To-Be-Tested>
```
eg. 
```
./trec_eval-9.0.7/trec_eval -m ndcg -m ndcg_cut.5,10,50 ./col764-ass2-release/t40-qrels.txt ./output-lm.txt
```

Running Custom Eval (*score.py*)
```
python score.py --gold_query_relevances_path <path> --results_path <path>
```

eg.
```
python score.py --gold_query_relevances_path ./col764-ass2-release/t40-qrels.txt --results_path ./output-lm.txt
```

-----

Running Local Embedding based Query Expansion Reranking
```
bash w2v_rerank.sh [query-file] [top-100-file] [collection-dir] [output-file] [expansions-file]
```

Eg.
```
bash w2v_rerank.sh ./col764-ass2-release/covid19-topics.xml ./col764-ass2-release/t40-top-100.txt /home/baadalvm/COL764/2020-07-16/ ./output.w2v.txt ./expansions.txt 
```

Note: That during the execution of this task, Two folders are created:
1. vectors (This stores the embeddings learnt for each Query)
2. relevance-corpus (This stores the text files for each query on which word2vec is trained)
----

Other Files:

- *constants.py*: contains hyperparameters used for both the tasks tuned to their best values
- *utils.py*: basic utility funcitons, such as those for processing text, parsing PDFs, JSON etc.
- *lm.py*: Contains implementations of basic Uni-gram Language models with Dirichilet Smoothing
- *tokenizer.py*: Simple Tokenizer
- *lm_rerank.py*: Main Python script used by Task-1
- *w2v_rerank.py*: Main Python script used by Task-2
- *generate.py*: Used in Task-2 to generate training corpus for Word2Vec
- *./word2vec*: Source code from Google for Word2Vec Training
- *trec_eval-9.0.7*: Source code for the TREC eval tool
- *embeddings.sh*: Used to Train Word2Vec for each Query on the Relevance Corpus Generated



Running Individual Python Files

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

