The following files are part of the submission

- `build.sh`: An Empty Shell File (No Build for Python)
- `utils.py`: A Utility File with useful methds for all models
- `constants.py`: A File containing Tuned Constants for all models (wherever applicable)
- `rrf.py`: The main Python File containing the implementation of Reciprocal Rank Fusion
- `borda.py`: The main Python File containing the implementation of Borda Counting based aggregation
- `condorcet.py`: The main Python File containing the implementation of Condorcet Voting based aggregation
- `bayes-fuse.py`: The main Python File containing the implementation of Bayes Fuse based aggregation
- `weighted-borda.py`: The main Python File containing the implementation of Weighted Borda Count based aggregation
- `rrf.sh`: The shell file to run Reciprocal Rank Fusion
- `bordacount.sh`: The shell file to run Borda count based aggregation
- `condorcet.sh`: The shell file to run Condorcet Voting based aggregation
- `bayesfuse.sh`: The shell file to run BayesFuse based aggregation
- `bordafuse.sh`: The shell file to run Weighted Borda Fuse based aggregation
- `README.md`: Markdown file describing the different files and commands to run the submission
- `2020CS10336.pdf`: A PDF file containing algorithmic and implementation details and Results.

How to Run

```
./build.sh
```

```
./rrf.sh [collection-file] [output-file]
```

```
./bordacount.sh [collection-file] [output-file]
```

```
./bordafuse.sh [collection-file] [output-file]
```

```
./bayesfuse.sh [collection-file] [output-file]
```

```
./condorcet.sh [collection-file] [output-file]
```

Eg.
```
./rrf.sh ~/Downloads/MQ2008-agg/agg.txt output.txt   
```

