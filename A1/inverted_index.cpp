#include <iostream>
#include <string>
#include <filesystem>
#include <set>
#include <map>
#include <vector>
#include <fstream>

#include "document.h"
#include "pugixml.hpp"

namespace fs = std::__fs::filesystem;

std::vector<Document> parse_file(std::string file)
{
    std::vector<Document> documents;
    Document current_document;
    bool inside_doc = false;
    bool inside_title = false;
    bool inside_content = false;

    std::ifstream input_file(file);
    if (!input_file) {
        std::cerr << "Error opening the file." << std::endl;
        return documents;
    }

    std::string line;
    while (std::getline(input_file, line)) {
        if (line.find("<DOC>") != std::string::npos) {
            inside_doc = true;
            current_document = {};
        } else if (line.find("</DOC>") != std::string::npos) {
            inside_doc = false;
            documents.push_back(current_document);
        } else if (inside_doc) {
            if (line.find("<DOCID>") != std::string::npos) {
                current_document.docID = line.substr(7, line.size() - 15);
            } else if (line.find("<TITLE>") != std::string::npos) {
                inside_title = true;
                if (line.find("</TITLE>") != std::string::npos){
                    inside_title = false;
                    current_document.title += line.substr(7, line.size() -15); 
                }else{
                    current_document.title += line.substr(7);
                }
            } else if (inside_title) {
                if (line.find("</TITLE>") != std::string::npos) {
                    inside_title = false;
                } else {
                    current_document.title += line;
                }
            } else if (line.find("<CONTENT>") != std::string::npos) {
                inside_content = true;
                if (line.find("</CONTENT>") != std::string::npos){
                    inside_content = false;
                    current_document.title += line.substr(9, line.size() -19); 
                }else{
                    current_document.title += line.substr(9);
                }
            } else if (inside_content) {
                if (line.find("</CONTENT>") != std::string::npos) {
                    inside_content = false;
                } else {
                    current_document.content += line;
                }
            }
        }
    }

    input_file.close();
    return documents;
}

int main(int argc, char *argv[]) {
    if(argc <=1)
    {
        std::cout << "Requires Training Directory as Script Argument" << std::endl;
        return 0;
    }
    std::string directory_path = argv[1];
    std::string vocabulary;

    std::cout << "Training Directory: " << directory_path << std::endl;

    for (const auto &file : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(file)) {
            std::cout << file.path() << std::endl;
            std::vector<Document> documents = parse_file(file.path());
            for (auto const& doc : documents) {
                std:: cout << (doc.docID) << "\n";
                std::cout << doc.title << "\n";
                // std::cout << doc.descipt
            }
        }
    }
    pugi::xml_document doc;
    if (!doc.load_file("./train_data/temp/temp")) {
        std::cerr << "Error loading XML file." << std::endl;
        return 1;
    }

    std::vector<Document> documents;

    for (pugi::xpath_node docNode : doc.select_nodes("//DOC")) {
        Document currentDocument;
        pugi::xml_node docXml = docNode.node(); // Get the corresponding xml_node

        for (pugi::xml_node node : docXml.children()) {
            if (std::string(node.name()) == "DOCID") {
                currentDocument.docID = node.text().get();
            } else if (std::string(node.name()) == "TITLE") {
                currentDocument.title = node.text().get();
            } else if (std::string(node.name()) == "CONTENT") {
                currentDocument.content = node.text().get();
            }
        }
        documents.push_back(currentDocument);
    }

    for (const auto& doc : documents) {
        std::cout << "DOCID: " << doc.docID << std::endl;
        std::cout << "TITLE: " << doc.title << std::endl;
        std::cout << "CONTENT: " << doc.content << std::endl;
        std::cout << "------------------------" << std::endl;
    }

    return 0;
}