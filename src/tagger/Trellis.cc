/**
 * @file    Trellis.cc                                                       
 * @Author  Miikka Silfverberg                                               
 * @brief   A class that knows the Viterbi algorithm and the forward-backward
 * algorithm.                                              
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

#include "Trellis.hh"

#ifndef TEST_Trellis_cc
#define TEST_Trellis_cc

#include <cmath>
#include <cfloat>

#include "Word.hh"

void normalize(std::vector<float> &v)
{
  float total = -FLT_MAX;

  for (unsigned int i = 0; i < v.size(); ++i)
    { 
      total = expsumlog(total, v[i]);
    }

    for (unsigned int i = 0; i < v.size(); ++i)
    { 
      v[i] = exp(v[i] - total);
    }
}

Trellis::Trellis(Sentence &sent, 
		 unsigned int boundary_label,
		 unsigned int beam):
  s(&sent),
  marginals_set(0),
  bw(boundary_label),
  beam(beam)
{
  reserve(sent.size(), boundary_label);

  for (unsigned int i = 0; i < sent.size() - 1; ++i)
    { 
      trellis[i].set_word(sent.at(i));
      trellis[i].set_ncol(&trellis[i+1]); 
    }

  trellis.back().set_word(sent.at(sent.size() - 1));
}

LabelVector Trellis::get_maximum_a_posteriori_assignment(const ParamTable &pt)
{
  LabelVector res;

  trellis.back().compute_viterbi(pt);
  trellis.back().set_labels(res);

  return res;
}

LabelVector Trellis::get_marginalized_max_assignment(const ParamTable &pt)
{
  set_marginals(pt);

  LabelVector res;

  for (unsigned int i = 0; i < s->size(); ++i)
    {
      float max_marginal = -FLT_MAX;
      unsigned int max_label = -1;

      for (unsigned int j = 0; j < s->at(i).get_label_count(); ++j)
	{
	  if (unigram_marginals[i][j] > max_marginal)
	    {
	      max_marginal = unigram_marginals[i][j];
	      max_label = s->at(i).get_label(j);
	    }
	}

      res.push_back(max_label);
    }

  return res;
}
#include <cassert>
void Trellis::set_maximum_a_posteriori_assignment(const ParamTable &pt)
{
  LabelVector labels = get_maximum_a_posteriori_assignment(pt);

  assert(labels.size() == s->size());

  for (unsigned int i = 0; i < labels.size(); ++i)
    {
      s->at(i).set_label(labels[i]);
      assert(s->at(i).get_label() != static_cast<unsigned int>(-1));
    }
}

void Trellis::set_marginalized_max_assignment(const ParamTable &pt)
{
  LabelVector labels = get_marginalized_max_assignment(pt);

  for (unsigned int i = 0; i < labels.size(); ++i)
    {
      s->at(i).set_label(labels[i]);
    }
}

void Trellis::reserve_marginals(void)
{  
  trigram_marginals.assign(trellis.size(), std::vector<float>());
  bigram_marginals.assign(trellis.size(), std::vector<float>());
  unigram_marginals.assign(trellis.size(), std::vector<float>());

  for (unsigned int i = 0; i < trellis.size(); ++i)
    {
      unsigned int labels = trellis[i].get_label_count();
      unigram_marginals[i].assign(labels, 0); 

      unsigned int label_bigrams = 
	labels * (i == 0 ? 1 : trellis[i - 1].get_label_count());

      bigram_marginals[i].assign(label_bigrams, 0); 

      if (i < 2)
	{ continue; }

      unsigned int label_trigrams = 
	label_bigrams * trellis[i - 2].get_label_count();

      trigram_marginals[i].assign(label_trigrams, 0); 
    }

}

void Trellis::set_unigram_marginals(void)
{
  for (unsigned int i = 0; i < trellis.size(); ++i)
    {
      for (unsigned int l = 0; l < trellis[i].get_label_count(); ++l)
	{
	  float unigram_score = -FLT_MAX;

	  for (unsigned int pl = 0; 
	       pl < (i == 0 ? 1 : trellis[i-1].get_label_count()); 
	       ++pl)
	    {
	      unigram_score = 
		expsumlog(unigram_score, 
			  bigram_marginals[i][get_index(i, l, pl)]);
	    }

	  unigram_marginals[i][get_index(i, l)] = unigram_score;
	}
    }
}

void Trellis::set_bigram_marginals(void)
{
  for (unsigned int i = 0; i < trellis.size(); ++i)
    {
      for (unsigned int l = 0; l < trellis[i].get_label_count(); ++l)
	{
	  for (unsigned int pl = 0; 
	       pl < (i == 0 ? 1 : trellis[i-1].get_label_count()); 
	       ++pl)
	    {
	      bigram_marginals[i][get_index(i, l, pl)] = 
		trellis[i].get_fw(pl, l) + trellis[i].get_bw(pl, l);
	    }
	}
    }
}

void Trellis::set_trigram_marginals(const ParamTable &pt)
{
  for (unsigned int i = 2; i < trellis.size(); ++i)
    {
      for (unsigned int l = 0; l < trellis[i].get_label_count(); ++l)
	{
	  unsigned int label = trellis[i].get_cell(0, l).label;

	  for (unsigned int pl = 0; pl < trellis[i-1].get_label_count(); ++pl)
	    {
	      unsigned int plabel = trellis[i - 1].get_cell(0, pl).label;

	      for (unsigned int ppl = 0; 
		   ppl < trellis[i-2].get_label_count(); 
		   ++ppl)
		{		 
		  unsigned int pplabel = trellis[i - 2].get_cell(0, ppl).label;

		  trigram_marginals[i][get_index(i, l, pl, ppl)] =
		    trellis[i - 1].get_fw(ppl, pl) + 
		    trellis[i].get_bw(pl, l) + 
		    pt.get_all_struct_fw(pplabel, plabel, label) +
		    pt.get_all_unstruct(s->at(i), label);
		}
	    }
	}
    }
}

void Trellis::set_marginals(const ParamTable &pt)
{
  if (marginals_set)
    { return; }

  marginals_set = 1;

  trellis[0].compute_bw(pt);
  trellis.back().compute_fw(pt);

  reserve_marginals();

  set_bigram_marginals();
  set_unigram_marginals();
  set_trigram_marginals(pt);

  for (unsigned int i = 1; i < trellis.size(); ++i)
    {
      normalize(trigram_marginals[i]);
      normalize(bigram_marginals[i]);
      normalize(unigram_marginals[i]);
    }
}

float Trellis::get_marginal(unsigned int position, 
			    unsigned int label) const
{
  return unigram_marginals[position][get_index(position, label)];
}

float Trellis::get_marginal(unsigned int position,
			    unsigned int plabel_index,
			    unsigned int label_index) const
{
  return bigram_marginals[position][get_index(position,
					      label_index, 
					      plabel_index)];
}

float Trellis::get_marginal(unsigned int position, 
			    unsigned int pplabel_index,
			    unsigned int plabel_index,
			    unsigned int label_index) const
{
  return trigram_marginals[position][get_index(position,
					       label_index,
					       plabel_index,
					       pplabel_index)];
}

unsigned int Trellis::size(void) const
{ 
  return trellis.size(); 
}

void Trellis::set_beam_mass(float mass)
{
  for (unsigned int i = 0; i < trellis.size(); ++i)
    { trellis.at(i).set_beam_mass(mass); }
}

void Trellis::set_beam(unsigned int beam)
{
  for (unsigned int i = 0; i < trellis.size(); ++i)
    { trellis.at(i).set_beam(beam); }
}

void Trellis::reserve(unsigned int n, unsigned int boundary_label)
{ 
  trellis.insert(trellis.end(), n, TrellisColumn(boundary_label, beam));
}

unsigned int Trellis::get_index(unsigned int position, 
				unsigned int l_index, 
				unsigned int pl_index, 
				unsigned int ppl_index) const
{
  unsigned int index = l_index;

  if (pl_index != static_cast<unsigned int>(-1))
    {
      index += pl_index * trellis[position].get_label_count();
    }

  if (ppl_index != static_cast<unsigned int>(-1))
    {
      index += ppl_index * trellis[position].get_label_count() * 
	trellis[position - 1].get_label_count();
    }

  return index;
}

void populate(Data &data, 
	      TrellisVector &v,
	      unsigned int boundary_label, 
	      unsigned int beam)
{
  for (unsigned int i = 0; i < data.size(); ++i)
    { v.push_back(Trellis(data.at(i), boundary_label, beam)); }
}

#else // TEST_Trellis_cc

class SillyLabelExtractor : public LabelExtractor
{
public:
  SillyLabelExtractor(void):
    LabelExtractor(1)
  {}

  void set_label_candidates(const std::string &word_form, 
			    bool use_lexicon,
			    unsigned int count, 
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

#include <cassert>
#include <cfloat>
#include <cmath>

bool float_eq(float f1, float f2)
{
  return fabs(f1 - f2) < 0.0001;
}

int main(void)
{
  FeatureTemplateVector dog_feats;
  dog_feats.push_back(0);
  dog_feats.push_back(1);
  
  LabelVector labels;
  labels.push_back(1);
  labels.push_back(9);

  Word boundary(0);

  Word dog("dog",
	   dog_feats,
	   labels,
	   "foo");

  FeatureTemplateVector cat_feats;
  cat_feats.push_back(2);
  cat_feats.push_back(3);
  
  Word cat("cat",
	   cat_feats,
	   labels,
	   "foo");

  FeatureTemplateVector horse_feats;
  horse_feats.push_back(4);
  horse_feats.push_back(5);
  
  Word horse("horse",
	     horse_feats,
	     labels,
	     "foo");

  WordVector words;
  words.push_back(dog);
  words.push_back(cat);
  words.push_back(horse);

  SillyLabelExtractor label_extractor;

  Sentence s(words, label_extractor, 2);

  ParamTable pt;
  
  // Random init parameters.

  pt.update_unstruct(0, 1, 2.056);
  pt.update_unstruct(0, 9, 4.096);
  pt.update_unstruct(1, 1, 3.602);
  pt.update_unstruct(1, 9, 8.519);
  pt.update_unstruct(2, 1, 7.676);
  pt.update_unstruct(2, 9, 9.619);
  pt.update_unstruct(3, 1, 5.574);
  pt.update_unstruct(3, 9, 3.167);
  pt.update_unstruct(4, 1, 0.280);
  pt.update_unstruct(4, 9, 0.778);
  pt.update_unstruct(5, 1, 4.386);
  pt.update_unstruct(5, 9, 1.145);

  pt.update_struct3(0,0,1,6.521);
  pt.update_struct3(0,0,9,7.494);
  pt.update_struct2(0,9,5.891, false);
  pt.update_struct2(0,1,0.883, false);
  pt.update_struct1(9,2.275);
  pt.update_struct1(1,3.68);

  pt.update_struct3(1,1,1,5.206);
  pt.update_struct3(1,1,9,4.958);
  pt.update_struct2(1,1,3.883, false);
  pt.update_struct2(1,9,4.309, false);

  pt.update_struct3(1,9,1,9.494);
  pt.update_struct3(1,9,9,6.355);
  pt.update_struct2(9,1,0.358, false);
  pt.update_struct2(9,9,6.690, false);

  Trellis trellis(s, 0);
  trellis.set_marginals(pt);

  LabelVector v = trellis.get_maximum_a_posteriori_assignment(pt);

  assert(v.size() == s.size());

  LabelVector v1 = trellis.get_marginalized_max_assignment(pt);

  assert(v == v1);

  // Manually compute total score and scores for (1), (1,0) and
  // (1,0,1) for horse in "dog cat horse".

  float tot_score = -FLT_MAX;

  float pos_3_l_1_score = -FLT_MAX;
  float pos_3_l_1_0_score = -FLT_MAX;
  float pos_3_l_1_0_1_score = -FLT_MAX;

  std::vector<unsigned int> max_label_indices(3, -1);
  
  for (unsigned int i = 0; i < 2; ++i)
    {
      for (unsigned int j = 0; j < 2; ++j)
	{
	  for (unsigned int k = 0; k < 2; ++k)
	    {
	      float f = 0;

	      f += pt.get_unstruct(0, labels[i]);
	      f += pt.get_unstruct(1, labels[i]);
	      f += pt.get_unstruct(2, labels[j]);
	      f += pt.get_unstruct(3, labels[j]);
	      f += pt.get_unstruct(4, labels[k]);
	      f += pt.get_unstruct(5, labels[k]);

	      f += pt.get_struct3(0, 0, labels[i]);
	      f += pt.get_struct2(0, labels[i], false);
	      f += pt.get_struct1(labels[i]);

	      f += pt.get_struct3(0, labels[i], labels[j]);
	      f += pt.get_struct2(labels[i], labels[j], false);
	      f += pt.get_struct1(labels[j]);

	      f += pt.get_struct3(labels[i], labels[j], labels[k]);
	      f += pt.get_struct2(labels[j], labels[k], false);
	      f += pt.get_struct1(labels[k]);
	      
	      f += pt.get_struct3(labels[j], labels[k], 0);
	      f += pt.get_struct3(labels[k], 0, 0);
	      f += pt.get_struct2(labels[k], 0, false);

	      tot_score = expsumlog(tot_score, f);
	      
	      if (k == 1)
		{
		  pos_3_l_1_score = expsumlog(pos_3_l_1_score, f);

		  if (j == 0)
		    {
		      pos_3_l_1_0_score = expsumlog(pos_3_l_1_0_score, f);

		      if (i == 1)
			{
			  pos_3_l_1_0_1_score = 
			    expsumlog(pos_3_l_1_0_1_score, f);
			}
		    }
		}
	    }
	}
    }

  assert(float_eq(exp(pos_3_l_1_score - tot_score), 
		  trellis.get_marginal(3,1)));
  assert(float_eq(exp(pos_3_l_1_0_score - tot_score), 
		  trellis.get_marginal(3,0,1)));
  assert(float_eq(exp(pos_3_l_1_0_1_score - tot_score), 
		  trellis.get_marginal(3,1,0,1)));  
}

#endif // TEST_Trellis_cc
