#include <iostream>
#include <string>
#include <filesystem>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>

#include "document.h"
#include "pugixml.hpp"
#include "tokenizer.h"

namespace fs = std::__fs::filesystem;

void print_progrss(int steps_completed, int total_steps)
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

void posting_list_to_disk(std::map<std::string, std::vector<std::pair<int, int>>> &postings_list, std::string file_path)
{
        std::ofstream temporary_file(file_path, std::ofstream::out | std::ofstream::trunc);
        if (!temporary_file) {
            std::cerr << "Error opening the file." << std::endl;
            return ;
        }
        for(auto &pr : postings_list)
        {
            temporary_file << pr.first << " ";
            temporary_file << pr.second.size() << " ";
            for(auto &x : pr.second)
            {
                temporary_file  << x.first << " " << x.second << " " ; // doc-id, cnt
            }
            temporary_file << "\n" ;
        }
        temporary_file.close();
}

void merge_helper(std::string &word, std::ofstream &temporary_file, std::istringstream &iss, int &num, std::string &line, std::ifstream &file, bool &read)
{
    temporary_file << word << " ";
    while(iss >> num)
    {
        temporary_file << num << " ";
    }
    temporary_file << "\n";

    read = (bool)std::getline(file, line);
    if(read)
    {
        iss = std::istringstream(line);
        iss >> word;
    }
    return;
}

void merge_postings_list_from_disk(std::string file_path_1, std::string file_path_2, std::string output_file)
{
    std::ifstream file1(file_path_1), file2(file_path_2);
    if (!file1 || !file2) {
        std::cerr << "Error opening files." << std::endl;
        return;
    }

    std::ofstream temporary_file(output_file, std::ofstream::out | std::ofstream::trunc);
    if (!temporary_file) {
        std::cerr << "Error opening the file." << std::endl;
        return;
    }

    std::string line1, line2;

    bool read1 = (bool)std::getline(file1, line1);
    bool read2 = (bool)std::getline(file2, line2);

    std::istringstream iss1(line1), iss2(line2);
    std::string word1, word2;
    int num1, num2;
    iss1 >> word1 ;
    iss2 >> word2 ;

    while(read1 && read2)
    {
        std::cout << word1 << " " << word2 << std::endl; 
        if(word1 < word2)
        {
            temporary_file << word1 << " ";
            while(iss1 >> num1)
            {
                temporary_file << num1 << " ";
            }
            temporary_file << "\n";

            read1 = (bool)std::getline(file1, line1);
            if(read1)
            {
                iss1 = std::istringstream(line1);
                iss1 >> word1;
            }
        }else if(word2 < word1)
        {
            temporary_file << word2 << " ";
            while(iss2 >> num2)
            {
                temporary_file << num2 << " ";
            }
            temporary_file << "\n";

            read2 = (bool)std::getline(file2, line2);
            if(read2)
            {
                iss2 = std::istringstream(line2);
                iss2 >> word2;
            }
        }else{
            temporary_file << word1 << " ";
            iss1 >> num1 ; iss2 >> num2 ;
            temporary_file << num1 + num2 << " " ; // frequency sum
            // file1 has smaller indices, write it first
            while(iss1 >> num1)
            {
                temporary_file << num1 << " ";
            }
            while(iss2 >> num2)
            {
                temporary_file << num2 << " ";
            }
            temporary_file << "\n";
            // read next lines 
            read1 = (bool)std::getline(file1, line1);
            if(read1)
            {
                iss1 = std::istringstream(line1);
                iss1 >> word1;
            }
            read2 = (bool)std::getline(file2, line2);
            if(read2)
            {
                iss2 = std::istringstream(line2);
                iss2 >> word2;
            }
        }
    }

    while(read1)
    {
        temporary_file << word1 << " ";
        while(iss1 >> num1)
        {
            temporary_file << num1 << " ";
        }
        temporary_file << "\n";

        read1 = (bool)std::getline(file1, line1);
        if(read1)
        {
            iss1 = std::istringstream(line1);
            iss1 >> word1;
        }
    }

    while(read2)
    {
        temporary_file << word2 << " ";
        while(iss2 >> num2)
        {
            temporary_file << num2 << " ";
        }
        temporary_file << "\n";

        read2 = (bool)std::getline(file2, line2);
        if(read2)
        {
            iss2 = std::istringstream(line2);
            iss2 >> word2;
        }
    }
    file1.close();
    file2.close();
    temporary_file.close();
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

int main(int argc, char *argv[]) {
    // if(argc <=1)
    // {
    //     std::cout << "Requires Training Directory as Script Argument" << std::endl;
    //     return 0;
    // }
    // std::string directory_path = argv[1];
    // std::string temporary_directory_path = "./temp";
    // std::string vocabulary;
    // // Start the clock
    // auto start = std::chrono::high_resolution_clock::now();

    // // directory for temporary file storage during index construction
    // if (fs::create_directory(temporary_directory_path)) {
    //     std::cout << "Directory created successfully." << std::endl;
    // } else {
    //     std::cout << "Failed to create directory." << std::endl;
    // }

    // std::cout << "Training Directory: " << directory_path << std::endl;
    // SimpleTokenizer tokenizer(std::set<char>{'.', ' ', ':', ';', '\"', '\'', '.', '?', '!', ',', '\n'});
    // std::map<std::string, int> docId_to_idx;
    // std::map<std::string, std::vector<std::pair<int, int>>> postings_list; // term --> list[doc_idx, term_frequency]
    // int document_cnt = 0;

    // int file_cnt = 0; // count the total number of files to be processed
    // for (const auto& entry : fs::directory_iterator(directory_path)) {
    //     if (fs::is_regular_file(entry)) {
    //         ++file_cnt;
    //     }
    // }

    // int files_processed = 0;
    // int temporary_file_count = 0;
    // for (const auto &file : fs::directory_iterator(directory_path)) {
    //     print_progrss(files_processed++, file_cnt);
    //     if (fs::is_regular_file(file)) {
    //         // process file
    //         std::vector<Document> documents = parse_file(file.path());
    //         for (auto const& doc : documents) {
    //             std::string document_content = doc.title + " " + doc.content;
    //             for(char &ch : document_content)
    //                 ch = tolower(ch);
    //             std::vector<std::string> tokens = tokenizer.tokenize(document_content);
    //             std::map<std::string, int> token_counts;
    //             for(auto const &token : tokens) token_counts[token] += 1;
    //             for(auto const &pr : token_counts)
    //             {
    //                 const std::string &token = pr.first;
    //                 const int frequency = pr.second;
    //                 postings_list[token].push_back(std::make_pair(document_cnt, frequency));
    //             }
    //             docId_to_idx[doc.docID] = document_cnt ++;
    //         }
    //     }

    //     if(files_processed % 100 == 0)
    //     {
    //         // write partial postings list to disk
    //         std::string temporary_file = "./temp/temp-" + std::to_string(temporary_file_count++);
    //         posting_list_to_disk(postings_list, temporary_file);

    //         // clear memory
    //         postings_list.clear();
    //     }
    // }

    // // write remaining posting list to disk
    // if(postings_list.size())
    // {
    //     std::string temporary_file = "./temp/temp-" + std::to_string(temporary_file_count++);
    //     posting_list_to_disk(postings_list, temporary_file);
    // }

    // // Stop the clock and print the execution time
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    // std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;
    merge_postings_list_from_disk("./temp/temp-0", "./temp/temp-1", "test");

    return 0;
}