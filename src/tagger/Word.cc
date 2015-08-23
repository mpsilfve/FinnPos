/**
 * @file    Word.cc                                                          
 * @Author  Miikka Silfverberg                                               
 * @brief   A class that represents one word.                                
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

#include "Word.hh"
#include <cassert>
#include <iostream>

#ifndef TEST_Word_cc

Word::Word(unsigned int boundary_label):
  word_form(BOUNDARY_WF),
  lemma(BOUNDARY_WF),
  label(boundary_label),
  label_candidates(1, boundary_label)
{}

Word::Word(const std::string &word_form,
	   const FeatureTemplateVector &feature_templates,
	   const LabelVector &labels,
	   const std::string &annotations):
  word_form(word_form),
  lemma(NO_STRING),
  annotations(annotations),
  label(NO_LABEL),
  label_candidates(labels),
  feature_templates(feature_templates)
{}

void Word::clear_label_guesses(void)
{
  label_candidates.clear();
}

void Word::set_label_guesses(const LabelExtractor &g, 
			     bool use_label_dict,
			     float mass)
{ 
  g.set_label_candidates(word_form, 
			 use_label_dict, 
			 mass, 
			 label_candidates); 
}

void Word::predict_lemma(LemmaExtractor &g, const LabelExtractor &e)
{ 
  if (label == static_cast<unsigned int>(NO_LABEL))
    { throw NoLabel(); }

  lemma = g.get_lemma_candidate(word_form, e.get_label_string(label)); 
}

void Word::set_lemma(const std::string &lemma)
{ this->lemma = lemma; }

void Word::set_label(unsigned int label)
{ this->label = label; }

unsigned int Word::get_feature_template_count(void) const
{ return feature_templates.size(); }

unsigned int Word::get_feature_template(unsigned int feature_template_index) 
  const
{ return feature_templates.at(feature_template_index); }

unsigned int Word::get_label_count(void) const
{ return label_candidates.size(); }

unsigned int Word::get_label(unsigned int label_index) const
{ return label_candidates.at(label_index); }

unsigned int Word::get_label(void) const
{ return label; }

std::string Word::get_lemma(void) const
{ return lemma; }

std::string Word::get_word_form(void) const
{ return word_form; }

int Word::get_recall(void)
{
  for (unsigned int i = 0; i < label_candidates.size(); ++i)
    { 
      if (label == label_candidates[i])
	{ return 1; }
    }
  
  return 0;
}

void Word::unset_lemma(void)
{
  lemma = NO_STRING;
}

void Word::unset_label(void)
{
  label = NO_LABEL;
}

std::string Word::get_annotations(void) const
{ return annotations; }

std::string Word::to_string(LabelExtractor &label_extractor) const
{
  return 
    get_word_form() + '\t' +
    '_' + '\t' +
    get_lemma() + '\t' +
    label_extractor.get_label_string(get_label()) + '\t' +
    get_annotations();
}

void Word::parse_aux_data(StringPairVector &p) const
{
  try 
    {
      finnposaux::parse_aux_field(annotations, p);
    }
  catch (...)
    {}
}
  
void Word::set_analyzer_lemmas(LabelExtractor &le)
{
  StringPairVector sp;
  parse_aux_data(sp);

  for (size_t i = 0; i < sp.size(); ++i)
    {
      analyzer_lemmas.push_back
	(LabelLemmaPair(le.get_label(sp[i].first), sp[i].second));
    }
}

#else // TEST_Word_cc

#include <cassert>
#include <iostream>

class SillyLabelExtractor : public LabelExtractor
{
public:
  SillyLabelExtractor(void):
    LabelExtractor(1)
  {}

  void set_label_candidates(const std::string &word_form, 
			    bool use_lexicon,
			    float count,
			    LabelVector &target) const
  {
    static_cast<void>(word_form);
    static_cast<void>(use_lexicon);

    int prev_size = target.size();

    for (unsigned int i = 0; i < count - prev_size; ++i)
      { 
	target.push_back(0); 
      }
  }
};

class SillyLemmaExtractor : public LemmaExtractor
{
public:
  std::string get_lemma_candidate(const std::string &word_form, 
				  const std::string &label)
  { 
    static_cast<void>(word_form);
    static_cast<void>(label);

    return "FOO"; 
  }
  
  bool is_known_wf(const std::string &word_form) const
  {
    static_cast<void>(word_form);

    return 1;
  }
};

int main(void)
{
  // Test boundary word.
  Word b_word(10);

  assert(b_word.get_label() == 10);
  assert(b_word.get_label_count() == 1);
  assert(b_word.get_label(0) == 10);
  assert(b_word.get_feature_template_count() == 0);

  FeatureTemplateVector feature_templates;
  feature_templates.push_back(0);
  feature_templates.push_back(1);

  LabelVector labels;
  labels.push_back(0);
  labels.push_back(1);

  Word word("dog",
	    feature_templates,
	    labels,
	    "foo");

  assert(word.get_feature_template_count() == 2);
  assert(word.get_feature_template(0) == 0);
  assert(word.get_feature_template(1) == 1);

  SillyLabelExtractor la_getter;
  static_cast<void>(la_getter.get_label("FOO"));

  SillyLemmaExtractor le_getter;

  word.set_label_guesses(la_getter, 0, 5);
  assert(word.get_label_count() == 5);
  assert(word.get_label(0) == 0);
  assert(word.get_label(1) == 1);
  assert(word.get_label(2) == 0);
  assert(word.get_label(3) == 0);
  assert(word.get_label(4) == 0);

  try
    {
      word.predict_lemma(le_getter, la_getter);
      assert(0);
    }
  catch (const NoLabel &e)
    {
      /* EXPECTED FAIL */
    }

  word.set_label(1);
  assert(word.get_label() == 1);

  try
    {
      word.predict_lemma(le_getter, la_getter);
    }
  catch (const NoLabel &e)
    {
      assert(0);
    }

  assert(word.get_lemma() == "FOO");
}

#endif // TEST_Word_cc
