import numpy as np
import xml.etree.ElementTree as ET
import nltk
nltk.download('stopwords')
from nltk.corpus import stopwords

tree = ET.parse('./col764-ass2-release/covid19-topics.xml')
root = tree.getroot()

stop_words = set(stopwords.words('english'))
topics = dict()
for topic in root.findall('topic'):
    topic_number = topic.get('number')
    query = topic.find('query').text
    question = topic.find('question').text
    narrative = topic.find('narrative').text
    
    # Print or process the extracted information as needed
    topics[topic_number] = {
        "query": query,
        "question": question,
        "narrative": narrative
    }

def parse_embedding_file(file_path):
    word_to_index = {}
    embeddings = []
    index_to_word = {}
    
    with open(file_path, 'r') as file:
        num_words, num_dimensions = map(int, file.readline().split())
        for i, line in enumerate(file):
            parts = line.split()
            word = parts[0]
            embedding = np.array(list(map(float, parts[1:])))
            word_to_index[word] = i
            index_to_word[i] = word
            embeddings.append(embedding)
    

    embeddings_array = np.array(embeddings)    
    return embeddings_array, word_to_index, index_to_word


for topic_idx, topic in topics.items():
        embeddings, word_to_index, index_to_word = parse_embedding_file(f"./vectors/vectors-{topic_idx}.text")
        query = ' '.join([word for word in (topic['query']).split() if word not in stop_words])
        print(query)

        query_matrix = []
        query_matrix_index = dict()
        counter = 0
        for term in query.split(" "):
            if term in word_to_index:
                query_matrix.append(embeddings[word_to_index[term]])
                query_matrix_index[counter] = term
                counter += 1
        query_matrix = np.vstack(query_matrix)

        similarity_matrix = np.matmul(query_matrix, embeddings.T)
        print(similarity_matrix.shape)

        flattened_simlarity_matrix = similarity_matrix.flatten()
        sorted_indices = np.argsort(flattened_simlarity_matrix)[::-1]

        # Get the top 20 indices as tuples (row, column)
        top_indices = [(idx // similarity_matrix.shape[1], idx % similarity_matrix.shape[1]) for idx in sorted_indices[:10]]

        for top_index in top_indices:
            query_term, corpus_term = top_index
            print(query.split()[query_term], index_to_word[corpus_term])
        
        print("-"*30)
