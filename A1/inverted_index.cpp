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
#include <unordered_map>

#include "document.h"
#include "pugixml.hpp"
#include "tokenizer.h"
#include "utils.h"

namespace fs = std::__fs::filesystem;


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

std::vector<std::string> fixed_byte_encoding(int number)
{
    std::vector<std::string> fixed_bytes;
    for (int i = 0; i < 4; ++i) {
        // Extract each byte from the integer, starting from the least significant byte
        unsigned char byte = (number >> (i * 8)) & 0xFF;
        std::bitset<8> bits(byte);
        fixed_bytes.push_back(bits.to_string());
    }
    return fixed_bytes;
}



void write_byte_string(std::ofstream &postings_list_file, std::string &byte_string)
{
    unsigned char byte = 0;
    assert(byte_string.size() == 8);
    for (size_t i = 0; i < 8; ++i) {
        byte |= (byte_string[i] - '0') << (7 - i);
    }
    postings_list_file.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
}

void write_multiple_byte_strings(std::ofstream &postings_list_file, std::vector<std::string> &byte_strings)
{
    size_t num_bytes = byte_strings.size();
    std::vector<unsigned char> buffer(num_bytes, 0);     // Construct a buffer to hold the binary data
    unsigned char byte;
    // Populate the buffer using bitwise operations
    for (size_t i = 0; i < byte_strings.size(); ++i) {
        std::string &byte_string = byte_strings[i];
        byte = 0;
        assert(byte_string.size() == 8);
        for (size_t i = 0; i < 8; ++i) {
            byte |= (byte_string[i] - '0') << (7 - i);
        }
        buffer[i] = byte; 
    }
    // Write the entire buffer to the file
    postings_list_file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(byte));
}

int main(int argc, char *argv[]) {
    // TODO 
    /*
    Command line arguments for type of tokenizer and the type of compression 
    */
    if(argc <=3)
    {
        std::cout << "Requires Training Directory as Script Argument and Tokenizer and Compression Type" << std::endl;
        return 0;
    }
    std::string directory_path = argv[1];
    int compression_arg = std::stoi(argv[2]);
    int tokenizer_arg = std::stoi(argv[3]);
    assert(compression_arg == 0 || compression_arg == 1);
    assert(tokenizer_arg == 0 || tokenizer_arg == 1);

    std::string temporary_directory_path = "./temp"; // temporary directory used to store files
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
    std::cout << (compression_arg ? "Compression" : "No Compression") << std::endl;
    Tokenizer* tokenizer = nullptr;
    if(tokenizer_arg == 0)
    {
        std::cout  << "Simple Tokenizer\n";
        tokenizer = new SimpleTokenizer(get_base_delimiters());
    }else{
        std::cout << "BPE Tokenizer\n";
        tokenizer = new BPETokenizer(get_base_delimiters(), "./bpe_merges");
    }

    std::unordered_map<std::string, int> docId_to_idx;
    std::map<std::string, std::vector<std::pair<int, int>>> postings_list; // term --> list[doc_idx, term_frequency], map is required since we want the vocabulary to be sorted
    int document_cnt = 0;

    int file_cnt = 0; // count the total number of files to be processed
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry)) {
            ++file_cnt;
        }
    }

    int files_processed = 0;
    int temporary_file_count = 0;
    unsigned int posting_file_bytes_estimate = 0;
    for (const auto &file : fs::directory_iterator(directory_path)) {
        print_progress(files_processed++, file_cnt);
        if (fs::is_regular_file(file)) {
            // process file
            std::vector<Document> documents = parse_file(file.path());
            for (auto const& doc : documents) {
                std::string document_content = doc.title + " " + doc.content;
                for(char &ch : document_content)
                    ch = tolower(ch);
                std::vector<std::string> tokens = tokenizer->tokenize(document_content);
                std::unordered_map<std::string, int> token_counts;
                for(auto const &token : tokens) token_counts[token] += 1;
                for(auto const &pr : token_counts)
                {
                    const std::string &token = pr.first;
                    const int frequency = pr.second;
                    postings_list[token].push_back(std::make_pair(document_cnt, frequency));
                    posting_file_bytes_estimate += 8;
                }
                docId_to_idx[doc.docID] = document_cnt ++;
            }
        }

        // 500 MB, limit to the posting list in RAM
        if( posting_file_bytes_estimate + (10*postings_list.size()) >= 5e8) // improve this metric and reduce writes to disk
        {
            // write partial postings list to disk
            posting_file_bytes_estimate = 0;
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

    // Stop the clock and print the execution time
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Execution time for Creating Initial Postings List: " << duration.count() << " seconds" << std::endl;

    
    start = std::chrono::high_resolution_clock::now();
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
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Execution time for Merging Initial Postings List: " << duration.count() << " seconds" << std::endl;

    
    // create the compressed postings list file and vocabulary
    start = std::chrono::high_resolution_clock::now();
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
    // write the tokenizer type and compression type to the postings list file
    char byte_value = static_cast<char>(compression_arg);
    postings_list_file.write(&byte_value, sizeof(byte_value));
    byte_value = static_cast<char>(tokenizer_arg);
    postings_list_file.write(&byte_value, sizeof(byte_value));
    int total_bytes_written = 2;

    std::unordered_map<unsigned int, float> normalized_document_vector_norms; // store the normalized document vector norms
    while(std::getline(uncomprssed_postings_file, line)) // iterates over the vocabulary and the corresponding postings list
    {
        std::istringstream iss(line);
        std::string term;
        int term_doc_frequency;
        iss >> term >> term_doc_frequency;
        // write term and its document count and the offset in the posting file to the vocabulary file
        vocab_file << term << " " << term_doc_frequency << " " << total_bytes_written << "\n" ;
        
        
        // write the posting list as bytes
        int previous_value = -1;
        std::vector<std::string> byte_strings;
        for(int idx = 0 ; idx < term_doc_frequency ; idx++)
        {
            int doc_id, tf, doc_id_to_be_stored;
            iss >> doc_id >> tf;

            // [TODO] implement no compression 
            doc_id_to_be_stored = previous_value != -1 ? doc_id - previous_value : doc_id; // gap encoding
            previous_value = doc_id;
            std::vector<std::string> bytes_doc_id, bytes_tf;
            if(compression_arg == 1)
            {
                // Variable Byte Encoding
                bytes_doc_id = variable_byte_encoding(doc_id_to_be_stored);
                bytes_tf = variable_byte_encoding(tf);
            }else if(compression_arg == 0)
            {
                // Fixed Byte Encoding
                bytes_doc_id = fixed_byte_encoding(doc_id);
                bytes_tf = fixed_byte_encoding(tf);
            }
            previous_value = doc_id;

            for(auto byte_string : bytes_doc_id)
                byte_strings.push_back(byte_string);

            for(auto byte_string : bytes_tf)
                byte_strings.push_back(byte_string);
                      
            total_bytes_written += bytes_doc_id.size() + bytes_tf.size();

            // update the document vector norms
            float document_vector_element = inverse_document_frequency(term_doc_frequency, document_cnt) * tf;
            normalized_document_vector_norms[doc_id] += document_vector_element * document_vector_element;
        }
        write_multiple_byte_strings(postings_list_file, byte_strings);
    }

    uncomprssed_postings_file.close();
    postings_list_file.close();

    // write document id to document idx mappings to the vocabulary file
    vocab_file << "\n" ; // seperator to seperate the document mappings and the actual vocabulary

    // also write normalized document vector norms and idx to id mappings to this file
    for(auto const &pr : docId_to_idx)
    {
        vocab_file << pr.second << " " << pr.first << " " << sqrt(normalized_document_vector_norms[pr.second]) <<  "\n";
    }
    vocab_file.close();


    // Stop the clock and print the execution time
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Execution time for Creating Postings List and Vocabulary File: " << duration.count() << " seconds" << std::endl;

    return 0;
}