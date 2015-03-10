/**
 * @file    LabelExtractor.cc                                                
 * @Author  Miikka Silfverberg                                               
 * @brief   Suffix-based label guesser                                       
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

#include "LabelExtractor.hh"

std::string get_suffix(const std::string &wf, unsigned int length);

#ifndef TEST_LabelExtractor_cc

#include <algorithm>
//#include <unordered_set>
#include "UnorderedMapSet.hh"
#include "Data.hh"

#define PADDING "^^^^^^^^^^" 
#define PADDING_LEN 10 

LabelExtractor::LabelExtractor(unsigned int max_suffix_len):
  max_suffix_len(max_suffix_len)
{
  static_cast<void>(get_label("_#_"));

  for (unsigned int i = 0; i < max_suffix_len + 1; ++i)
    { 
      label_counts.push_back(i); 
    }
}

LabelExtractor::~LabelExtractor(void)
{}

unsigned int LabelExtractor::get_boundary_label(void) const
{ return 0; }

#include <cassert>

unsigned int LabelExtractor::get_label(const std::string &label_string)
{
  if (label_map.find(label_string) == label_map.end())
    {
      unsigned int id = label_map.size();
      label_map[label_string] = id;
      string_map.push_back(label_string);

      if (label_string.find('|') != std::string::npos)
	{
	  StringVector sub_label_strings;
	  split(label_string, sub_label_strings, '|');
	  LabelVector &sub_labels = sub_label_map[id];

	  assert(sub_labels.empty());

	  for (unsigned int i = 0; i < sub_label_strings.size(); ++i)
	    {
	      sub_labels.push_back(get_label("SL:" + sub_label_strings[i]));
	    }
	}
      /*      else
	{
	  sub_label_map[id].push_back(id);
	  }*/
    }

  return label_map[label_string];
}

LabelVector no_sub_labels;

const LabelVector &LabelExtractor::sub_labels(unsigned int label) const
{ 
  if (sub_label_map.count(label) != 0)
    { return sub_label_map.find(label)->second; }
  else
    { return no_sub_labels; }
}

unsigned int LabelExtractor::label_count(void) const
{
  return string_map.size();
}

LabelVector LabelExtractor::get_labels(const StringVector &label_strings)
{
  LabelVector res;

  for (unsigned int i = 0; i < label_strings.size(); ++i)
    { 
      res.push_back(get_label(label_strings[i]));
    }

  return res;
}

std::string get_suffix(const std::string &wf, unsigned int length)
{
  return (PADDING + wf).substr(PADDING_LEN + wf.size() - length);
}

typedef std::unordered_map<std::string, std::unordered_map<unsigned int,
							   unsigned int> >
StringLabelCounts;

struct CmpPairs
{
  bool operator() (const std::pair<unsigned int, unsigned int> &p1,
		   const std::pair<unsigned int, unsigned int> &p2) const
  { 
    return p1.second > p2.second;
  }
};

void LabelExtractor::train(Data &data)
{
  StringLabelCounts inverse_label_counts;
  StringLabelCounts lexicon_counts;

  for (unsigned int i = 0; i < max_suffix_len + 1; ++i)
    { 
      label_counts[i].train(data);
    }

  std::vector<std::unordered_map<std::string, unsigned int> > words(10);
  
  for (unsigned int i = 0; i < data.size(); ++i)
    {
      for (unsigned int j = 0; j < data.at(i).size(); ++j)
	{
	  const Word &w = data.at(i).at(j);

	  std::string wf = w.get_word_form();
	  
	  words[i % 10][wf] = 1;

	  if (wf == BOUNDARY_WF)
	    { continue; }

	  unsigned int label = w.get_label();

	  if (lexicon_counts.count(wf) == 0 || lexicon_counts[wf].count(label) == 0)
	    { lexicon_counts[wf][label] = 0; }
	      
	  ++lexicon_counts[wf][label];
	}
    }

  for (unsigned int i = 0; i < 10; ++i)
    {
      for (std::unordered_map<std::string, unsigned int>::const_iterator it = words[i].begin();
	   it != words[i].end();
	   ++it)
	{
	  bool found = 0;

	  for (unsigned int j = 0; j < 10; ++j)
	    {
	      if (i != j and words[j].count(it->first) != 0)
		{ 
		  found = 1;
		  break;
		}		
	    }

	  if (not found)
	    { 
	      oov_words[it->first] = 1; 
	    }
	}
    }

  for (StringLabelCounts::const_iterator it = lexicon_counts.begin();
       it != lexicon_counts.end();
       ++it)
    {
      LabelVector &v = lexicon[it->first];

      for (std::unordered_map<unsigned int, unsigned int>::const_iterator jt =
	     it->second.begin();
	   jt != it->second.end();
	   ++jt)
	{ 
	  v.push_back(jt->first);

	  if (oov_words.count(it->first) != 0)
	    {
	      open_classes[jt->first] = 1;
	    }
	} 
    }  
}

bool LabelExtractor::is_oov(const std::string &wf) const
{ return lexicon.count(wf) == 0 or oov_words.count(wf) != 0; }

bool LabelExtractor::open_class(unsigned int label) const
{ return open_classes.count(label) != 0; }

void LabelExtractor::set_label_candidates(const std::string &word_form, 
					  bool use_lexicon,
					  float mass, 
					  LabelVector &target) const
{
  if (word_form == BOUNDARY_WF)
    { 
      target.push_back(get_boundary_label());
      return;
    }

  if (use_lexicon and lexicon.count(word_form) != 0)
    {
      //std::unordered_set<unsigned int> label_set(target.begin(), target.end());
      target.clear();
      std::unordered_set<unsigned int> label_set;
      SubstringLabelMap::const_iterator it = lexicon.find(word_form);
      label_set.insert(it->second.begin(), it->second.end());
      target.assign(label_set.begin(), label_set.end());
      return;
    }

  if (use_lexicon and not target.empty())
    { return; }

  int wf_len = 
    (word_form.size() < max_suffix_len ? word_form.size() : max_suffix_len); 

  while (wf_len > 0 && label_counts[wf_len].empty())
    { --wf_len; }

  label_counts[wf_len].set_guesses(word_form, target, mass);

  if (lexicon.count(word_form) != 0)
    {
      std::unordered_set<unsigned int> label_set(target.begin(), target.end());
      SubstringLabelMap::const_iterator it = lexicon.find(word_form);
      label_set.insert(it->second.begin(), it->second.end());
      target.clear();
      target.assign(label_set.begin(), label_set.end());
    }
}

const std::string &LabelExtractor::get_label_string(unsigned int label) const
{
  if (label >= string_map.size())
    { 
      throw IllegalLabel(); 
    }

  return string_map.at(label);
}

void LabelExtractor::store(std::ostream &out) const
{
  write_val(out, max_suffix_len);
  write_map(out, label_map);
  write_vector(out, string_map);

  for (unsigned int i = 0; i < max_suffix_len + 1; ++i)
    {
      label_counts[i].store(out);
    }

  write_map(out, lexicon);
  write_map(out, sub_label_map);
  write_map(out, oov_words);
  write_map(out, open_classes);
}

void LabelExtractor::load(std::istream &in, bool reverse_bytes)
{
  max_suffix_len = 0;
  label_map.clear();
  string_map.clear();
  label_counts.clear();
  lexicon.clear();
  sub_label_map.clear();
  oov_words.clear();
  open_classes.clear();

  read_val<unsigned int>(in, max_suffix_len, reverse_bytes);
  read_map   (in, label_map,     reverse_bytes);
  read_vector(in, string_map,    reverse_bytes);
  
  for (unsigned int i = 0; i < max_suffix_len + 1; ++i)
    {
      label_counts.push_back(SuffixLabelMap());
      label_counts.back().load(in, reverse_bytes);
    }

  read_map   (in, lexicon,       reverse_bytes);
  read_map   (in, sub_label_map, reverse_bytes);

  read_map(in, oov_words, reverse_bytes);
  read_map(in, open_classes, reverse_bytes);
}

bool LabelExtractor::operator==(const LabelExtractor &another) const
{
  if (this == &another)
    { return 1; }
  
  return
    (max_suffix_len == another.max_suffix_len and
     label_map == another.label_map           and
     string_map == another.string_map         and
     label_counts == another.label_counts     and
     lexicon == another.lexicon               and
     sub_label_map == another.sub_label_map   and
     oov_words == another.oov_words           and
     open_classes == another.open_classes);
}

#else // TEST_LabelExtractor_cc

#include <sstream>
#include <string>
#include <cassert>
//#include <unordered_set>
#include "UnorderedMapSet.hh"

#include "ParamTable.hh"
#include "Data.hh"

int main(void)
{
  std::string contents("\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN\t_\n"
		       ".\tWORD=.\t.\t.\t_\n"
		       "\n"
		       "\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN\t_\n"
		       ".\tWORD=.\t.\t.\t_\n"
		       "\n"
		       "\n"
		       ".\tWORD=.\t.\t.\t_\n");

  std::istringstream in(contents);

  LabelExtractor le(3);

  ParamTable pt;

  Data data(in, 1, le, pt, 2);

  le.train(data);

  LabelVector labels1;
  LabelVector labels2;

  le.set_label_candidates("hog", 0, 0.001, labels1);
  le.set_label_candidates("hog", 0, 1.01, labels2);

  std::unordered_set<unsigned int> labelS1(labels1.begin(), labels1.end());
  std::unordered_set<unsigned int> labelSS1;
  labelSS1.insert(le.get_label("NN"));
  //assert(labelS1 == labelSS1);

  assert(labels2.size() == 3);
  std::unordered_set<unsigned int> labelS2(labels2.begin(), labels2.end());
  assert(labelS2.size() == 3);
  assert(labelS2.count(0) == 0);
  std::unordered_set<unsigned int> labelSS2;
  labelSS2.insert(le.get_label("DT"));
  labelSS2.insert(le.get_label("NN"));
  labelSS2.insert(le.get_label("."));
  assert(labelS2 == labelSS2);

  LabelVector dog_labels;
  le.set_label_candidates("dog", 1, 5, dog_labels);
  assert(dog_labels.size() == 1);

  std::ostringstream le_out;
  le.store(le_out);
  std::istringstream le_in(le_out.str());
  LabelExtractor le_copy;
  le_copy.load(le_in, false);
  assert(le == le_copy);
}

#endif // TEST_LabelExtractor_cc
