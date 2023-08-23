import re
from typing import List
from constants import DELIMITERS
from collections import defaultdict
from nltk.tokenize import RegexpTokenizer


class SimpleTokenizer:
    
    def __init__(self, delimiters):
        self.delimiters = delimiters
        pattern = "[" + re.escape(''.join(self.delimiters)) + "]+"
        self._tokenizer_ = RegexpTokenizer(pattern=pattern, gaps=True)


    def train(self):
        pass

    
    def tokenize(self, text: str)->List[str]:
        tokens = self._tokenizer_.tokenize(text)
        return tokens

class BPETokenizer:
    def __init__(self, delimiters, max_iter=10000):
        self.max_iter = max_iter
        self.delimiters = delimiters
        self.word_frequencies = defaultdict(int)
        self.alphabet = []
        self.merges = []
        
    def compute_pair_frequencies(self):
        pair_frequencies = defaultdict(int)
        for word, frequency in self.word_frequencies.items():
            current_word_split = self.splits[word]
            if len(current_word_split) != 1:
                for idx in range(0, len(current_word_split)-1):
                    vocab_pair = (current_word_split[idx], current_word_split[idx+1])
                    pair_frequencies[vocab_pair] += frequency
        return pair_frequencies
    
    def merge_pair(self, first, second):
        for word in self.word_frequencies.keys():
            split = self.splits[word]
            if len(split) != 1:
                idx = 0
                while (idx < len(split)-1):
                    if split[idx] == first and split[idx+1] == second:
                        split = split[:idx] + [(first+second)] + split[idx+2:]
                    else:
                        idx += 1
            self.splits[word] = split
                    
        

    def train(self, corpus: List[str]):
        ### compute word frequencies
        self.pre_tokenizer = SimpleTokenizer(self.delimiters)
        for text in corpus:
            pre_tokenized_text = self.pre_tokenizer.tokenize(text)
            for word in pre_tokenized_text:
                self.word_frequencies[word] += 1
        
        ### compute initial alphabet
        for word in self.word_frequencies.keys():
            for letter in word:
                if letter not in self.alphabet:
                    self.alphabet.append(letter)
        self.alphabet.sort()
        self.vocab = ["<E>"] + self.alphabet.copy()
        
        ## split each word
        self.splits = {word: [char for char in word] for word in self.word_frequencies.keys()}
        
        iter = 0
        while(iter < self.max_iter):
            pair_frequencies = self.compute_pair_frequencies()
            most_frequent_vocab_pair = max(pair_frequencies.keys(), key=pair_frequencies.get)
            self.merges.append(most_frequent_vocab_pair)
            self.vocab.append(most_frequent_vocab_pair[0] + most_frequent_vocab_pair[1])
            self.merge_pair(*most_frequent_vocab_pair)
            iter +=1
    
    def tokenize(self, text):
        pre_tokenized_text = self.pre_tokenizer.tokenize(text)
        splits = [[char for char in word] for word in pre_tokenized_text]
        for merge in self.merges:
            for word_idx, split in enumerate(splits):
                idx = 0
                while(idx < len(split)-1):
                    if(split[idx] == merge[0] and split[idx+1] == merge[1]):
                        split = split[:idx] + [(merge[0]+merge[1])] + split[idx+2:]
                    else:
                        idx += 1
                splits[word_idx] = split
        return sum(splits, start=[])
        
        
            
    


if __name__ == "__main__":
    text = """This, is the 'text' I want to tokenize; My name is:chinmay. """
    corpus = [
        "This is part of the information retreival course",
        "Tokenization algorithm using byte pair encoding",
        "My name is chinmay",
        "This is a randomly generated sentence.",
    ]
    tokenizer = BPETokenizer(DELIMITERS, max_iter=20)
    tokenizer.train(corpus=corpus)
    tokens = tokenizer.tokenize(text)
    print(tokens)
        
        
        
        
        