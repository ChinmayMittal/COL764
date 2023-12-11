Directory Structure and Files

- *build.sh*: This file is the build script for the project, it also downloads the external dependencies for this project like the lightweight XML parser library *pugixml*. This build script creates two binaries *inverted_index* and *search* which are latter used to create the inverted index and search using the queries provided
Usage (Ensure Internet Connectivity)
```
./build.sh
```
- *invidx.sh* This is the script to create the inverted index given the collection
Usage
```
./invidx.sh [coll-path] [indexfile] [0|1] [0|1]
```

- The description of the different command line arguments is as follows:

    - *coll-path* Path to the folder containing the entire collection
    - *indexfile* Name of the index file (indexfile.dict) and the dictionary file (indexfile.dict) created
    - *Compression Type* 0 specifies no-compression, 1 specifies Variable-Byte + Gap Encoding
     *Tokenizer* 0 specifies a simple delimiter based tokenizer, 1 specifies the BPE tokenizer

- *tf_idf_search.sh* This is the script used to search the collection given the queries and generate the relevant documents
Usage
```
./tf_idf_search.sh [queryfile] [resultsfile] [indexfile] [dictfile]
```

- *invidx_cons.cpp* Source code for creating the Inverted Index
- *search.cpp* Source code for searching
- *tokenizer.cpp*, *tokenizer.h* Source code for the Simple Tokenizer and the BPE Tokenizer
- *bpe_merges* Pre-Learnt Merges for the Byte Pair Encoding
- *utils.cpp*, *utils.h* Source code for Utilities

### How To Run ?

```
./build.sh
./invidx.sh ./train_data/combined  my-index 0 1
./tf_idf_search.sh ./train_data/test_query.txt ./results my-index.idx  my-index.dict
```

