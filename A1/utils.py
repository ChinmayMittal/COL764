import os
import re
import xml.etree.ElementTree as ET

def get_files(directory_path):
    files = [file for file in os.listdir(directory_path) if os.path.isfile(os.path.join(directory_path, file))]
    return files

def parse_file(file_path):
    with open(file_path, "r") as file:
        file_content = file.read()
        documents = file_content.split("<DOC>")[1:]
        documents = ["<DOC>" + document for document in documents]
        parsed_documents = []
        for idx, document in enumerate(documents):
            root = ET.fromstring(document)
            document = {}
            for element in root:
                document[element.tag] = re.sub('\n+', '\n', element.text.strip())
            parsed_documents.append(document)
        return parsed_documents

def simple_progress_bar(iterable):
    bar_length = 40
    for i, item in enumerate(iterable, 1):
        progress = i / len(iterable)
        arrow = '=' * int(progress * bar_length)
        spaces = ' ' * (bar_length - len(arrow))
        print(f"\r[{arrow}{spaces}] {progress*100:.2f}%", end='', flush=True)
        yield item


    