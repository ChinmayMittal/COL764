import os
import re
import string
import csv
import json
from math import log10
from typing import List
from constants import DELIMITERS
from tokenizer import SimpleTokenizer

import nltk
nltk.download('punkt')
from nltk.stem import SnowballStemmer



def simple_progress_bar(iterable, total=None, bar_length=30):
    if total is None:
        try:
            total = len(iterable)
        except TypeError:
            raise TypeError("If 'total' is not provided, 'iterable' must have a length.")

    for i, item in enumerate(iterable):
        progress = (i / total) * 100
        num_blocks = int(round(bar_length * progress / 100))
        bar = "#" * num_blocks + "-" * (bar_length - num_blocks)
        status = f"[{bar}] {progress:.1f}% ({i}/{total})"
        print(status, end="\r")
        yield item
    print()  # Print a newline to complete the progress bar display

def parse_results_file(results_file_path):
    
    results = dict() ## query_number -> rank -> cord_id
    with open(results_file_path, 'r') as results_file:
        for line in results_file.readlines():
            if line.strip():
                query_number, ignore_col, cord_id, rank, score, run_id = tuple(line.split())
                query_number = int(query_number)
                rank = int(rank)
                if query_number not in results:
                    results[query_number] = dict()
                results[query_number][rank] = cord_id
    return results

def preprocess_text( text: str,
                     lowercase: bool,
                     punctuations: bool,
                     digits: bool,
                     stemming: bool,
                     stopwords_elimination: bool) -> str:
    
    if lowercase:
        text = text.lower()
   
    if punctuations:
        translator = str.maketrans('', '', string.punctuation)
        text = text.translate(translator)
        tokenizer = SimpleTokenizer(DELIMITERS)
        text = " ".join(tokenizer.tokenize(text))
    
    if stemming:
        stemmer = SnowballStemmer("english")
        words = nltk.word_tokenize(text)
        stemmed_words = [stemmer.stem(word) for word in words]
        text = ' '.join(stemmed_words)
        
    if digits:
        digits_pattern = r"\d+(\.\d+)?"
        text = re.sub(digits_pattern, "<NUMBER>", text)
    
    if stopwords_elimination:
        stop_words_set = {'what', 'i', 'didn', 'doing', 'themselves', 'd', 'ourselves', "you'll", "you've", "should've", 'its', 'before', 'while', 'off', 'can', 'once', 'only', 'being', 'have', 'but', 'nor', 'above', 'any', 'by', 'itself', 'shan', 'during', 'below', 'needn', 'with', 'shouldn', 'be', "it's", 're', 'she', "doesn't", 'the', 'wasn', 'a', 'you', "needn't", 'yourself', 'an', "aren't", "weren't", 'did', 'himself', 'herself', "don't", 'yourselves', 'aren', 'hadn', 'than', 'not', "that'll", 'couldn', 'so', 'do', 'having', 'mustn', "wouldn't", "haven't", "isn't", 'these', 'has', 'and', 'until', 'yours', 'y', 'theirs', 'under', 'does', 's', 'it', 'same', 'their', 'ours', 'after', 'don', 'doesn', 'at', 'ain', 'is', 'for', 'wouldn', "won't", 'he', 'or', 'over', 'all', 'are', 'to', 'myself', 'against', 'own', 'her', 'very', "couldn't", 'just', 'won', 'because', 'had', 'm', 'some', "mustn't", 'ma', 'your', 'each', 'out', 'such', 'again', 'more', 'am', "hasn't", 'isn', 'into', 'down', 'my', 'further', 'was', 'weren', 'whom', 'haven', 'through', 'when', 'here', "you're", 'why', 'this', 'should', 'hers', 'were', 've', 'then', "didn't", 'we', "you'd", "wasn't", 'his', 'o', 'few', 'will', "hadn't", 'no', 'if', "she's", 'of', 'there', 'which', 'now', 'mightn', 'me', 'been', 'from', 'how', "shan't", 'in', 'them', "mightn't", 'our', "shouldn't", 'between', 'where', 'as', 'too', 'up', 'both', 'him', 'who', 'that', 'll', 'those', 'they', 'on', 'other', 'most', 'hasn', 't', 'about'}
        text = ' '.join(word for word in text.split() if word not in stop_words_set)
        
    
        
    return text    
    

def get_text_from_cord_id(cord_uid: str, meta_data, corpus_dir_path: str) -> List[str]:
    try:
            text_list = list()
            for entry in meta_data[cord_uid]:
                pmc_json_files = entry['pdfs']
                pdf_json_files = entry['pmcs']
                abstract, title = entry['abstract'], entry['title']
                # print(pdf_json_files, pmc_json_files, sep='\n')
                if len(pmc_json_files) != 0:
                    for pmc_json_file in pmc_json_files:
                        pmc_json_file_path = os.path.join(corpus_dir_path, pmc_json_file)
                        with open(pmc_json_file_path, 'r') as pmc_file:
                            pmc_data = json.load(pmc_file)
                            for text_group in pmc_data['body_text']:
                                text = text_group['text']
                                text_list.append(text)
                elif len(pdf_json_files) != 0:
                    for pdf_json_file in pdf_json_files:
                        pdf_json_file_path = os.path.join(corpus_dir_path, pdf_json_file)
                        with open(pdf_json_file_path, 'r') as pdf_file:
                            pdf_data = json.load(pdf_file)
                            for text_group in pdf_data['body_text']:
                                text = text_group['text']
                                text_list.append(text)
                else:
                            text_list.append(abstract)
                            text_list.append(title)
            return text_list
    except Exception as e:
        print(f"Error While Processing {cord_uid}")
        return []

def preprocess_meta_data_file(cord_uids: List[str], meta_data_path: str):
    ### precompute pdf, pmc paths and abstract and titles for all these cord_uids
    meta_data_dict = dict() ### cord_uid --> list[dict: (pmc_json_files, pdf_json_files, abstract, title)]
    with open(meta_data_path, 'r') as meta_data_file:
        reader = csv.DictReader(meta_data_file)
        for row in reader:
            if row['cord_uid'] in cord_uids:
                cord_uid = row['cord_uid']
                pmc_json_files = [name.strip() for name in row['pmc_json_files'].split(';') if "json" in name]
                pdf_json_files = [name.strip() for name in row['pdf_json_files'].split(';') if "json" in name]
                abstract, title = row['abstract'], row['title']
                if cord_uid not in meta_data_dict:
                    meta_data_dict[cord_uid] = []
                meta_data_dict[cord_uid].append({
                    'abstract': abstract,
                    'title': title,
                    'pdfs': pdf_json_files,
                    'pmcs': pmc_json_files
                })
                
    return meta_data_dict

def kl_divergence(document_model, relevance_model_probabilities):
    accumulator = 0
    for word in relevance_model_probabilities.keys():
        accumulator += document_model.probability(word) * log10(document_model.probability(word)/relevance_model_probabilities[word])
    return accumulator
        
def kl_divergence_reverse(document_model, query_model):
    accumulator = 0
    for word in query_model.keys():
        if query_model[word] > 0:
            accumulator += query_model[word] * log10(query_model[word]/document_model.probability(word))
    return accumulator

if  __name__ == '__main__':
    cord_id = 'ug7v899j'
    # cord_id = '8l411r1w'
    meta_data_path = '/Users/chinmaymittal/Downloads/2020-07-16/metadata.csv'
    document_dir_path = '/Users/chinmaymittal/Downloads/2020-07-16/'
    meta_data = preprocess_meta_data_file(cord_uids=[cord_id], meta_data_path=meta_data_path)
    print(meta_data[cord_id])
    print(get_text_from_cord_id(cord_id, meta_data, document_dir_path))