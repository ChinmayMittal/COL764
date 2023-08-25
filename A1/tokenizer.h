#include <set>
#include <vector>
#include <string>
#include <map>

class SimpleTokenizer
{
    private:
        std::set<char> delimiters;

        bool is_delimiter(char ch) const;
    
    public:
        SimpleTokenizer(const std::set<char>& delimiters);
        std::vector<std::string> tokenize(const std::string& text) const;

};

class BPETokenizer
{
    private:
        std::set<char> delimiters;
        int max_iterations;
        std::map<std::string, uint64_t> word_frequency;
        std::map<std::string, std::vector<std::string>> splits;
        std::vector<std::string> vocabulary;
        SimpleTokenizer pretokenizer;
        std::map<std::vector<std::string>, uint64_t> compute_pair_frequencies();
        std::vector<std::vector<std::string>> merges;

    public:
        BPETokenizer(const std::set<char>& delimiters, int max_iter);
        void train(std::vector<std::string> &corpus);
        void merge_pair(std::string &first, std::string &second);


};
