#include "document.h"

void print_progress(int steps_completed, int total_steps);
std::vector<Document> parse_file(std::string file);
float inverse_document_frequency(float document_frequency, float total_documents);
float term_frequency(float term_count);