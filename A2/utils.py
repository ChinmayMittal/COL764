import os
import csv
import json
from math import log10
from typing import List

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