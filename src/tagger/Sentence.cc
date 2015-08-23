/**
 * @file    Sentence.cc                                                      
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

#include "Sentence.hh"

#ifndef TEST_Sentence_cc

#include <algorithm>

#include "io.hh"

Sentence::Sentence(const WordVector &words, 
		   LabelExtractor &label_extractor, 
		   unsigned int degree)
{
  Word bw(label_extractor.get_boundary_label());
  static_cast<void>(degree);
  // Add boundary symbols and words to internal container.
  sentence.insert(sentence.end(), 1, bw);
  sentence.insert(sentence.end(), words.begin(), words.end());
  sentence.insert(sentence.end(), /*degree*/2, bw);
}

Sentence::Sentence(std::istream &ifile, 
		   bool is_gold, 
		   LabelExtractor &label_extractor, 
		   ParamTable &pt, 
		   unsigned int degree, 
		   unsigned int &line_counter)
{
  static_cast<void>(degree);
  // Add boundary symbols.
  Word bw(label_extractor.get_boundary_label());
  //  std::cerr << degree << std::endl;
  sentence.insert(sentence.end(), /*degree*/ 2, bw);

  while (ifile.peek() != EOF)
    {
      ++line_counter;

      try
	{
	  Entry entry = get_next_line(ifile);

	  if (entry.token.empty() or entry.feat_templates.empty())
	    { 
	      throw SyntaxError(); 
	    }

	  sentence.push_back(Word(entry.token,
				  pt.get_feat_templates(entry.feat_templates),
				  label_extractor.get_labels(entry.labels),
				  entry.annotations));
	  sentence.back().set_analyzer_lemmas(label_extractor);

	  if (is_gold)
	    {
	      if (entry.lemma.empty() or /*entry.labels.size() != 1*/ entry.labels.size() == 0)
		{ 
		  throw SyntaxError(); 
		}

	      unsigned int label = label_extractor.get_label(entry.labels[0]);

	      sentence.back().set_lemma(entry.lemma);
	      sentence.back().set_label(label);	     
	    }
	}
      catch (EmptyLine &e)
	{ 
	  break; 
	}
    }

  if (sentence.size() == /*degree*/2)
    { 
      sentence.clear(); 
    }
  else
    {
      sentence.insert(sentence.end(), /*degree*/2, bw);
    }
}

const Word &Sentence::at(unsigned int i) const
{ 
  return sentence.at(i); 
}

Word &Sentence::at(unsigned int i)
{ 
  return sentence.at(i); 
}

unsigned int Sentence::size(void) const
{ 
  return sentence.size(); 
}

unsigned int Sentence::get_max_label_count(void) const
{
  unsigned int max_count = 0;

  for (unsigned int i = 0; i < sentence.size(); ++i)
    { 
      max_count = std::max(sentence[i].get_label_count(), max_count); 
    }

  return max_count;
}

void Sentence::clear_label_guesses(void)
{
  for (unsigned int i = 0; i < sentence.size(); ++i)
    { sentence[i].clear_label_guesses(); }
}

void Sentence::set_label_guesses(const LabelExtractor &g, bool use_label_dict, float mass)
{
  for (unsigned int i = 0; i < sentence.size(); ++i)
    { 
      sentence[i].set_label_guesses(g, use_label_dict, mass); 
    }
}

void Sentence::predict_lemma(LemmaExtractor &g, const LabelExtractor &e)
{
  for (unsigned int i = 0; i < sentence.size(); ++i)
    { 
      sentence[i].predict_lemma(g, e); 
    }
}

void Sentence::unset_lemma(void)
{
  for (unsigned int i = 0; i < sentence.size(); ++i)
    { 
      sentence[i].unset_lemma(); 
    }
}

void Sentence::unset_label(void)
{
  for (unsigned int i = 0; i < sentence.size(); ++i)
    { 
      sentence[i].unset_label(); 
    }
}

void Sentence::print(std::ostream &out, LabelExtractor &label_extractor)
{
  for (unsigned int i = 0; i < size() - 0; ++i)
    { 
      if (at(i).get_word_form() == BOUNDARY_WF)
	{ continue; }

      out << at(i).to_string(label_extractor);
      
      if (i + 1 < size() and at(i + 1).get_word_form() != BOUNDARY_WF)
	{ out << std::endl; }
    }
}

void Sentence::parse_aux_data(StringPairVector &p) const
{
  for (unsigned int i = 0; i < sentence.size(); ++i)
    { 
      sentence[i].parse_aux_data(p);
    }
}

#else // TEST_Sentence_cc

#include <cassert>
#include <cstdlib>
#include <sstream>
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
  SillyLabelExtractor label_extractor;
  SillyLemmaExtractor lemma_extractor;

  ParamTable pt;

  std::string empty;

  std::istringstream empty_in(empty);
  
  unsigned int dummy_counter = 0;

  Sentence empty_s(empty_in, 0, label_extractor, pt, 2, dummy_counter);
 
  assert(empty_s.size() == 0);

  std::string sentence("The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN VB\t_\n"
		       ".\tWORD=.\t.\t.\t_");

  unsigned int line_counter = 0;

  std::istringstream in(sentence);

  static_cast<void>(in);

  Sentence s(in, 0, label_extractor, pt, 2, line_counter);
  
  assert(line_counter == 3);
  assert(s.size() == 3 + 2*2);
  assert(s.get_max_label_count() == 2);

  s.set_label_guesses(label_extractor, 0, 5);
  assert(s.get_max_label_count() == 5);

  const Word &dog = s.at(3);

  assert(dog.get_word_form() == "dog");
  assert(dog.get_label_count() == 5);
  assert(dog.get_label(0) == label_extractor.get_label("NN"));
  assert(dog.get_label(1) == label_extractor.get_label("VB"));
  assert(dog.get_label(2) == 0);
  assert(dog.get_label(3) == 0);
  assert(dog.get_label(4) == 0);
  
  for (unsigned int i = 0; i < 3 + 4; ++i)
    { s.at(i).set_label(0); }

  s.predict_lemma(lemma_extractor, label_extractor);

  //  assert(dog.get_lemma() == "FOO");
}

#endif // TEST_Sentence_cc
