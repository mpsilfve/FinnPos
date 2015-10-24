#include "SuffixLabelMap.hh"
#include <cassert>

#ifndef TEST_SuffixLabelMap_cc

#define MAX_SUFFIX_LEN 10

#include <algorithm>
#include <utility>

#include "Data.hh"

SuffixLabelMap::SuffixLabelMap(unsigned int max_word_length):
  max_word_length(max_word_length),
  std_dev_tag_prob(-1.0)
{}

void SuffixLabelMap::train(const Data &data)
{
  for (unsigned int i = 0; i < data.size(); ++i)
    { train(data.at(i)); }

  normalize();
}

void SuffixLabelMap::train(const Sentence &s)
{
  for (unsigned int i = 0; i < s.size(); ++i)
    { train(s.at(i)); }
}

void SuffixLabelMap::train(const Word &w)
{
  std::string word_form = w.get_word_form();
  unsigned int label    = w.get_label();
  
  if (word_form.size() <= max_word_length)
    { 
      if (word_form == BOUNDARY_WF)
	{ return; }

      count(word_form, label);
      count(label);
    }
  else
    { static_cast<void>(label_probs[label]); }
}

void SuffixLabelMap::count(const std::string &word_form, 
			   unsigned int label)
{
  for (unsigned int i = 0; i < word_form.size() + 1; ++i)
    {
      if (word_form.size() - i > MAX_SUFFIX_LEN)
	{ continue; }

      suffix_label_probs[word_form.substr(i)][label] += 1;
    }
}

void SuffixLabelMap::count(unsigned int label)
{ label_probs[label] += 1; }

void SuffixLabelMap::normalize(void)
{
  for (StringCountMap::iterator it = suffix_label_probs.begin();
       it != suffix_label_probs.end();
       ++it)
    { 
      float tot = 0;

      for (std::unordered_map<unsigned int, float>::iterator 
	     jt = it->second.begin();
	   jt != it->second.end(); 
	   ++jt)
	{ tot += jt->second; }
      
      // tot cannot be 0. very probably :)
      assert(not it->second.empty());
      assert(tot);

      for (std::unordered_map<unsigned int, float>::iterator 
	     jt = it->second.begin();
	   jt != it->second.end(); 
	   ++jt)
	{ jt->second /= tot; }
    }

  float tot = 0;

  for (LabelCountMap::iterator it = label_probs.begin();
       it != label_probs.end();
       ++it)
    { tot += it->second; }

  if (tot == 0)
    { 
      std_dev_tag_prob = 0; 
      return;
    }

  // Compute the standard deviation of the probabilities of labels.
  std_dev_tag_prob = 0;

  for (LabelCountMap::iterator it = label_probs.begin();
       it != label_probs.end();
       ++it)
    { 
      it->second /= tot; 
      std_dev_tag_prob += pow(it->second - 1.0/label_probs.size(), 2);
    }
  
  std_dev_tag_prob = pow(std_dev_tag_prob / (label_probs.size() - 1), 0.5);

}

void SuffixLabelMap::store(std::ostream &out) const
{
  write_val(out, max_word_length);
  write_val(out, std_dev_tag_prob);
  write_map(out, suffix_label_probs);
  write_map(out, label_probs);
}

void SuffixLabelMap::load(std::istream &in, bool reverse_bytes)
{
  max_word_length = 0;
  std_dev_tag_prob = 0;
  suffix_label_probs.clear();
  label_probs.clear();

  read_val(in, max_word_length, reverse_bytes);
  read_val(in, std_dev_tag_prob, reverse_bytes);
  read_map(in, suffix_label_probs, reverse_bytes);
  read_map(in, label_probs, reverse_bytes);
}

bool SuffixLabelMap::empty(void) const
{ return suffix_label_probs.size() == 0; }

void SuffixLabelMap::set_guesses(const std::string &word_form, 
				 LabelVector &v, 
				 float mass,
				 int candidate_count) const
{
  assert(suffix_label_probs.count(""));
  static_cast<void>(v);
  static_cast<void>(mass);
  std::unordered_map<unsigned int, float> probs = 
    suffix_label_probs.find("")->second;

  for (int i = static_cast<int>(word_form.size()); i >= 0; --i)
    {
      if (word_form.size() - i > MAX_SUFFIX_LEN)
	{ continue; }

      const std::string &substr = word_form.substr(i);

      if (suffix_label_probs.count(substr))
	{
	  const std::unordered_map<unsigned int, float> &p = 
	    suffix_label_probs.find(substr)->second;

	  for (std::unordered_map<unsigned int, float>::const_iterator 
		 it = probs.begin();
	       it != probs.end();
	       ++it)
	    { 
	      float label_prob = 0;
	      
	      if (p.count(it->first) != 0)
		{ label_prob = p.find(it->first)->second; }
	      
	      probs[it->first] *= std_dev_tag_prob;
	      probs[it->first] += label_prob;
	      probs[it->first] /= (1 + std_dev_tag_prob);
	    }
	}
      else
	{ 
	  break; 
	}
    }

  std::vector<std::pair<float, unsigned int> > label_prob_pairs;

  for (std::unordered_map<unsigned int, float>::const_iterator 
	 it = probs.begin();
       it != probs.end();
       ++it)
    { 
      label_prob_pairs.push_back(std::pair<float, unsigned int>
				 (it->second, it->first));
    }
  
  std::sort(label_prob_pairs.begin(), label_prob_pairs.end());
  std::reverse(label_prob_pairs.begin(), label_prob_pairs.end());

  float tentative_mass = 0;

  for (unsigned int i = 0; i < label_prob_pairs.size(); ++i)
    { 
      v.push_back(label_prob_pairs[i].second);

      tentative_mass += label_prob_pairs[i].first;
      if (candidate_count != -1 and 
	  static_cast<int>(i) >= candidate_count)
	{ break; }

      if (tentative_mass > mass and i > 20)
	{ break; }      
    }
}

bool SuffixLabelMap::operator==(const SuffixLabelMap &another) const
{
  return 
    max_word_length == another.max_word_length and
    std_dev_tag_prob == another.std_dev_tag_prob and
    suffix_label_probs == another.suffix_label_probs and
    label_probs == another.label_probs;
}

#else // TEST_SuffixLabelMap_cc

#include <sstream>

#include "Word.hh"

int main(void)
{
  SuffixLabelMap slm(11);
  
  FeatureTemplateVector fv(1);
  LabelVector lv(1);
  
  Word w1("koira",fv,lv,"");
  w1.set_label(1);

  Word w2("koiran",fv,lv,"");
  w2.set_label(2);

  Word w3("kuu",fv,lv,"");
  w3.set_label(1);

  Word w4("kuun",fv,lv,"");
  w4.set_label(2);

  Word w5("qwertykuu",fv,lv,"");
  w5.set_label(1);

  Word w6("qwertykuun",fv,lv,"");
  w6.set_label(2);

  Word w7("qwertyuikuu",fv,lv,"");
  w7.set_label(1);

  Word w8("qwertyiikuun",fv,lv,"");
  w8.set_label(2);

  slm.train(w1);
  slm.train(w2);
  slm.train(w3);
  slm.train(w4);
  slm.train(w5);
  slm.train(w6);
  slm.train(w7);
  slm.train(w8);

  slm.normalize();

  LabelVector label_guesses;
  slm.set_guesses("qwweerkissan",label_guesses,0.5);
  assert(label_guesses.size() == 1);
  assert(label_guesses[0] == 2);

  label_guesses.clear();
  slm.set_guesses("qwweerkissan",label_guesses,1);
  assert(label_guesses.size() == 2);
  assert(label_guesses[0] == 2);
  assert(label_guesses[1] == 1);

  std::ostringstream slm_out;
  slm.store(slm_out);
  std::istringstream slm_in(slm_out.str());
  SuffixLabelMap slm_copy;
  slm_copy.load(slm_in, false);
  assert(slm == slm_copy);

  SuffixLabelMap empty_slm(0);
  std::ostringstream empty_out;
  empty_slm.store(empty_out);
  std::istringstream empty_in(empty_out.str());
  SuffixLabelMap empty_slm_copy;
  empty_slm_copy.load(empty_in, false);
  assert(empty_slm == empty_slm_copy); 
}

#endif // TEST_SuffixLabelMap_cc
