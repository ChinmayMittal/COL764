from typing import Dict
from constants import DELIMITERS, DIRICHILET_MU
from tokenizer import SimpleTokenizer
from utils import get_text_from_cord_id, preprocess_meta_data_file

class DocumentLanguageModel:
    
    def __init__(self, cord_uid: str, meta_data, corpus_path: str ):
        self.cord_uid = cord_uid
        self.tokenizer = SimpleTokenizer(DELIMITERS)
        text_list = get_text_from_cord_id(cord_uid, meta_data, corpus_path)
        self.document_length = 0
        self.word_counts: Dict[str, int] = dict()
        for text in text_list:
            text_tokens = self.tokenizer.tokenize(text)
            self.document_length += len(text_tokens)
            for token in text_tokens:
                if token in self.word_counts:
                    self.word_counts[token] += 1
                else:
                    self.word_counts[token] = 1
        self.background_lm = None
        self.mu = DIRICHILET_MU
    
    def add_background_model(self, background_lm):
        self.background_lm = background_lm
        
    def probability(self, word: str):
        term_frequency = self.word_counts.get(word,0)
        return (term_frequency + self.mu * self.background_lm.probability(word))/(self.document_length + self.mu)
                    
class CombinedLanguageModel:
    
    def __init__(self):
        self.total_count = 0
        self.word_counts: Dict[str, int] = dict()
        
    def add_document(self, document_model: DocumentLanguageModel):
        self.total_count += document_model.document_length
        for word, count in document_model.word_counts.items():
            if word in self.word_counts:
                self.word_counts[word] += count
            else:
                self.word_counts[word] = count
    
    def probability(self, word: str):
        term_frequency = self.word_counts.get(word,0)
        return (term_frequency/self.total_count)
        
if __name__ == '__main__':
    cord_uid1 = 'sdlhh79b'
    cord_uid2 = 'ug7v899j'
    meta_data_path = '/Users/chinmaymittal/Downloads/2020-07-16/metadata.csv'
    document_dir_path = '/Users/chinmaymittal/Downloads/2020-07-16/'
    meta_data = preprocess_meta_data_file(cord_uids=[cord_uid1, cord_uid2], meta_data_path=meta_data_path)
    lm1 = DocumentLanguageModel(cord_uid = cord_uid1, meta_data=meta_data, corpus_path=document_dir_path)
    print(lm1.word_counts)
    print(lm1.document_length)
    lm2 = DocumentLanguageModel(cord_uid = cord_uid2, meta_data=meta_data, corpus_path=document_dir_path)
    print(lm2.word_counts)
    print(lm2.document_length)
    combined_lm = CombinedLanguageModel()
    combined_lm.add_document(lm1)
    combined_lm.add_document(lm2)
    print(combined_lm.word_counts)
    print(combined_lm.total_count)
    

        