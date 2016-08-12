/* LabelExtractorWrapper.i */
%module guesser

%include "std_vector.i"
%include "std_string.i"
%include "typemaps.i"

 // Instantiate templates used
%template() std::vector<std::string>;

 %{
   #include <vector>
   #include <string>
   #include "LabelExtractorWrapper.hh"

   /* Put header files here or function declarations like below */
   extern LabelExtractor * get_label_guesser(unsigned int max_suffix_length);
   extern LabelExtractor * load(const char * filename);
   extern void store(const LabelExtractor * guesser, const char * filename);
   extern void train(std::vector<std::string> data, LabelExtractor * guesser);
   extern std::vector<std::string>  count_guess(std::string wf,
						 bool use_lexicon,
						 int guess_count,
						 const LabelExtractor * guesser);
   extern std::vector<std::string>  mass_guess(std::string wf,
						bool use_lexicon,
						float guess_mass,
						const LabelExtractor * guesser);

   %}


extern LabelExtractor * get_label_guesser(unsigned int max_suffix_length);
extern LabelExtractor * load(const char * filename);
extern void store(const LabelExtractor * guesser, const char * &filename);
extern void train(std::vector<std::string> data, LabelExtractor * guesser);
extern std::vector<std::string>  count_guess(std::string wf,
					     bool use_lexicon,
					     int guess_count,
					     const LabelExtractor * guesser);
extern std::vector<std::string>  mass_guess(std::string wf,
					    bool use_lexicon,
					    float guess_mass,
					    const LabelExtractor * guesser);
