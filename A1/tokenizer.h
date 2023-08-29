#include <set>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

class Tokenizer {
public:
    virtual ~Tokenizer() {}
    virtual std::vector<std::string> tokenize(const std::string& text) = 0;
};

class SimpleTokenizer : public Tokenizer
{
    private:
        std::set<char> delimiters;

        bool is_delimiter(char ch) const;
    
    public:
        SimpleTokenizer(const std::set<char>& delimiters);
        std::vector<std::string> tokenize(const std::string& text);

};

struct PairHash {
    size_t operator()(const std::pair<std::string, std::string>& p) const {
        // Use std::hash for each string and combine the results
        size_t hash1 = std::hash<std::string>{}(p.first);
        size_t hash2 = std::hash<std::string>{}(p.second);

        // A common hash combining technique
        return hash1 ^ (hash2 << 1);
    }
};


class BPETokenizer : public Tokenizer
{
    private:
        std::set<char> delimiters;
        int max_iterations;
        int total_initial_merges;
        std::unordered_map<std::string, uint32_t> word_frequency;
        std::unordered_map<std::string, std::vector<std::string>> splits;
        SimpleTokenizer pretokenizer;
        std::unordered_map<std::pair<std::string, std::string>, uint32_t, PairHash> compute_pair_frequencies();
        std::vector<std::pair<std::string, std::string>> merges;
        void merge_pair(std::string &first, std::string &second);

    public:
        BPETokenizer(const std::set<char>& delimiters, int max_iter);
        BPETokenizer(const std::set<char>& delimiters, const std::string initial_merge_file);
        void train(std::vector<std::string> &corpus, const std::string &output_file_path, bool write_intermediate);
        void train(const std::string training_directory,const std::string output_file_path, const std::string initial_merges_file_path);
        std::unordered_map<std::string, std::vector<std::string>> word_to_tokens;
        std::vector<std::string> tokenize(const std::string &text);


};
