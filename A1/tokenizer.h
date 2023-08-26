#include <set>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

class SimpleTokenizer
{
    private:
        std::set<char> delimiters;

        bool is_delimiter(char ch) const;
    
    public:
        SimpleTokenizer(const std::set<char>& delimiters);
        std::vector<std::string> tokenize(const std::string& text) const;

};

struct VectorHash {
    template <typename T>
    size_t operator()(const std::vector<T>& v) const {
        size_t seed = v.size();
        for (const T& elem : v) {
            seed ^= std::hash<T>()(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};


class BPETokenizer
{
    private:
        std::set<char> delimiters;
        int max_iterations;
        std::unordered_map<std::string, uint64_t> word_frequency;
        std::unordered_map<std::string, std::vector<std::string>> splits;
        std::vector<std::string> vocabulary;
        SimpleTokenizer pretokenizer;
        std::unordered_map<std::vector<std::string>, uint64_t, VectorHash> compute_pair_frequencies();
        std::vector<std::vector<std::string>> merges;
        void merge_pair(std::string &first, std::string &second);

    public:
        BPETokenizer(const std::set<char>& delimiters, int max_iter);
        void train(std::vector<std::string> &corpus);
        void train(const std::string training_directory,const std::string output_file_path);
        std::vector<std::string> tokenize(std::string text);


};
