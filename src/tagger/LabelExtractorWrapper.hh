#include <string>
#include <vector>

#include "LabelExtractor.hh"

#ifndef HEADER_LabelExtractorWrapper_hh

LabelExtractor * get_label_guesser(unsigned int max_suffix_length);

LabelExtractor * load(const char  * filename);
void store(const LabelExtractor * guesser, const std::string  * filename);

void train(std::vector<std::string> data, LabelExtractor * guesser);

std::vector<std::string>  count_guess(std::string wf, 
				       bool use_lexicon,
				       int guess_count,
				       const LabelExtractor * guesser);

std::vector<std::string>  mass_guess(std::string wf, 
				      bool use_lexicon,
				      float guess_mass,
				      const LabelExtractor * guesser);

#endif
