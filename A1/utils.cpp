#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <set>

#include "pugixml.hpp"
#include "document.h"

void print_progress(int steps_completed, int total_steps)
{
    float progress = static_cast<float>(steps_completed)/total_steps;
    int barWidth = 40;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int j = 0; j < barWidth; ++j) {
        if (j < pos) std::cout << "=";
        else if (j == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "%\r";
    std::cout.flush();
}

std::vector<Document> parse_file(std::string file)
{
    std::vector<Document> documents;

    std::ifstream input_file(file);
    if (!input_file) {
        std::cerr << "Error opening the file." << std::endl;
        return documents;
    }

    pugi::xml_document doc;
    if (!doc.load_file(file.c_str())) {
        std::cerr << "Error loading XML file." << std::endl;
        return documents;
    }

    for (pugi::xpath_node doc_node : doc.select_nodes("//DOC")) {
        Document current_document;
        pugi::xml_node docXml = doc_node.node(); // Get the corresponding xml_node

        for (pugi::xml_node node : docXml.children()) {
            if (std::string(node.name()) == "DOCID") {
                current_document.docID = node.text().get();
            } else if (std::string(node.name()) == "TITLE") {
                current_document.title = node.text().get();
            } else if (std::string(node.name()) == "CONTENT") {
                current_document.content = node.text().get();
            }
        }
        documents.push_back(current_document);
    }


    input_file.close();
    return documents;
}

float inverse_document_frequency(float document_frequency, float total_documents)
{
    return log2(1 + total_documents/document_frequency);
}

float term_frequency(float term_count)
{
    return (term_count ? 1 + log2(term_count) : 0) ;
}

std::set<char> get_base_delimiters()
{
    return std::set<char>{'.' , ' ' , ':', ';', '\"', '\'', '.', '?', '!', ',', '\n'};
}
