/**
 *  @file    Word.hh                                                       
 *  @Author  Miikka Silfverberg                                            
 *  @brief   A class that represents one word.                               
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

#ifndef HEADER_Word_hh
#define HEADER_Word_hh

#include <vector>
#include <string>

#include "LabelExtractor.hh"
#include "LemmaExtractor.hh"
#include "ParamTable.hh"
#include "exceptions.hh"

#define BOUNDARY_WF "_#_"

#define NO_STRING ""
#define NO_LABEL -1

class Word
{
public:
  // Initialize to boundary words
  Word(unsigned int boundary_label);
  
  // Initialize using a label list.
  Word(const std::string &word_form,
       const FeatureTemplateVector &feature_templates,
       const LabelVector &labels,
       const std::string &annotations);

  int get_recall(void);

  void set_label_guesses(const LabelExtractor &g, 
			 bool use_label_dict,
			 float mass);
  
  void clear_label_guesses(void);

  void predict_lemma(LemmaExtractor &g, const LabelExtractor &e);

  void set_lemma(const std::string &lemma);
  void set_label(unsigned int label);

  unsigned int get_feature_template_count(void) const;
  unsigned int get_feature_template(unsigned int feature_template_index) const;

  unsigned int get_label_count(void) const;
  unsigned int get_label(unsigned int label_index) const;

  unsigned int get_label(void) const;

  std::string get_lemma(void) const;

  std::string get_word_form(void) const;

  std::string get_annotations(void) const;

  void unset_lemma(void);
  void unset_label(void);

  std::string to_string(LabelExtractor &label_extractor) const;

 private:
  std::string           word_form;
  std::string           lemma;
  std::string           annotations;

  unsigned int          label;
  
  LabelVector           label_candidates;
  FeatureTemplateVector feature_templates;
};

#endif // HEADER_Word_hh
