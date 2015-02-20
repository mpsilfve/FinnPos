/**
 * @file    Sentence.hh                                                      
 * @Author  Miikka Silfverberg                                               
 * @brief   Container for Word objects.                                      
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

#ifndef HEADER_Sentence_hh
#define HEADER_Sentence_hh

#include <vector>
#include <fstream>
#include <iostream>

#include "Word.hh"
#include "ParamTable.hh"

typedef std::vector<Word> WordVector;

class Sentence
{
 public:

  Sentence(const WordVector &words, 
	   LabelExtractor &label_extractor, 
	   unsigned int degree);

  Sentence(std::istream &ifile, 
	   bool is_gold, 
	   LabelExtractor &label_extractor, 
	   ParamTable &pt, 
	   unsigned int degree, 
	   unsigned int &line_counter);

  const Word &at(unsigned int i) const;
  Word &at(unsigned int i);
  unsigned int size(void) const;
  unsigned int get_max_label_count(void) const;
  void clear_label_guesses(void);
  void set_label_guesses(const LabelExtractor &g, 
			 bool use_label_dict,
			 float mass);
  void predict_lemma(LemmaExtractor &g, const LabelExtractor &e);

  void unset_lemma(void);
  void unset_label(void);

  void print(std::ostream &out, LabelExtractor &label_extractor);

 private:
  WordVector sentence;
};

#endif // HEADER_Sentence_hh
