#include <iostream>
#include <sstream>
#include <fstream>
#include <queue>
#include <map>

#include "tokenizer.h"
#include "utils.h"

struct Query {
    int number;
    std::string title;
    std::string description;

    // Constructor
    Query(int num, const std::string& tit, const std::string& desc) 
        : number(num), title(tit), description(desc) {}
};

std::vector<Query> parseQueries(const std::string& filename) {
    std::vector<Query> queries;
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return queries;  // return empty vector
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line == "<top>") {
            int number = 0;
            std::string title, description;

            while (std::getline(file, line) && line != "</top>") {
                if (line.find("<num>") != std::string::npos) {
                    std::istringstream iss(line);
                    iss >> line >> line ;  // Discard "<num> Number:"
                    iss >> number;
                } else if (line.find("<title>") != std::string::npos) {
                    title = line.substr(7);
                } else if (line.find("<desc>") != std::string::npos) {
                    // Concatenate multiple lines until we encounter another tag (e.g., <narr>)
                    while (std::getline(file, line) && line.find('<') == std::string::npos) {
                        description += line + " ";  // Add a space for separation
                    }
                }
            }

            queries.push_back(Query(number, title, description));
        }
    }

    file.close();
    return queries;
}



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
    if(argc < 3)
    {
        std::cout << "Script Requires Query File and Output File paths\n";
        return 1;
    }
    std::string query_file_path = argv[1];
    std::string output_file_path = argv[2];
    std::cout << "Query File: " << query_file_path << "\n";
    std::cout << "Output File path: " << output_file_path << "\n";

    std::ofstream output_file;
    output_file.open(output_file_path, std::ios::out | std::ios::trunc);
    if (!output_file) {  // Check if the file was opened successfully
        std::cerr << "Unable to open file for writing";
        return 1;
    }

    SimpleTokenizer tokenizer(std::set<char>{'.', ' ', ':', ';', '\"', '\'', '.', '?', '!', ',', '\n'});

    // read the query files and get all queries
    std::vector<Query> queries = parseQueries(query_file_path);

    std::map<std::string, std::pair<int,int>> vocab_dict; // term --> <doc_freq, byte_offset>
    std::ifstream vocab_file("vocabulary"); // find a more effecient way to do this
    if(!vocab_file)
    {
        std::cerr << "Error opening the file." << std::endl;
        return 1;
    }
    std::string line;
    while(std::getline(vocab_file, line))
    {
        if(line.length() == 0)
            break ; // this line marks the end of the vocabulary and start of document mappings
        std::istringstream iss(line);
        std::string vocab_term;
        iss >> vocab_term;
        int document_frequency, byte_offset;
        iss >> document_frequency >> byte_offset;
        vocab_dict[vocab_term] = std::make_pair(document_frequency, byte_offset);
    }
    // store document mappings
    std::unordered_map<int, std::string> document_idx_to_id;
    std::unordered_map<int, float> normalized_document_vector_norms;
    unsigned total_document_count = 0;
    while(std::getline(vocab_file, line))
    {
        std::istringstream iss(line);
        unsigned int document_idx;
        std::string document_id;
        float document_vector_norm;
        iss >> document_idx >> document_id >> document_vector_norm;
        total_document_count ++;
        document_idx_to_id[document_idx] = document_id;
        normalized_document_vector_norms[document_idx] = document_vector_norm;
    }
    vocab_file.close();
    // // tokenize the query

    for(auto const &query : queries)
    {
        std::string query_content = query.title + " " + query.description ; // create query content only using title and description
        for(auto &ch : query_content) ch = tolower(ch); // convert to lower case
        std::vector<std::string> query_tokens = tokenizer.tokenize(query_content);
        std::map<std::string, int> query_token_cnt;
        for(auto const &token: query_tokens)
            query_token_cnt[token] += 1;
        
        std::vector<float> document_scores(total_document_count, 0); // final document scores for this query
        std::vector<int> document_term_counts(total_document_count, 0); // count of query terms in the document

        for(auto const & query_token_pair : query_token_cnt)
        {
            // iterate over query tokens
            std::string query_term = query_token_pair.first;
            unsigned int query_term_frequency = query_token_pair.second;

            // get postings list corresponding to the term
            // find the idf for that token and the offsets in the postings list
            int document_frequency, byte_offset;
            if(vocab_dict.count(query_term) == 0)
                continue; // ignore query term if not in are in vocabulary
            document_frequency = vocab_dict[query_term].first;
            byte_offset = vocab_dict[query_term].second;
            std::cout << document_frequency << " " << byte_offset << std::endl;
            std::vector<std::pair<int, int>>postings_list = get_postings_list("postings", byte_offset, document_frequency);
            for(auto const &pr : postings_list)
            {
                int document_id = pr.first;
                int term_frequency_in_document = pr.second;
                document_scores[document_id] += term_frequency(term_frequency_in_document) * term_frequency(query_term_frequency) \
                                                * inverse_document_frequency(document_frequency, total_document_count) * inverse_document_frequency(document_frequency, total_document_count);
                document_term_counts[document_id] += 1;
            }

    
            vocab_file.close();
        }
        // need to normalize document scores [TODO]
        for(int idx = 0; idx < document_scores.size(); idx++) document_scores[idx] /= normalized_document_vector_norms[idx];

        // get the best documents
        // min heap for this purpose stores <score, id> for the best documents
        std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> minHeap;
        int best_k = 10; // to be experimented with
        for(int document_idx = 0; document_idx < document_scores.size(); ++document_idx)
        {
            if(minHeap.size() < best_k)
            {
                minHeap.push({document_scores[document_idx], document_idx});
            }else{
                if(minHeap.top().first < document_scores[document_idx])
                {
                    // replace min heap elements
                    minHeap.pop();
                    minHeap.push({document_scores[document_idx], document_idx});
                }
            }
        }

        std::vector<int> best_documents;
        while(!minHeap.empty())
        {
            best_documents.push_back(minHeap.top().second);
            minHeap.pop();
        }

        for(int idx = best_documents.size()-1 ; idx >= 0 ; idx--) {
            output_file << query.number << " 0 " << document_idx_to_id[best_documents[idx]]  << " " << 1 << "\n";
        }
    }

    output_file.close();

    return 0;
}