#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <sstream>
#include <list>

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

std::vector<std::string> SimpleTokenizer::tokenize(const std::string& text) {
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

BPETokenizer::BPETokenizer(const std::set<char>& delimiters, const std::string initial_merge_file):
    delimiters(delimiters),
    pretokenizer(delimiters)
{
    std::ifstream initial_merges(initial_merge_file);
    if(!initial_merges)
    {
        std::cerr << "Cannot Load Initial Merge File " << std::endl;
        return;
    }
    std::string line;
    while(std::getline(initial_merges, line))
    {
        std::istringstream iss(line);
        std::string first, second;
        iss >> first >> second;
        merges.push_back(std::make_pair(first, second));
    }
    initial_merges.close();
}

std::unordered_map<std::pair<std::string, std::string>, uint32_t, PairHash> BPETokenizer::compute_pair_frequencies()
{
    std::unordered_map<std::pair<std::string, std::string>, uint32_t, PairHash>pair_frequencies;
    std::vector<std::string> words_to_be_deleted;
    for(auto const &pair : word_frequency)
    {
        const std::string &word = pair.first;
        uint32_t frequency = pair.second;
        std::vector<std::string> &current_word_split = splits[word];
        if(current_word_split.size() > 1)
        {
            for(int idx = 0; idx < current_word_split.size()-1; idx++)
            {
                pair_frequencies[std::make_pair(current_word_split[idx], current_word_split[idx+1])] += frequency;
            }
        }else{
            words_to_be_deleted.push_back(word);
        }
    }
    for(const auto &word : words_to_be_deleted)
    {
        word_frequency.erase(word_frequency.find(word));
        splits.erase(splits.find(word));
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

void BPETokenizer::train(std::vector<std::string> &corpus, const std::string &output_file_path, bool write_intermediate)
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

    // split each word
    for(auto const &pr : word_frequency)
    {
        const std::string &word = pr.first;
        for(int idx = 0; idx < word.size() ; idx++)
        {
            splits[word].push_back(word.substr(idx,1));
        }
    }
    std::cout << "Computed Initial Word Splits\n" ;

    if(merges.size())
    {
        // initial merges exist
        for(int merge_idx = 0; merge_idx < merges.size(); merge_idx++)
        {
            merge_pair(merges[merge_idx].first, merges[merge_idx].second);
            if(merge_idx % 100 == 0) 
                std::cout << "Processed " << merge_idx << " initial merges" << std::endl;
        }
        std::ofstream merge_file(output_file_path, std::ios::out | std::ios::app);
        if(!merge_file)
        {
            std::cerr << "Cannot open output file \n";
            return ;
        }
        for(int merge_idx = 0; merge_idx < merges.size() ; merge_idx ++)
        {
            merge_file << merges[merge_idx].first << " " << merges[merge_idx].second << "\n";
        }
        merges.clear();
        merge_file.close();


        std:: cout << "Processed Initial Merges\n";
    }
    uint32_t iter = 0;
    while(iter < max_iterations - total_initial_merges)
    {
        if(iter % 10 == 0)
            std::cout  << iter << "\n" ;
        auto start = std::chrono::high_resolution_clock::now();
        std::unordered_map<std::pair<std::string, std::string>, uint32_t, PairHash> pair_frequencies = compute_pair_frequencies();
        std::pair<std::string, std::string> most_frequent_pair;
        uint32_t most_frequent_pair_count = 0;
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
        if(iter % 10 == 0) std::cout << "Time taken by MFP Calc: " << duration.count() << " milliseconds" << std::endl ;
        if(most_frequent_pair_count == 0)
        {
            break;
        }
        merges.push_back(std::make_pair(most_frequent_pair.first, most_frequent_pair.second));
        start = std::chrono::high_resolution_clock::now();
        merge_pair(most_frequent_pair.first, most_frequent_pair.second);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        if(iter % 10 == 0)  std::cout << "Time taken by Merge Pair Calc: " << duration.count() << " milliseconds" << std::endl ;
        iter += 1;

        // write the current merges to a file
        if(write_intermediate and iter % 250 == 0)
        {
            std::ofstream merge_file(output_file_path, std::ios::out | std::ios::app);
            if(!merge_file)
            {
                std::cerr << "Cannot open output file \n";
                return ;
            }
            for(int merge_idx = 0; merge_idx < merges.size() ; merge_idx ++)
            {
                merge_file << merges[merge_idx].first << " " << merges[merge_idx].second << "\n";
            }
            merges.clear();
            merge_file.close();
        }
    }

    
    // write the remaining merges to a file
    std::ofstream merge_file(output_file_path, std::ios::out | std::ios::app);
    if(!merge_file)
    {
        std::cerr << "Cannot open output file \n";
        return ;
    }
    for(int merge_idx = 0; merge_idx < merges.size() ; merge_idx ++)
    {
        merge_file << merges[merge_idx].first << " " << merges[merge_idx].second << "\n";
    }
    merges.clear();
    merge_file.close();


}

void BPETokenizer::train(const std::string training_directory,const std::string output_file_path, const std::string initial_merges_file_path)
{
    std::vector<std::string> corpus;
    std::vector<fs::path> non_text_files;

    // load initial merges
    if(initial_merges_file_path != "")
    {
        std::cout << "Loading initial merges \n" ;
        std::ifstream initial_merges_file(initial_merges_file_path);
        if(! initial_merges_file)
        {
            std::cerr << "Cannot open initial merge file " << initial_merges_file_path << std::endl;
            return;
        }
        std::string line;
        unsigned int total_initial_merges = 0;
        while(std::getline(initial_merges_file, line))
        {
            std::istringstream iss(line);
            std::string first_string, second_string;
            iss >> first_string >> second_string;
            merges.push_back(std::make_pair(first_string, second_string));
            total_initial_merges ++ ;
        }
        this->total_initial_merges = total_initial_merges;
        initial_merges_file.close();
    }
    else
    {
        this->total_initial_merges = 0;
    }
    getNonTextFilesRecursively(training_directory, non_text_files);
    for(int idx = 0; idx < non_text_files.size(); idx += 3) // prune dataset
    {
        if(idx % 25 == 0 )
        {
            std::cout << idx << std::endl ;
        }
        std::vector<Document> documents = parse_file(non_text_files[idx]);
        for(int doc_idx = 0; doc_idx < documents.size(); doc_idx += 5) // prune dataset
        {
            std::string document_string = documents[doc_idx].title + " " + documents[doc_idx].content;
            for(auto & ch : document_string)
                ch = tolower(ch);
            corpus.push_back(document_string);
        }
    }
    std::cout << "Corpus Collected, Training ...\n";
    train(corpus, output_file_path, true);
}

std::vector<std::string> BPETokenizer::tokenize(const std::string &text)
{
    std::vector<std::string> pretokenized_text = pretokenizer.tokenize(text);
    std::unordered_map<std::string, std::list<std::string>> splits;
    for(auto const &word : pretokenized_text)
    {   
        if(word_to_tokens.count(word) == 0 && splits.count(word) == 0) // we don't have mapping for this word
        {
            for(int idx = 0 ; idx < word.size() ; idx++)
            {
                splits[word].push_back(word.substr(idx,1)); // create initial split
            }
        }
    }
    if(splits.size()) // word's exist for which we need to search merges
    {
        for (auto const& merge : merges) // will have to iterate over all merges to find right merges
        {
            const std::string &first_string = merge.first, &second_string = merge.second;
            std::vector<std::string> words_done;
            for(auto &pr : splits)
            {
                auto it = pr.second.begin();
                while(it != pr.second.end())
                {
                    if (*it == first_string && std::next(it) != pr.second.end() && *std::next(it) == second_string) {
                        it = pr.second.erase(it, std::next(it, 2));
                        pr.second.insert(it, first_string + second_string);
                    } else {
                        ++it;
                    }
                }
                if(pr.second.size() == 1) // this word has been processed
                {
                    words_done.push_back(pr.first);
                    word_to_tokens[pr.first].push_back(pr.second.front());
                }
            }
            for(auto &word : words_done)
            {
                splits.erase(word); // this words are fully processed and can be removed
            }
            if(splits.size() == 0) // no more words left to process no need to iterate remaining merges
            {
                break;
            }
        }
    }

    std::vector<std::string> tokens;
    for(int word_index = 0; word_index < pretokenized_text.size(); ++word_index)
    {
        std::string &word = pretokenized_text[word_index];
        if(word_to_tokens.count(word))
        {
            for(auto &token : word_to_tokens[word])
            {
                tokens.push_back(token);
            }
        }
        else{
            for(auto &token : splits[word])
            {
                tokens.push_back(token);
                word_to_tokens[word].push_back(token);
            }
        }
    }
    // std::cout << word_to_tokens.size() << std::endl;
    return tokens;

}

// int main(int argc, char* argv[])
// {
//     BPETokenizer tokenizer(std::set<char>{'.', ' ', ':', ';', '\"', '\'', '.', '?', '!', ',', '\n'}, 500);
//     // std::vector<std::string> corpus;
//     // corpus.push_back("This is part of the information retreival course");
//     // corpus.push_back("My name is chinmay");
//     // corpus.push_back("This is a randomly generated sentence");
//     // corpus.push_back("Tokenization algorithm using byte pair encoding");

//     // tokenizer.train(corpus);
//     tokenizer.train("./train_data/", "bpe_merges", "./data_files/bpe_merges");
//     std::string text = "This, is the \'text\' I want to tokenize; My name is:chinmay. "; 
//     // std::vector<std::string> tokens = tokenizer.tokenize(text);
//     // for(auto token : tokens)
//     //     std::cout << "-->" << token << "\n" ;


//     return 0;
// }
