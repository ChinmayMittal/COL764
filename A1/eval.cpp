#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <set>
#include <sstream>


int main(int argc, char* argv[])
{
    std::string results_file = argv[1];
    std::string ground_truth_file = argv[2];
    std::ifstream ground_truth(ground_truth_file);
    std::ifstream results(results_file);
    if(!results || !ground_truth)
    {
        std::cerr << "Error in opening file\n";
        return 1;
    }

    std::unordered_map<int, std::set<std::string>> query_number_to_relevant_documents;
    std::string line;
    while(std::getline(ground_truth, line))
    {
        std::istringstream iss(line);
        int query_number, iteration, relevance;
        std::string docID;
        iss >> query_number >> iteration >> docID >> relevance;
        // std::cout << query_number << " " << iteration << " " << docID << " " << relevance << "\n";
        if(relevance)
        {
            query_number_to_relevant_documents[query_number].insert(docID);
        }
    }
    ground_truth.close();
    int current_query_number = -1;
    int current_correct_count = 0;
    int query_results_count = 0;
    float average_f1 = 0.0;
    while(std::getline(results, line))
    {
        std::istringstream iss(line);
        int query_number, iteration, relevance;
        std::string docID;
        iss >> query_number >> iteration >> docID >> relevance;
        if(query_number != current_query_number)
        {
            if(query_number != -1)
                std::cout << current_query_number << " Correct: " << current_correct_count << " Average F1: "  << average_f1 / 4  << "\n";
            current_query_number=query_number;
            current_correct_count = 0;
            query_results_count = 0;
            average_f1 = 0;
        }
        if(query_number_to_relevant_documents[query_number].count(docID))
        {
            current_correct_count ++;
        }
        query_results_count ++;


        if(query_results_count % 25 == 0)
        {
            float precision = current_correct_count / (float) query_results_count ;
            float recall =  current_correct_count / (float) query_number_to_relevant_documents[query_number].size();
            float f1 = (precision + recall ? 2*(precision * recall) / (precision + recall) : 0);
            // std::cout << query_number << " " << query_results_count << "\n";
            // std::cout << "Precision: " << precision << "\n";
            // std::cout << "Recall: " << recall << "\n";
            // std::cout << "F1: " << f1 << "\n";
            average_f1 += f1;
        }
        

    }
    results.close();

    return 0;
}