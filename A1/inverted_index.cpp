#include <iostream>
#include <string>
#include <filesystem>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>
#include <cstdio>
#include <bitset>
#include <cassert>

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

void merge_helper(std::ifstream &file,
                  std::ofstream &temporary_file,
                  std::istringstream &iss,
                  std::string &word, int &num, std::string &line, bool &read)
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
        if(word1 < word2)
        {
            merge_helper(file1, temporary_file, iss1, word1, num1, line1, read1);
        }else if(word2 < word1)
        {
            merge_helper(file2, temporary_file, iss2, word2, num2, line2, read2);
        }
        else
        {
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
        merge_helper(file1, temporary_file, iss1, word1, num1, line1, read1);
    }

    while(read2)
    {
        merge_helper(file2, temporary_file, iss2, word2, num2, line2, read2);
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

std::string byte_to_bit_string(unsigned char byte) {
    return std::bitset<8>(byte).to_string();
}

std::vector<std::string>  variable_byte_encoding(int num)
{
    std::vector<std::string> variable_bytes;
    bool first = true;
    if(num ==0)
    {
        std::string bit_string = "10000000";
        variable_bytes.push_back(bit_string);
    }
    while(num)
    {
        std::string bit_string;
        if(num < (1<<7))
        {
            unsigned char byte_value = num;
            bit_string =  byte_to_bit_string(byte_value);
            num = 0;
        }else{
            unsigned char rem = num % (1<<7);
            bit_string = byte_to_bit_string(rem);
            num /= (1<<7);
        }
        if(first)
        {
            bit_string[0] = '1';
            first = false;
        }
        variable_bytes.push_back(bit_string);
    }
    std::reverse(variable_bytes.begin(), variable_bytes.end());
    return variable_bytes;
};

void write_byte_string(std::ofstream &postings_list_file, std::string &byte_string)
{
    unsigned char byte = 0;
    assert(byte_string.size() == 8);
    for (size_t i = 0; i < 8; ++i) {
        byte |= (byte_string[i] - '0') << (7 - i);
    }
    postings_list_file.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
}

int main(int argc, char *argv[]) {
    if(argc <=1)
    {
        std::cout << "Requires Training Directory as Script Argument" << std::endl;
        return 0;
    }
    std::string directory_path = argv[1];
    std::string temporary_directory_path = "./temp";
    std::string vocabulary;
    // Start the clock
    auto start = std::chrono::high_resolution_clock::now();

    // directory for temporary file storage during index construction
    if (fs::create_directory(temporary_directory_path)) {
        std::cout << "Directory created successfully." << std::endl;
    } else {
        std::cout << "Failed to create directory." << std::endl;
    }

    std::cout << "Training Directory: " << directory_path << std::endl;
    SimpleTokenizer tokenizer(std::set<char>{'.', ' ', ':', ';', '\"', '\'', '.', '?', '!', ',', '\n'});
    std::map<std::string, int> docId_to_idx;
    std::map<std::string, std::vector<std::pair<int, int>>> postings_list; // term --> list[doc_idx, term_frequency]
    int document_cnt = 0;

    int file_cnt = 0; // count the total number of files to be processed
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry)) {
            ++file_cnt;
        }
    }

    int files_processed = 0;
    int temporary_file_count = 0;
    for (const auto &file : fs::directory_iterator(directory_path)) {
        print_progrss(files_processed++, file_cnt);
        if (fs::is_regular_file(file)) {
            // process file
            std::vector<Document> documents = parse_file(file.path());
            for (auto const& doc : documents) {
                std::string document_content = doc.title + " " + doc.content;
                for(char &ch : document_content)
                    ch = tolower(ch);
                std::vector<std::string> tokens = tokenizer.tokenize(document_content);
                std::map<std::string, int> token_counts;
                for(auto const &token : tokens) token_counts[token] += 1;
                for(auto const &pr : token_counts)
                {
                    const std::string &token = pr.first;
                    const int frequency = pr.second;
                    postings_list[token].push_back(std::make_pair(document_cnt, frequency));
                }
                docId_to_idx[doc.docID] = document_cnt ++;
            }
        }

        if(files_processed % 100 == 0)
        {
            // write partial postings list to disk
            std::string temporary_file = "./temp/temp-" + std::to_string(temporary_file_count++);
            posting_list_to_disk(postings_list, temporary_file);

            // clear memory
            postings_list.clear();
        }
    }



    // write remaining posting list to disk
    if(postings_list.size())
    {
        std::string temporary_file = "./temp/temp-" + std::to_string(temporary_file_count++);
        posting_list_to_disk(postings_list, temporary_file);
    }

    std::cout << "Total Documents  " << document_cnt << "\n";
    std::ofstream document_file("documents", std::ofstream::out | std::ofstream::trunc); // normal file
    if(!document_file)
    {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
    for(auto const &pr : docId_to_idx)
    {
        document_file << pr.second << " " << pr.first << "\n";
    }
    document_file.close();

    // merge the postings list into one
    int current_count = temporary_file_count;
    while(current_count > 1)
    {
        std:: cout << "--" << current_count << "--" << "\n" ;
        for(int idx = 0; idx < current_count-1; idx+=2)
        {

            std::string file1 = "./temp/temp-" + std::to_string(idx);
            std::string file2 = "./temp/temp-" + std::to_string(idx+1);
            std::cout << file1 << " " << file2 << "\n" ; 
            merge_postings_list_from_disk(file1, file2, "./temp/test");
            if (std::remove(file1.c_str()) != 0) {
                std::cerr << "Error deleting the file." << std::endl;
                return 1;
            }
            if (std::remove(file2.c_str()) != 0) {
                std::cerr << "Error deleting the file." << std::endl;
                return 1;
            }
            if (std::rename("./temp/test", std::string("./temp/temp-" + std::to_string(idx/2)).c_str()) != 0) {
            std::cerr << "Error renaming the file." << std::endl;
            return 1;
            }
        }
        if(current_count % 2 == 1)
        {
            if (std::rename(std::string("./temp/temp-" + std::to_string(current_count-1)).c_str(), std::string("./temp/temp-" + std::to_string(current_count/2)).c_str()) != 0) {
            std::cerr << "Error renaming the file." << std::endl;
            return 1;
            }
        }
        current_count = (current_count+1)/2;
    }

    // create the compressed postings list file and vocabulary
    std::ofstream vocab_file("vocabulary", std::ofstream::out | std::ofstream::trunc); // normal file
    std::ofstream postings_list_file("postings", std::ios::binary | std::ios::trunc); // binary file
    if( !vocab_file || !postings_list_file)
    {
        std::cerr << "Error opening files." << std::endl;
        return 1;
    }

    std::ifstream uncomprssed_postings_file("./temp/temp-0");
    
    if(!uncomprssed_postings_file)
    {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    std::string line;
    int total_bytes_written =0;

    while(std::getline(uncomprssed_postings_file, line))
    {
        std::istringstream iss(line);
        std::string term;
        int term_frequency;
        iss >> term >> term_frequency;
        // write term and its document count and the offset in the posting file to the vocabulary file
        vocab_file << term << " " << term_frequency << " " << total_bytes_written << "\n" ;
        
        // write the posting list as bytes
        for(int idx = 0 ; idx < term_frequency ; idx++)
        {
            int doc_id, tf;
            iss >> doc_id >> tf;
            std::vector<std::string> variable_bytes_doc_id = variable_byte_encoding(doc_id);
            std::vector<std::string> variable_bytes_tf = variable_byte_encoding(tf);

            for(auto byte_string : variable_bytes_doc_id)
            {
                write_byte_string(postings_list_file, byte_string);
            }

            for(auto byte_string : variable_bytes_tf)
            {
                write_byte_string(postings_list_file, byte_string);
            }            
            total_bytes_written += variable_bytes_doc_id.size() + variable_bytes_tf.size();
        }
    }

    vocab_file.close();
    uncomprssed_postings_file.close();
    postings_list_file.close();

    // std::vector<std::string> variable_bytes = variable_byte_encoding(214577);

    // Stop the clock and print the execution time
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;

    return 0;
}