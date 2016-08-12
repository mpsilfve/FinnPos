#include <fstream>
#include <iostream>

#include "LabelExtractorWrapper.hh"
#include "Data.hh"
#include "Sentence.hh"
#include "Word.hh"
#include "io.hh"

LabelExtractor * get_label_guesser(unsigned int max_suffix_length)
{ return new LabelExtractor(max_suffix_length,1); }

LabelExtractor * load(const char * filename)
{
  LabelExtractor * guesser = new LabelExtractor();
  std::ifstream in(filename);

  guesser->load(in, 0);
  return guesser;
}

void store(const LabelExtractor * guesser, const char * filename)
{
  std::ofstream out(filename);
  guesser->store(out);
}

void train(std::vector<std::string> data, LabelExtractor * guesser)
{
  static_cast<void>(guesser->get_boundary_label());
  Data train_data;
  for (int i = 0; i < data.size(); ++i)
    {
      StringVector wf_and_label;
      split(data.at(i), wf_and_label, '\t');

      Word w(wf_and_label[0], 
	     FeatureTemplateVector(), 
	     LabelVector(1, guesser->get_label(wf_and_label[1])),
	     "");
      w.set_label(guesser->get_label(wf_and_label[1]));

      WordVector wv(1,w);
      Sentence s(wv, *guesser, 2);
      train_data.push_back(s);
    }
  train_data.randomize();
  guesser->train(train_data);
}

std::vector<std::string>  count_guess(std::string wf, 
				     bool use_lexicon,
				     int guess_count,
				     const LabelExtractor * guesser)
{
  LabelVector label_ids;
  guesser->set_label_candidates(wf,
				use_lexicon,
				1.0,
				label_ids,
				guess_count);

  std::vector<std::string>  labels;

  for (unsigned int i = 0; i < label_ids.size(); ++i)
    {
      labels.push_back(guesser->get_label_string(label_ids[i]));
    }

  return labels;
}

std::vector<std::string>  mass_guess(std::string wf, 
				    bool use_lexicon,
				    float guess_mass,
				    const LabelExtractor * guesser)
{
  LabelVector label_ids;
  guesser->set_label_candidates(wf,
				use_lexicon,
				guess_mass,
				label_ids);
  
  std::vector<std::string> labels;

  for (unsigned int i = 0; i < label_ids.size(); ++i)
    {
      labels.push_back(guesser->get_label_string(label_ids[i]));
    }

  return labels;
}

#ifdef TEST_LabelExtractorWrapper_cc

#include <cassert>

int main()
{
  LabelExtractor guesser = get_label_guesser(10);

  std::vector<std::string> data;
  data.push_back("koira\t[UPOS=NOUN][NUM=SG][CASE=NOM]");
  data.push_back("koiran\t[UPOS=NOUN][NUM=SG][CASE=GEN]");
  data.push_back("koirat\t[UPOS=NOUN][NUM=PL][CASE=NOM]");
  data.push_back("koirien\t[UPOS=NOUN][NUM=PL][CASE=GEN]");

  train(data, guesser);
  std::vector<std::string> res = count_guess("kissan",
					    0,
					    1,
					    guesser);

  assert(res.size() == 1 && res[0] == "[UPOS=NOUN][NUM=SG][CASE=GEN]");

  store(guesser,"foo");
  LabelExtractor guesser_copy = load("foo");
  res = count_guess("kissan",
		    0,
		    1,
		    guesser_copy);
  
  assert(res.size() == 1 && res[0] == "[UPOS=NOUN][NUM=SG][CASE=GEN]"); 
}
#endif
