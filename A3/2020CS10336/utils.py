def ouptut_formatter(query_number, document_id, rank, score, method):
    return f"{query_number} Q0 {document_id} {rank} {score} chinmay-{method}"

def parse_comment(comment):
    """
    Parses a line of text representing the comment in each line of the MQ2008-agg format

    Args:
    line (str): A string in the format "docid = [doc_id] inc = [inc] prob = [prob]"

    Returns:
    dict: A dictionary containing the parsed data.
    """
    parts = comment.split()

    # Extracting values from the string
    doc_id = parts[2]
    inc = float(parts[5])
    prob = float(parts[8])

    # Creating the dictionary
    parsed_dict = {
        'docid': doc_id,
        'inc': inc,
        'prob': prob
    }

    return parsed_dict

def parse_mq2008_agg_file(file_path):
    """
    Parses a file in the MQ2008-agg format.

    Args:
    file_path (str): Path to the file to parse.

    Returns:
    dict qid --> [list of dicts for each document with relevant info]
    """
    parsed_data = dict()

    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split('#')
            rank_list_info = parts[0].strip().split(' ')
            meta_data = parts[1]

            # Parsing the line
            query_id = int(rank_list_info[1].split(':')[1])
            data = {
                'relevance_label': int(rank_list_info[0]),
                'query_id': query_id
            }
            if query_id not in parsed_data:
                parsed_data[query_id] = list()

            # Parsing the ranks
            ranks = {}
            for part in rank_list_info[2:]:
                list_id, rank = part.split(':')
                ranks[int(list_id)] = None if rank == 'NULL' else int(rank)
            data['ranks'] = ranks

            # Parsing the comment
            parsed_comment = parse_comment(meta_data)
            data['doc_id'] = parsed_comment['docid']
            data['comment'] = parsed_comment
            
            parsed_data[query_id].append(data)

    return parsed_data

def generate_qrels(agg_file_path):
    parsed_data = parse_mq2008_agg_file(agg_file_path)
    qrels_file = open('qrels.txt', 'w')
    for query in parsed_data.keys():
        document_data = parsed_data[query]
        document_data = [(doc['doc_id'], doc['relevance_label']) for doc in document_data]
        document_data.sort(reverse=True, key=lambda x: x[1])
        for idx, (doc, score) in enumerate(document_data):
            qrels_file.write(f"{query} Q0 {doc} {score}\n")
    qrels_file.close()
if __name__ == '__main__':
    # Example usage
    agg_file_path = '/Users/chinmaymittal/Downloads/MQ2008-agg/agg.txt'
    parsed_data = parse_mq2008_agg_file(agg_file_path)
    print(len(parsed_data[10002]))
    print(parsed_data[10002][0])
    generate_qrels(agg_file_path)
    

