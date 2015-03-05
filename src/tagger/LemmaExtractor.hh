/**
 * @file    LemmaExtractor.hh                                                
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

#ifndef HEADER_LemmaExtractor_hh
#define HEADER_LemmaExtractor_hh

#include <vector>
//#include <unordered_map>
#include "UnorderedMapSet.hh"

#include <string>
#include <utility>
#include <iostream>

#include "LabelExtractor.hh"
#include "ParamTable.hh"
#include "exceptions.hh"

typedef std::vector<unsigned int> LemmaVector;

class Word;
class Data;
class PerceptronTrainer;

std::string lowercase(const std::string &word);

class LemmaExtractor
{
 public:
  LemmaExtractor(void);

  ~LemmaExtractor(void);

  void train(const Data &train_data, 
	     const Data &dev_data, 
	     const LabelExtractor &le,
	     std::ostream &msg_out);

  virtual std::string get_lemma_candidate(const std::string &word_form, 
					  const std::string &label);

  virtual bool is_known_wf(const std::string &word_form) const;

  void set_max_passes(size_t max_passes);

  bool print_stuff;

  void store(std::ostream &out) const;
  void load(std::istream &in, bool reverse_bytes);
  bool operator==(const LemmaExtractor &another) const;
protected:
  typedef std::pair<std::string, std::string> StringPair;
  typedef std::unordered_map<std::string, unsigned int> ClassIDMap;
  typedef std::unordered_map<unsigned int, StringPair> IDClassMap;

  typedef std::unordered_map<std::string, std::string>
  LemmaLexicon;
  
  typedef std::unordered_map<std::string, ClassIDMap> SuffixMap;

  ParamTable param_table;
  unsigned int class_count;
  LemmaLexicon lemma_lexicon;  
  SuffixMap suffix_map;
  IDClassMap id_map;
  ClassIDMap feat_dict;
  ClassIDMap word_form_dict;
  size_t max_passes;

  void extract_classes(const Data &data, const LabelExtractor &le);

  unsigned int get_feat_id(const std::string &feat_string);

  unsigned int get_class_number(const std::string &word, 
				const std::string &lemma);

  unsigned int get_class_number(const std::string &word, 
				const std::string &lemma) const;

  void set_class_candidates(const std::string &word,
			    LabelVector &class_vector) const;

  Word *  extract_feats(const std::string &word_form, 
			const std::string &label);
  
  unsigned int get_lemma_candidate_class(const std::string &word_form, 
					 const std::string &label);

  unsigned int get_lemma_candidate_class(const Word &w, ParamTable * pt = 0);


  std::string get_lemma(const std::string &word_form, 
			unsigned int klass) const;
  
  friend struct TEST_LemmaExtractor;
  friend class PerceptronTrainer;
};

#endif // HEADER_LemmaExtractor_hh
