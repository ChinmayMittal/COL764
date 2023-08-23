#include <iostream>
#include <map>
#include <fstream>

#include "tokenizer.h"

void read_variable_byte(std::ifstream &postings_file, int &num)
{ 
    bool finished_reading = false;
    while(!finished_reading)
    {
        char byte;
        postings_file.read(&byte, 1); // Read 1 byte from the file

        if (postings_file.gcount() != 1) {
            std::cerr << "Failed to read " << 1 << " byte." << std::endl;
            return ;
        }
        std::string binary_string = std::bitset<8>(byte).to_string();
        std::bitset<7> bits(binary_string.substr(1));
        int integer_value = static_cast<int>(bits.to_ulong());
        num = num * (128) + integer_value;
        if(binary_string[0] == '1') finished_reading = true;
    }
}

std::vector<std::pair<int, int>> get_postings_list(std::string postings_file_path, int byte_offset, int postings_list_length)
{
    std::vector<std::pair<int, int>> postings_list;
    std::ifstream postings_file(postings_file_path, std::ios::binary); // Open the binary file in binary mode
    if (!postings_file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return postings_list;
    }

    postings_file.seekg(byte_offset, std::ios::beg);

    for(int idx = 0 ; idx < postings_list_length ; idx ++)
    {
        int document_id=0, term_frequency=0;
        read_variable_byte(postings_file, document_id);
        read_variable_byte(postings_file, term_frequency);
        postings_list.push_back({document_id, term_frequency});
    }
    return postings_list;
}

int main(int argc, char* argv[])
{
    std::string query = "International Organized Crime";
    SimpleTokenizer tokenizer(std::set<char>{'.', ' ', ':', ';', '\"', '\'', '.', '?', '!', ',', '\n'});

    std::vector<std::string> query_tokens = tokenizer.tokenize(query);
    std::map<std::string, int> query_token_cnt;
    for(auto const &token: query_tokens)
    {
        query_token_cnt[token] += 1;
    }
    std::vector<std::pair<int, int>>postings_list = get_postings_list("postings", 548275, 4);
    for(auto pr : postings_list)
    {
        std::cout << "<" << pr.first << ", " << pr.second << "> ";
    }

    return 0;
}