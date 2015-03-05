/**
 * @file    Data.hh                                                          
 * @Author  Miikka Silfverberg                                               
 * @brief   Container for Sentence-objects.                                  
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

#include "Data.hh"

#ifndef TEST_Data_cc

#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>

#include "io.hh"

bool Data::silent = 0;

Data::Data(void)
{}

Data::Data(const std::string &fn, 
	   bool is_gold,
	   LabelExtractor &extractor, 
	   ParamTable &pt, 
	   unsigned int degree)
{
  std::ifstream ifile(fn.c_str());

  init(fn, ifile, is_gold, extractor, pt, degree); 
}

Data::Data(std::istream &in, 
	   bool is_gold, 
	   LabelExtractor &extractor, 
	   ParamTable &pt, 
	   unsigned int degree)
{
  init("<stdin>", in, is_gold, extractor, pt, degree); 
}

void Data::init(const std::string &fn, 
		std::istream &in, 
		bool is_gold, 
		LabelExtractor &extractor, 
		ParamTable &pt, 
		unsigned int degree)
{
  unsigned int line = 0;

  while (in)
    {
      /*      try
	      {*/
	  Sentence s(in, is_gold, extractor, pt, degree, line);

	  if (s.size() > 0)
	    {
	      data.push_back(s);
	    }
	  /*	}
      catch (SyntaxError &e)
	{ 
	  if (not silent)
	    {
	      std::cerr << "Syntax error on line " 
			<< line 
			<< " in file " 
			<< fn 
			<< "." 
			<< std::endl;
	    }
	  
	    throw e;
	    }*/
	  static_cast<void>(fn);
    }
}

const Sentence &Data::at(unsigned int i) const
{ return data.at(i); }

Sentence &Data::at(unsigned int i)
{ return data.at(i); }

unsigned int Data::size(void) const
{ return data.size(); }

void Data::set_label_guesses(const LabelExtractor &g, 
			     bool use_label_dict,
			     float mass)
{
  for (unsigned int i = 0; i < data.size(); ++i)
    {
      data[i].set_label_guesses(g, use_label_dict, mass);
    }
}

void Data::clear_label_guesses(void)
{
  for (unsigned int i = 0; i < data.size(); ++i)
    { data[i].clear_label_guesses(); }
}

void Data::predict_lemma(LemmaExtractor &g, const LabelExtractor &e)
{
  for (unsigned int i = 0; i < data.size(); ++i)
    {
      data[i].predict_lemma(g, e);
    }
}

void Data::unset_lemma(void)
{
  for (unsigned int i = 0; i < data.size(); ++i)
    {
      data[i].unset_lemma();
    }
}

void Data::unset_label(void)
{
  for (unsigned int i = 0; i < data.size(); ++i)
    {
      data[i].unset_label();
    }
}

Acc Data::get_acc(const Data &other, 
		  const LemmaExtractor &lemma_extractor) const
{
  if (data.size() != other.data.size())
    {
      throw IncompatibleData();
    }

  float label_match = 0;
  float iv_label_match = 0;
  float oov_label_match = 0;

  float lemma_match = 0;
  float iv_lemma_match = 0;
  float oov_lemma_match = 0;

  float words = 0;
  float iv_words = 0;
  float oov_words = 0;

  for (unsigned int i = 0; i < data.size(); ++i)
    {
      if (data[i].size() != other.data[i].size())
	{
	  throw IncompatibleData();
	}
      
      for (unsigned int j = 0; j < data.at(i).size(); ++j)
	{
	  const Word &word1 = data[i].at(j);
	  const Word &word2 = other.data[i].at(j);

	  if (word1.get_word_form() != word2.get_word_form())
	    {
	      throw IncompatibleData();
	    }
	  
	  if (word1.get_word_form() == BOUNDARY_WF)
	    { continue; }

	  label_match += (word1.get_label() == word2.get_label());
	  lemma_match += (word1.get_lemma() == word2.get_lemma());
	  
	  ++words;

	  if (lemma_extractor.is_known_wf(word1.get_word_form()))
	    { 
	      iv_label_match += (word1.get_label() == word2.get_label());
	      iv_lemma_match += (word1.get_lemma() == word2.get_lemma());
	      
	      ++iv_words;
	    }
	  else
	    {
	      oov_label_match += (word1.get_label() == word2.get_label());
	      oov_lemma_match += (word1.get_lemma() == word2.get_lemma());
	      
	      ++oov_words;
	    }
	}
    }

  Acc acc;

  acc.label_acc = (words == 0 ? 0.0 : label_match / words);
  acc.iv_label_acc = (iv_words == 0 ? 0.0 : iv_label_match / iv_words);
  acc.oov_label_acc = (oov_words == 0 ? 0.0 : oov_label_match / oov_words);

  acc.lemma_acc = (words == 0 ? 0.0 : lemma_match / words);
  acc.iv_lemma_acc = (iv_words == 0 ? 0.0 : iv_lemma_match / iv_words);
  acc.oov_lemma_acc = (oov_words == 0 ? 0.0 : oov_lemma_match / oov_words);

  return acc;
}

void Data::print(std::ostream &out, LabelExtractor &label_extractor)
{
  for (unsigned int i = 0; i < size(); ++i)
    {
      at(i).print(out, label_extractor);
      out << std::endl;

      if (i + 1 < data.size())
	{ out << std::endl; }
    }
}

void Data::randomize(void)
{
  // Use fixed seed, in order to assure the replicability of experiments.q
  std::srand(0);
  std::random_shuffle(data.begin(), data.end());
}

#else // TEST_Data_cc

#include <sstream>
#include <cassert>

class SillyLabelExtractor : public LabelExtractor
{
public:
  SillyLabelExtractor(void):
    LabelExtractor(1)
  {}

  void set_label_candidates(const std::string &word_form, 
			    bool use_lexicon,
			    float mass, 
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
  std::string contents("\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN VB\t_\n"
		       ".\tWORD=.\t.\t.\t_\n"
		       "\n"
		       "\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN VB\t_\n"
		       ".\tWORD=.\t.\t.\t_\n");

  std::istringstream in(contents);

  SillyLabelExtractor label_extractor;

  ParamTable pt;

  Data data(in, 0, label_extractor, pt, 2);

  assert(data.size() == 2);
  assert(data.at(0).size() == 3 + 4);
  assert(data.at(1).size() == 3 + 4);
}

#endif // TEST_Data_cc
