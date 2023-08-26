#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <unordered_map>

# include "tokenizer.h"
#include "document.h"
#include "pugixml.hpp"
#include "utils.h"

namespace fs = std::__fs::filesystem;


void getNonTextFilesRecursively(const fs::path& directory, std::vector<fs::path>& nonTextFiles) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            if (entry.path().extension() != ".txt") {
                nonTextFiles.push_back(entry.path());
            }
        } else if (fs::is_directory(entry.path())) {
            getNonTextFilesRecursively(entry.path(), nonTextFiles);
        }
    }
}

SimpleTokenizer::SimpleTokenizer(const std::set<char>& delimiters) : delimiters(delimiters)
{

}

bool SimpleTokenizer::is_delimiter(char ch) const {
    return delimiters.find(ch) != delimiters.end();
}

std::vector<std::string> SimpleTokenizer::tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = 0;

    while (end < text.length()) {
        while (end < text.length() && !is_delimiter(text[end])) {
            ++end;
        }

        if (start != end) {
            tokens.push_back(text.substr(start, end - start));
        }

        // skip delimiters
        while (end < text.length() && is_delimiter(text[end])) {
            ++end;
        }

        start = end;
    }

    return tokens;
}

BPETokenizer::BPETokenizer(const std::set<char>& delimiters, int max_iter):
    delimiters(delimiters),
    max_iterations(max_iter),
    pretokenizer(delimiters)
{

}


std::unordered_map<std::vector<std::string>, uint64_t, VectorHash> BPETokenizer::compute_pair_frequencies()
{
    std::unordered_map<std::vector<std::string>, uint64_t, VectorHash> pair_frequencies;
    for(auto const &pair : word_frequency)
    {
        const std::string &word = pair.first;
        uint64_t frequency = pair.second;
        std::vector<std::string> &current_word_split = splits[word];
        if(current_word_split.size() > 1)
        {
            for(int idx = 0; idx < current_word_split.size()-1; idx++)
            {
                std::vector<std::string> vocab_pair{current_word_split[idx], current_word_split[idx+1]};
                pair_frequencies[vocab_pair] += frequency;
            }
        }
    }
    return pair_frequencies;
}

void BPETokenizer::merge_pair(std::string &first, std::string &second)
{
    for(auto const &pr: word_frequency)
    {
        const std::string &word = pr.first;
        std::vector<std::string> &split = splits[word];
        if(split.size() != 1)
        {
            uint32_t idx = 0;
            while(idx < split.size()-1)
            {
                if(split[idx] == first and split[idx+1] == second)
                {
                    split.erase(split.begin() + idx, split.begin() + idx + 2);
                    split.insert(split.begin() + idx, first + second);
                }else{
                    idx ++;
                }
            }
        }
    }
}

void BPETokenizer::train(std::vector<std::string> &corpus)
{
    // compute word frequencies
    for(auto const &text : corpus)
    {
        std::vector<std::string> pre_tokenized_text = pretokenizer.tokenize(text);
        for(auto const &word : pre_tokenized_text)
        {
            word_frequency[word] += 1;
        }
    }
    std::cout << "Computed Word Frequencies\n" ;
    // compute initial alphabet
    std::set<std::string> alphabets;
    for(auto const &pr : word_frequency)
    {
        for(int idx = 0; idx < pr.first.size() ; idx++)
        {
            alphabets.insert(pr.first.substr(idx,1));
        }
    }
    vocabulary.push_back("<E>");
    for(auto const &alphabet: alphabets)
    {
        vocabulary.push_back(alphabet);
    }
    std::cout << "Computed Initial Vocabulary\n";
    // split each word
    for(auto const &pr : word_frequency)
    {
        std::string word = pr.first;
        for(int idx = 0; idx < word.size() ; idx++)
        {
            splits[word].push_back(word.substr(idx,1));
            // std::cout << word.substr(idx, 1)  << "--"<< std::endl;
        }
    }
    std::cout << "Computed Initial Word Splits\n" ;

    uint32_t iter = 0;

    while(iter < max_iterations)
    {
        if(iter % 10 == 0)
        {
            std::cout  << iter << "\n" ;
        }
        auto start = std::chrono::high_resolution_clock::now();
        std::unordered_map<std::vector<std::string>, uint64_t, VectorHash> pair_frequencies = compute_pair_frequencies();
        std::vector<std::string> most_frequent_pair;
        uint64_t most_frequent_pair_count = 0;
        for(auto &pr : pair_frequencies)
        {
            if(pr.second > most_frequent_pair_count)
            {
                most_frequent_pair = pr.first;
                most_frequent_pair_count = pr.second;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        std::cout << "Time taken by MFP Calc: " << duration.count() << " milliseconds" << std::endl ;
        if(most_frequent_pair_count == 0)
        {
            break;
        }
        // std::cout << most_frequent_pair[0] << " " << most_frequent_pair[1] << " " << most_frequent_pair_count << "\n" ;
        merges.push_back(most_frequent_pair);
        vocabulary.push_back(most_frequent_pair[0] + most_frequent_pair[1]);
        start = std::chrono::high_resolution_clock::now();
        merge_pair(most_frequent_pair[0], most_frequent_pair[1]);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        std::cout << "Time taken by Merge Pair Calc: " << duration.count() << " milliseconds" << std::endl ;
        iter += 1;
    }

}

void BPETokenizer::train(const std::string training_directory,const std::string output_file_path)
{
    std::vector<std::string> corpus;
    std::vector<fs::path> non_text_files;
    getNonTextFilesRecursively(training_directory, non_text_files);
    for(int idx = 0; idx < non_text_files.size(); idx += 5) // prune dataset
    {
        if(idx % 25 == 0 )
        {
            std::cout << idx << std::endl ;
        }
        std::vector<Document> documents = parse_file(non_text_files[idx]);
        for(int doc_idx = 0; doc_idx < documents.size(); doc_idx += 10) // prune dataset
        {
            std::string document_string = documents[doc_idx].title + " " + documents[doc_idx].content;
            for(auto & ch : document_string)
                ch = tolower(ch);
            corpus.push_back(document_string);
        }
    }
    std::cout << "Corpus Collected, Training ...\n";
    train(corpus);
    std::ofstream merge_file(output_file_path, std::ios::out | std::ios::trunc);
    if(!merge_file)
    {
        std::cerr << "Cannot open output file \n";
        return ;
    }
    for(int merge_idx = 0; merge_idx < merges.size() ; merge_idx ++)
    {
        merge_file << merges[merge_idx][0] << " " << merges[merge_idx][1] << "\n";
    }
    merge_file.close();
}

std::vector<std::string> BPETokenizer::tokenize(std::string text)
{
    std::vector<std::string> pretokenized_text = pretokenizer.tokenize(text);
    std::vector<std::vector<std::string>> splits;
    for(auto const &word : pretokenized_text)
    {
        std::vector<std::string> split;
        for(int idx = 0 ; idx < word.size() ; idx++)
        {
            split.push_back(word.substr(idx,1));
        }
        splits.push_back(split);
    }

    for (auto const& merge : merges)
    {
        const std::string &first_string = merge[0], &second_string = merge[1];
        for(int word_idx = 0; word_idx < splits.size(); ++word_idx)
        {
            uint32_t idx = 0;
            while(idx < splits[word_idx].size()-1)
            {
                if(splits[word_idx][idx] == first_string and splits[word_idx][idx+1]==second_string)
                {
                    splits[word_idx].erase(splits[word_idx].begin() + idx, splits[word_idx].begin() + idx + 2);
                    splits[word_idx].insert(splits[word_idx].begin() + idx, first_string + second_string);
                }else{
                    idx ++;
                }
            }

        }
    }

    std::vector<std::string> tokens;
    for(auto &split : splits)
    {
        for(auto &token : split)
        {
            tokens.push_back(token);
        }
    }
    return tokens;

}

int main(int argc, char* argv[])
{
    BPETokenizer tokenizer(std::set<char>{'.', ' ', ':', ';', '\"', '\'', '.', '?', '!', ',', '\n'}, 10000);
    // std::vector<std::string> corpus;
    // corpus.push_back("This is part of the information retreival course");
    // corpus.push_back("My name is chinmay");
    // corpus.push_back("This is a randomly generated sentence");
    // corpus.push_back("Tokenization algorithm using byte pair encoding");

    // tokenizer.train(corpus);
    tokenizer.train("./train_data/f1", "bpe_merges");
    std::string text = "This, is the \'text\' I want to tokenize; My name is:chinmay. "; 
    std::vector<std::string> tokens = tokenizer.tokenize(text);
    for(auto token : tokens)
        std::cout << "-->" << token << "\n" ;


    return 0;
}
