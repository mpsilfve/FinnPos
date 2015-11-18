/**
 * @file    LemmaExtractor.cc                                                
 * @Author  Miikka Silfverberg                                               
 * @brief   Lemmatizer class.                                                
 */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// (C) Copyright 2014, University of Helsinki                                //
// Licensed under the Apache License, Version 2.0 (the "License");           //
// you may not use this file except in compliance with the License.          //
// You may obtain a copy of the License at                                   //
// http://www.apache.org/licenses/LICENSE-2.0                                //
// Unless required by applicable law or agreed to in writing, software       //
// distributed under the License is distributed on an "AS IS" BASIS,         //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
// See the License for the specific language governing permissions and       //
// limitations under the License.                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "LemmaExtractor.hh"

std::string uppercase(const std::string &word);

std::string init_uppercase(const std::string &word);

typedef std::pair<std::string, std::string> StringPair;

StringPair get_minimal_suffix_edit(const std::string &word1, 
				   const std::string &word2);

bool has_digit(const std::string &word);

bool has_upper(const std::string &word);

#ifndef TEST_LemmaExtractor_cc

#include <ctype.h>
#include <cstring>
#include <cfloat>
//#include <unordered_set>
#include "UnorderedMapSet.hh"
#include <locale>
#include <cassert>

#include "PerceptronTrainer.hh"
#include "Word.hh"

#define PADDING "^^^^^^^^^^" 

std::locale loc;

// Required because each param_table needs a LabelExtractor.
const LabelExtractor dummy_extractor;

LemmaExtractor::LemmaExtractor(void):
  print_stuff(0),
  class_count(1),
  max_passes(50)
{}

LemmaExtractor::~LemmaExtractor(void)
{}

bool LemmaExtractor::is_known_wf(const std::string &word_form) const
{
  return word_form_dict.count(word_form) > 0;
}

void LemmaExtractor::set_max_passes(size_t max_passes)
{ this->max_passes = max_passes; }

void LemmaExtractor::train(const Data &train_data, 
			   const Data &dev_data, 
			   const LabelExtractor &le,
			   std::ostream &msg_out,
			   const TaggerOptions &options)
{
  extract_classes(train_data, le);
  PerceptronTrainer trainer(max_passes, 3, param_table, -1, *this, msg_out, options);
  trainer.train_lemmatizer(train_data, dev_data, *this, le);

  param_table.set_label_extractor(dummy_extractor);
}

std::string lowercase(const std::string &word)
{
  std::string lc_word = word;

  for (unsigned int i = 0; i < lc_word.size(); ++i)
    {
      lc_word[i] = tolower(lc_word[i]);
    }

  size_t pos = 0;

  while ((pos = lc_word.find("Å")) != std::string::npos)
    {
      lc_word.replace(pos, strlen("Å"), "å");
    }

  pos = 0;

  while ((pos = lc_word.find("Ä")) != std::string::npos)
    {
      lc_word.replace(pos, strlen("Ä"), "ä");
    }

  pos = 0;

  while ((pos = lc_word.find("Ö")) != std::string::npos)
    {
      lc_word.replace(pos, strlen("Ö"), "ö");
    }

  return lc_word;
}

std::string uppercase(const std::string &word)
{
  std::string uc_word = word;

  for (unsigned int i = 0; i < uc_word.size(); ++i)
    {
      uc_word[i] = toupper(uc_word[i]);
    }

  size_t pos = 0;

  while ((pos = uc_word.find("å")) != std::string::npos)
    {
      uc_word.replace(pos, strlen("å"), "Å");
    }

  pos = 0;

  while ((pos = uc_word.find("ä")) != std::string::npos)
    {
      uc_word.replace(pos, strlen("ä"), "Ä");
    }

  pos = 0;

  while ((pos = uc_word.find("ö")) != std::string::npos)
    {
      uc_word.replace(pos, strlen("ö"), "Ö");
    }

  return uc_word;
}

std::string init_uppercase(const std::string &word)
{
  std::string uc_word = word;

  if (uc_word.find("å") == 0)
    { 
      uc_word.replace(0, 2, "Å");
    }
  else if (uc_word.find("ä") == 0)
    { 
      uc_word.replace(0, 2, "Ä");
    }
  else if (uc_word.find("ö") == 0)
    { 
      uc_word.replace(0, 2, "Ö");
    }
  else
    {
      uc_word[0] = toupper(uc_word[0]);
    }

  return uc_word;
}

std::string suffix(const std::string word, size_t start)
{
  if (start >= word.size())
    { return ""; }

  return word.substr(start);
}

StringPair get_minimal_suffix_edit(const std::string &word1,
				   const std::string &word2)
{
  unsigned int start = 0;

  for (unsigned int i = 0; i < word1.size(); ++i)
    {
      if (i >= word2.size())
	{ break; }
      
      if (word1[i] != word2[i])
	{ break; }

      ++start;
    }

  //  if (start > 0)
  //    { --start; }

  return StringPair(suffix(word1, start),
		    suffix(word2, start));
}

unsigned int LemmaExtractor::get_class_number(const std::string &word, 
					      const std::string &lemma)
{
  std::string lc_word = lowercase(word);
  std::string lc_lemma = lowercase(lemma);

  StringPair suffixes = get_minimal_suffix_edit(lc_word, lc_lemma);

  if (suffix_map.count(suffixes.first) == 0)
    { static_cast<void>(suffix_map[suffixes.first]); }

  const std::string &wf_suffix = suffixes.first;
  const std::string &lemma_suffix = suffixes.second;

  if (suffix_map[wf_suffix].count(lemma_suffix) == 0)
    { 
      suffix_map[wf_suffix][lemma_suffix] = class_count;
      id_map[class_count] = StringPair(wf_suffix, lemma_suffix);
      ++class_count;
    }

  return suffix_map[wf_suffix][lemma_suffix];
}

void LemmaExtractor::store(std::ostream &out) const
{
  param_table.store(out);
  write_val(out, class_count);
  write_map(out, lemma_lexicon);
  write_map<std::string, std::string, unsigned int>
    (out, suffix_map);
  write_map(out, id_map);
  write_map(out, feat_dict);
  write_map(out, word_form_dict);
  write_val(out, max_passes);
}

void LemmaExtractor::load(std::istream &in, bool reverse_bytes)
{
  param_table.load(in, reverse_bytes);
  read_val<unsigned int>(in, class_count, reverse_bytes);
  read_map(in, lemma_lexicon, reverse_bytes);
  read_map<std::string, std::string, unsigned int>
    (in, suffix_map, reverse_bytes);
  read_map(in, id_map, reverse_bytes);
  read_map(in, feat_dict, reverse_bytes);
  read_map(in, word_form_dict, reverse_bytes);
  read_val<size_t>(in, max_passes, reverse_bytes);

  param_table.set_label_extractor(dummy_extractor);
}

bool LemmaExtractor::operator==(const LemmaExtractor &another) const
{
  if (this == &another)
    { return 1; }

  return 
    (param_table == another.param_table       and
     class_count == another.class_count       and
     lemma_lexicon == another.lemma_lexicon   and
     suffix_map == another.suffix_map         and
     id_map == another.id_map                 and
     feat_dict == another.feat_dict           and
     word_form_dict == another.word_form_dict and
     max_passes == another.max_passes);
}

unsigned int LemmaExtractor::get_class_number(const std::string &word, 
					      const std::string &lemma) const
{
  std::string lc_word = lowercase(word);
  std::string lc_lemma = lowercase(lemma);

  StringPair suffixes = get_minimal_suffix_edit(lc_word, lc_lemma);

  const std::string wf_suffix = suffixes.first;
  const std::string lemma_suffix = suffixes.second;

  if (suffix_map.count(wf_suffix) == 0)
    { throw UnknownSuffixPair(); }
  
  if (suffix_map.find(wf_suffix)->second.count(lemma_suffix) == 0)
    { throw UnknownSuffixPair(); }

  return suffix_map.find(wf_suffix)->second.find(lemma_suffix)->second;
}

void LemmaExtractor::extract_classes(const Data &data,
				     const LabelExtractor &e)
{
  int max_class = 0;
  for (unsigned int i = 0; i < data.size(); ++i)
    {
      for (unsigned int j = 0; j < data.at(i).size(); ++j)
	{
	  const Word &word = data.at(i).at(j);

	  int klass = get_class_number(word.get_word_form(),
				       word.get_lemma());
	  
	  max_class = (max_class < klass ? klass : max_class);
	  
	  lemma_lexicon[word.get_word_form() + "<W+LA>" + 
			e.get_label_string(word.get_label())] = 
	    word.get_lemma();

	  word_form_dict[word.get_word_form()] = 1;
	}
    }
  std::cerr << "Extracted " <<  max_class + 1 
	    << " edit scripts." << std::endl;
}

void LemmaExtractor::set_class_candidates(const std::string &word,
					  LabelVector &class_vector) const
{
  std::unordered_set<unsigned int> potential_classes;
  
  std::string lc_word = lowercase(word);

  for (unsigned int i = 1; i < lc_word.size() + 1; ++i)
    {
      std::string substr = lc_word.substr(i);

      if (suffix_map.count(substr) != 0)
	{
	  const ClassIDMap &clm = suffix_map.find(substr)->second;

	  for (ClassIDMap::const_iterator it = clm.begin(); 
	       it != clm.end(); 
	       ++it)
	    { 	      
	      potential_classes.insert(it->second);
	    }
	}
    }

  class_vector.assign(potential_classes.begin(), potential_classes.end());
}

bool has_upper(const std::string &word)
{
  if (word.find("Ä") != std::string::npos or
      word.find("Å") != std::string::npos or
      word.find("Ö") != std::string::npos)
    { return 1; }

  const char * c_word = word.c_str();
  
  const char * upper_pt = std::use_facet< std::ctype<char> >(loc).scan_is 
    (std::ctype<char>::upper, c_word, c_word + word.size() + 1);

  return upper_pt - c_word < static_cast<long>(word.size());
}

bool has_digit(const std::string &word)
{
  const char * c_word = word.c_str();
  
  const char * digit_pt = std::use_facet< std::ctype<char> >(loc).scan_is 
    (std::ctype<char>::digit, c_word, c_word + word.size() + 1);

  return digit_pt - c_word < static_cast<long>(word.size());
}

std::string get_main_label(const std::string &label)
{
  if (label.find('|') == std::string::npos)
    { return label; }
  else
    { return label.substr(0, label.find('|')); }
}

std::string get_feats(const std::string &label)
{
  if (label.find('|') == std::string::npos)
    { return label; }
  else
    { return label.substr(label.find('|')); }
}

Word * LemmaExtractor::extract_feats(const std::string &word_form, 
				     const std::string &label,
				     bool use_label)
{
  std::string word = lowercase(word_form);

  FeatureTemplateVector feats;
  
  feats.push_back( get_feat_id("WORD=" + /*word_form*/word) );

  std::string padded_word_form = PADDING + /*word_form*/word;

  std::string main_label = get_main_label(label);

  for (unsigned int i = padded_word_form.size() - 7; 
       i <= padded_word_form.size();
       ++i)
    {
      feats.push_back( get_feat_id("SUFFIX=" + padded_word_form.substr(i)) );
      feats.push_back( get_feat_id("SUFFIX=" + padded_word_form.substr(i) + " LABEL=" + label) );
      //      feats.push_back( get_feat_id("SUFFIX=" + padded_word_form.substr(i) + " MAIN_LABEL=" + main_label) );
    }

  //  padded_word_form = /*word_form*/word + PADDING;
  for (unsigned int i = 1;
       i <= 5;
       ++i)
    {
      if (i > padded_word_form.size() - 1)
	{ break; }

      feats.push_back( get_feat_id("PREFIX=" + padded_word_form.substr(0,i)));
      feats.push_back( get_feat_id("PREFIX=" + padded_word_form.substr(0,i) + " LABEL=" + label) );
      //      feats.push_back( get_feat_id("PREFIX=" + padded_word_form.substr(0,i) + " MAIN_LABEL=" + main_label) );
    }
  
  feats.push_back(get_feat_id("INFIX4=" + padded_word_form.substr(0, padded_word_form.size() - 2).substr(padded_word_form.size() - 4)));
  feats.push_back(get_feat_id("INFIX4=" + padded_word_form.substr(0, padded_word_form.size() - 2).substr(padded_word_form.size() - 4) 
			      + " LABEL=" + label));

  feats.push_back(get_feat_id("INFIX5=" + padded_word_form.substr(0, padded_word_form.size() - 3).substr(padded_word_form.size() - 5)));
  feats.push_back(get_feat_id("INFIX5=" + padded_word_form.substr(0, padded_word_form.size() - 3).substr(padded_word_form.size() - 5) 
			      + " LABEL=" + label));

  feats.push_back(get_feat_id("INFIX6=" + padded_word_form.substr(0, padded_word_form.size() - 4).substr(padded_word_form.size() - 6)));
  feats.push_back(get_feat_id("INFIX6=" + padded_word_form.substr(0, padded_word_form.size() - 4).substr(padded_word_form.size() - 6)
			      + " LABEL=" + label));

  if (use_label)
    {
      feats.push_back( get_feat_id("LABEL=" + label) );  
      feats.push_back( get_feat_id("MFEATS=" + get_feats(label)) );  
      //feats.push_back( get_feat_id("MAIN_LABEL=" + get_main_label(label)) );
    }

  if (has_upper(word_form))
    { feats.push_back( get_feat_id("UC") ); }

  if (has_digit(word_form))
    { feats.push_back( get_feat_id("DIGIT") ); }

  return new Word(word_form, feats, LabelVector(), "");
  static_cast<void>(label);
}

unsigned int LemmaExtractor::get_feat_id(const std::string &feat_string)
{
  if (feat_dict.count(feat_string) == 0)
    { 
      unsigned int id = feat_dict.size();
      feat_dict[feat_string] = id; 
    }

  return feat_dict[feat_string];
}

std::string LemmaExtractor::get_lemma(const std::string &word_form, 
				      unsigned int klass) const
{
  std::string lc_word_form = lowercase(word_form);
  
  if (id_map.count(klass) == 0)
    { 
      throw UnknownClass(); 
    }

  const StringPair &word_and_lemma_suffixes = id_map.find(klass)->second;

  const std::string &word_suffix = word_and_lemma_suffixes.first;
  const std::string &lemma_suffix = word_and_lemma_suffixes.second;
  
  size_t pos = lc_word_form.rfind(word_suffix);

  assert(pos != std::string::npos);
  
  std::string lemma = lc_word_form.substr(0, pos) + lemma_suffix;

  /*
  if (has_upper(word_form))
    { return init_uppercase(lemma); }
  */

  return lemma;
}

unsigned int LemmaExtractor::get_lemma_candidate_class(const Word &w,
						       ParamTable * pt)
{
  if (pt == 0)
    { pt = &param_table; }

  LabelVector lemma_class_candidates;
  set_class_candidates(w.get_word_form(), lemma_class_candidates);

  float max_score = -FLT_MAX;
  unsigned int max_class = -1;

  for (unsigned int i = 0; i < lemma_class_candidates.size(); ++i)
    {
      unsigned int klass = lemma_class_candidates[i];
      
      float score = pt->get_all_unstruct(w, klass, 0);

      if (score > max_score)
	{
	  max_score = score;
	  max_class = klass;
	}
    }
 
  return max_class;
}

unsigned int LemmaExtractor::get_lemma_candidate_class
(const std::string &word_form, 
 const std::string &label)
{
  Word * w = extract_feats(word_form, label);

  unsigned int klass = get_lemma_candidate_class(*w);

  delete w;

  return klass;
}

std::string LemmaExtractor::get_lemma_candidate(const std::string &word_form, 
						const std::string &label)
{
  std::string lexicon_entry = word_form + "<W+LA>" + label;

  if (lemma_lexicon.count(lexicon_entry) != 0)
    { return lemma_lexicon[lexicon_entry]; }

  // FIXME
  lexicon_entry = word_form + "<W>";

  if (lemma_lexicon.count(lexicon_entry) != 0)
    { return lemma_lexicon[lexicon_entry]; }

  return get_lemma(word_form, 
		   get_lemma_candidate_class(word_form, 
					     label)); 
}  

#else // TEST_LemmaExtractor_cc

struct TEST_LemmaExtractor
{
  LemmaExtractor &le;

  TEST_LemmaExtractor(LemmaExtractor &le):
    le(le)
  {}

  unsigned int get_class_number(const std::string &word, 
				const std::string &lemma)
  { return le.get_class_number(word, lemma); }

  void set_class_candidates(const std::string &word,
			    LabelVector &class_vector) const
  { le.set_class_candidates(word, class_vector); }

  std::string get_lemma(const std::string &word_form, 
			unsigned int klass) const
  { return le.get_lemma(word_form, klass); }
};

#include <cassert>
#include <sstream>

#include "Data.hh"

int main(void)
{
  assert(lowercase("koira") == "koira");
  assert(lowercase("KOIRA") == "koira");
  assert(lowercase("Koira") == "koira");
  assert(lowercase("KoirA") == "koira");

  assert(uppercase("koira") == "KOIRA");
  assert(uppercase("KOIRA") == "KOIRA");
  assert(uppercase("Koira") == "KOIRA");
  assert(uppercase("KoirA") == "KOIRA");

  assert(init_uppercase("koira") == "Koira");
  assert(init_uppercase("KOIRA") == "KOIRA");
  assert(init_uppercase("Koira") == "Koira");
  assert(init_uppercase("KoirA") == "KoirA");

  assert(lowercase("äijän") == "äijän");
  assert(lowercase("ÄIJÄN") == "äijän");
  assert(lowercase("Äijän") == "äijän");
  assert(lowercase("Äijän") == "äijän");

  assert(uppercase("äijän") == "ÄIJÄN");
  assert(uppercase("ÄIJÄN") == "ÄIJÄN");
  assert(uppercase("Äijän") == "ÄIJÄN");
  assert(uppercase("Äijän") == "ÄIJÄN");

  assert(init_uppercase("äijän") == "Äijän");
  assert(init_uppercase("ÄIJÄN") == "ÄIJÄN");
  assert(init_uppercase("Äijän") == "Äijän");
  assert(init_uppercase("Äijän") == "Äijän");

  assert(get_minimal_suffix_edit("koira", "koira") == StringPair("",""));
  assert(get_minimal_suffix_edit("koira", "koiran") == StringPair("","n"));
  assert(get_minimal_suffix_edit("äijä", "äijä") == StringPair("",""));
  assert(get_minimal_suffix_edit("äijän", "äijää") == StringPair("n","ä"));
  assert(get_minimal_suffix_edit("laidun", "laitumen") == 
	 StringPair("dun","tumen"));

  LemmaExtractor le;

  TEST_LemmaExtractor tle(le);

  assert(tle.get_class_number("koira", "koira") == 1);
  assert(tle.get_class_number("KOIRA", "KOIRA") == 1);
  assert(tle.get_class_number("Koira", "Koira") == 1);

  assert(tle.get_class_number("isä", "isä") == 1);
  assert(tle.get_class_number("ISÄ", "ISÄ") == 1);
  assert(tle.get_class_number("Isä", "Isä") == 1);

  assert(tle.get_class_number("koiran", "koira") == 2);
  assert(tle.get_class_number("KOIRAN", "KOIRA") == 2);
  assert(tle.get_class_number("Koiran", "Koira") == 2);

  assert(tle.get_class_number("isän", "isä") == 2);
  assert(tle.get_class_number("ISÄN", "ISÄ") == 2);
  assert(tle.get_class_number("Isän", "Isä") == 2);

  assert(tle.get_class_number("isästä", "isä") == 3);
  assert(tle.get_class_number("ISÄSTÄ", "ISÄ") == 3);
  assert(tle.get_class_number("Isästä", "Isä") == 3);
  
  assert(tle.get_lemma("isä", 1) == "isä");
  assert(tle.get_lemma("isän", 1) == "isän");
  assert(tle.get_lemma("isän", 2) == "isä");

  assert(tle.get_lemma("kissa", 1) == "kissa");
  assert(tle.get_lemma("kissan", 1) == "kissan");
  assert(tle.get_lemma("kissan", 2) == "kissa");
  assert(tle.get_lemma("kissastä", 1) == "kissastä");
  assert(tle.get_lemma("kissastä", 3) == "kissa");

  LabelVector labels;
  tle.set_class_candidates("äidin", labels);

  assert(labels.size() == 2);
  assert(labels[0] == 1 or labels[0] == 2);
  assert(labels[1] == 1 or labels[1] == 2);
  assert(labels[0] != labels[1]);

  labels.clear();
  tle.set_class_candidates("äidistä", labels);

  assert(labels.size() == 2);
  assert(labels[0] == 3 or labels[0] == 1);
  assert(labels[1] == 3 or labels[1] == 1);
  assert(labels[0] != labels[1]);

  labels.clear();
  tle.set_class_candidates("ÄIDISTÄ", labels);

  assert(labels.size() == 2);
  assert(labels[0] == 3 or labels[0] == 1);
  assert(labels[1] == 3 or labels[1] == 1);
  assert(labels[0] != labels[1]);

  assert(not has_upper(""));
  assert(not has_upper("koira"));
  assert(has_upper("Koira"));
  assert(not has_upper("äiti"));
  assert(has_upper("Äiti"));
  assert(has_upper("äitI"));
  assert(has_upper("A"));
  assert(has_upper("Å"));
  assert(has_upper("Ä"));
  assert(has_upper("Ö"));

  assert(not has_digit(""));
  assert(not has_digit("koira"));
  assert(not has_digit("äiti"));
  assert(has_digit("2"));
  assert(has_digit("koira1"));
  assert(has_digit("1koira"));
  assert(has_digit("äiti1"));
  assert(has_digit("1äiti"));

  std::string contents("\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dogs\tWORD=dogs\tdog\tNN\t_\n"
		       ".\tWORD=.\t.\t.\t_\n"
		       "\n"
		       "\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dogs\tWORD=dogs\tdog\tNN\t_\n"
		       ".\tWORD=.\t.\t.\t_\n");

  std::istringstream in(contents);

  ParamTable pt;

  LabelExtractor label_extractor(10);

  Data train_data(in, 1, label_extractor, pt, 2);
  Data dev_data(train_data);

  LemmaExtractor lemma_extractor;
  std::ostringstream null_stream;

  lemma_extractor.train(train_data, dev_data, label_extractor, null_stream);

  assert(lemma_extractor.get_lemma_candidate("hogs", "NN") == "hog");

  std::ostringstream lemma_extractor_out;
  lemma_extractor.store(lemma_extractor_out);
  std::istringstream lemma_extractor_in(lemma_extractor_out.str());
  LemmaExtractor lemma_extractor_copy;
  lemma_extractor_copy.load(lemma_extractor_in, false);
  assert(lemma_extractor == lemma_extractor_copy);
}

#endif // TEST_LemmaExtractor_cc
