# include "tokenizer.h"
#include <iostream>

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

std::map<std::vector<std::string>, uint64_t> BPETokenizer::compute_pair_frequencies()
{
    std::map<std::vector<std::string>, uint64_t> pair_frequencies;
    for(auto const &pair : word_frequency)
    {
        std::string word = pair.first;
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
    for(auto const & pr: word_frequency)
    {
        std::string word = pr.first;
        std::vector<std::string> split = splits[word];
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
        splits[word] = split;
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
    // compute initial alphabet
    std::vector<std::string> alphabets;
    for(auto const &pr : word_frequency)
    {
        for(int idx = 0; idx < pr.first.size() ; idx++)
        {
            alphabets.push_back(pr.first.substr(idx,1));
        }
    }
    sort(alphabets.begin(),alphabets.end());
    vocabulary.push_back("<E>");
    for(auto const &alphabet: alphabets)
    {
        vocabulary.push_back(alphabet);
    }

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

    uint32_t iter = 0;

    while(iter < max_iterations)
    {
        std::map<std::vector<std::string>, uint64_t> pair_frequencies = compute_pair_frequencies();
        std::vector<std::string> most_frequent_pair;
        uint64_t most_frequent_pair_count = 0;
        for(auto pr : pair_frequencies)
        {
            if(pr.second > most_frequent_pair_count)
            {
                most_frequent_pair = pr.first;
                most_frequent_pair_count = pr.second;
            }
        }
        if(most_frequent_pair_count == 0)
        {
            break;
        }
        std::cout << most_frequent_pair[0] << " " << most_frequent_pair[1] << " " << most_frequent_pair_count << "\n" ;
        merges.push_back(most_frequent_pair);
        vocabulary.push_back(most_frequent_pair[0] + most_frequent_pair[1]);
        merge_pair(most_frequent_pair[0], most_frequent_pair[1]);
        iter += 1;
    }

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
    BPETokenizer tokenizer(std::set<char>{'.', ' ', ':', ';', '\"', '\'', '.', '?', '!', ',', '\n'}, 10);
    std::vector<std::string> corpus;
    corpus.push_back("This is part of the information retreival course");
    corpus.push_back("My name is chinmay");
    corpus.push_back("This is a randomly generated sentence");
    corpus.push_back("Tokenization algorithm using byte pair encoding");

    tokenizer.train(corpus);
    std::string text = "This, is the \'text\' I want to tokenize; My name is:chinmay. "; 
    std::vector<std::string> tokens = tokenizer.tokenize(text);
    for(auto token : tokens)
        std::cout << "-->" << token << "\n" ;
    return 0;
}
