#ifndef HEADER_SuffixLabelMap_hh
#define HEADER_SuffixLabelMap_hh

#include <unordered_map>
#include <string>
#include <vector>

#include "exceptions.hh"

class Data;
class Sentence;
class Word;

typedef std::vector<unsigned int> LabelVector;

class SuffixLabelMap
{
public:
  SuffixLabelMap(unsigned int max_word_length=10);

  void train(const Data &data);
  void train(const Sentence &ds);
  void train(const Word &w);

  void normalize(void);
  
  void set_guesses(const std::string &word_form, LabelVector &v, float mass) const;

  bool operator==(const SuffixLabelMap &another) const;
  bool empty(void) const;

  void store(std::ostream &out) const;
  void load(std::istream &in, bool reverse_bytes);
private:
  typedef std::unordered_map<std::string, std::unordered_map<unsigned int, 
							     float> > 
  StringCountMap;

  typedef std::unordered_map<unsigned int, float> LabelCountMap;

  unsigned int max_word_length;
  float std_dev_tag_prob;
  StringCountMap suffix_label_probs;
  LabelCountMap label_probs;
  
  void count(const std::string &word_form, unsigned int label);
  void count(unsigned int label);
};

#endif // HEADER_SuffixLabelMap_hh
