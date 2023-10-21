import re
from typing import List
from constants import DELIMITERS
from nltk.tokenize import RegexpTokenizer


class SimpleTokenizer:

    def __init__(self, delimiters):
        self.delimiters = delimiters
        pattern = "[" + re.escape(''.join(self.delimiters)) + "]+"
        self._tokenizer_ = RegexpTokenizer(pattern=pattern, gaps=True)

    def tokenize(self, text: str)->List[str]:
        tokens = self._tokenizer_.tokenize(text.lower())
        return tokens
    
if __name__ == "__main__":
    text = """This, is the 'text' I want to tokenize; My name is:chinmay. """
    tokenizer = SimpleTokenizer(DELIMITERS)
    tokens = tokenizer.tokenize(text)
    print(tokens)