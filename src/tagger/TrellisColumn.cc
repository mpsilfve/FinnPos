/**
 * @file    TrellisColumn.hh                                                 
 * @Author  Miikka Silfverberg                                               
 * @brief   A class for representing the columns of a Trellis.        
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

#include "TrellisColumn.hh"
#include "io.hh"

float expsumlog(float x, float y);

#ifndef TEST_TrellisColumn_cc

#include <algorithm>
#include <cmath>
#include <cfloat>

#define STRUCT_SL 1

float expsumlog(float x, float y)
{
  if (y > x)
    {
      std::swap(x,y);
    }

  return (x - y > 50 ? x : (x + log(1 + exp(y - x))));
}

TrellisColumn::TrellisColumn(unsigned int boundary_label,
			     unsigned int beam_width):
  pcol                        (0),
  ncol                        (0),
  word                        (0),
  boundary_label (boundary_label),
  beam_width(beam_width),
  use_adaptive_beam(0)
{}  

void TrellisColumn::set_ncol(TrellisColumn * ncol)
{
  this->ncol = ncol;
  ncol->pcol = this;
}

void TrellisColumn::set_word(const Word &word, int plabels)
{
  this->word = &word;

  label_count  = word.get_label_count();  
  plabel_count = plabels; 

  reserve(label_count * plabel_count);

  for (unsigned int plabel_index = 0; 
       plabel_index < plabel_count; 
       ++plabel_index)
    {
      for (unsigned int label_index = 0; 
	   label_index < label_count; 
	   ++label_index)
	{
	  get_cell(plabel_index, label_index).label = 
	    word.get_label(label_index);
	  get_cell(plabel_index, label_index).label_index = 
	    label_index;
	}
    }
}

unsigned int TrellisColumn::get_label_count(void) const
{ 
  return label_count;
}

void TrellisColumn::reserve(unsigned int label_count)
{
  int diff = static_cast<int>(label_count) - cells.size();
  cells.insert(cells.end(), diff, TrellisCell());
}

TrellisCell &TrellisColumn::get_cell(unsigned int plabel_index, 
				     unsigned int label_index)
{
  return cells.at(plabel_count * label_index + plabel_index);
}

const TrellisCell &TrellisColumn::get_cell(unsigned int plabel_index, 
					   unsigned int label_index) const
{
  return cells.at(plabel_count * label_index + plabel_index);
}

float TrellisColumn::get_fw(unsigned int plabel_index, 
			    unsigned int label_index) const
{
  return get_cell(plabel_index, label_index).fw;
}

float TrellisColumn::get_bw(unsigned int plabel_index, 
			    unsigned int label_index) const
{
  return get_cell(plabel_index, label_index).bw;
}

float TrellisColumn::get_viterbi(unsigned int plabel_index, 
				 unsigned int label_index) const
{
  return get_cell(plabel_index, label_index).viterbi;
}

unsigned int TrellisColumn::get_label(unsigned int label_index) const
{ 
  return word->get_label(label_index);
}

unsigned int TrellisColumn::get_plabel(unsigned int plabel_index) const
{ 
  return (pcol == 0 ? boundary_label : pcol->get_label(plabel_index));
}
unsigned int TrellisColumn::get_pplabel(unsigned int pplabel_index) const
{
  return (pcol == 0 ? boundary_label: pcol->get_plabel(pplabel_index));
}

unsigned int TrellisColumn::get_nlabel(unsigned int nlabel_index) const
{
  return (ncol == 0 ? boundary_label : ncol->get_label(nlabel_index));
}

float TrellisColumn::get_emission_score(const ParamTable &pt,
					unsigned int label_index) const
{
  return pt.get_all_unstruct(*word, word->get_label(label_index), 1);
}

float TrellisColumn::get_transition_fw_score(const ParamTable &pt, 
					     unsigned int label_index, 
					     unsigned int plabel_index) const
{
  float score = -FLT_MAX;

  for (unsigned int k = 0; k < (pcol == 0 ? 1 : pcol->plabel_count); ++k)
    {
      unsigned int pplabel = get_pplabel(k);
      unsigned int plabel  = get_plabel(plabel_index);      
      unsigned int label   = get_label(label_index);

      float pcol_fw = (pcol == 0 ? 0 : pcol->get_fw(k, plabel_index));
      
      float tr_score = pt.get_all_struct_fw(pplabel, plabel, label, STRUCT_SL);

      score = expsumlog(score, tr_score + pcol_fw);
    }

  return score;
}

float TrellisColumn::get_transition_bw_score(const ParamTable &pt, 
					     unsigned int plabel_index, 
					     unsigned int label_index) const
{
  float score = -FLT_MAX;

  for (unsigned int i = 0; i < (ncol == 0 ? 1 : ncol->label_count); ++i)
    {
      float ncol_em = (ncol == 0 ? 0 : ncol->get_emission_score(pt, i));
      float ncol_bw = (ncol == 0 ? 0 : ncol->get_bw(label_index, i));
      
      unsigned int plabel = get_plabel(plabel_index);
      unsigned int label  = get_label(label_index);
      unsigned int nlabel = get_nlabel(i);

      float tr_score = pt.get_all_struct_bw(plabel, label, nlabel, STRUCT_SL);

      score = expsumlog(score, tr_score + ncol_em + ncol_bw);
    }

  return score;
}

void TrellisColumn::compute_fw(const ParamTable &pt)
{
  if (word == 0)
    {
      throw WordNotSet();
    }

  if (pcol != 0)
    {
      pcol->compute_fw(pt);
    }

  for (unsigned int i = 0; i < label_count; ++i)
    {
      float em = get_emission_score(pt, i); 

      for (unsigned int j = 0; j < plabel_count; ++j)
	{
	  float tr = get_transition_fw_score(pt, i, j);

	  get_cell(j, i).fw = em + tr;
	}
    }
}

void TrellisColumn::compute_bw(const ParamTable &pt)
{
  if (word == 0)
    {
      throw WordNotSet();
    }

  if (ncol != 0)
    {
      ncol->compute_bw(pt);
    }

  for (unsigned int i = 0; i < label_count; ++i)
    {
      for (unsigned int j = 0; j < plabel_count; ++j)
	{
	  TrellisCell &c = get_cell(j, i);
	  
	  c.bw = get_transition_bw_score(pt, j, i);
	}
    }
}

void set_labels(LabelVector &res, TrellisCell * c, unsigned int boundary_label)
{
  res.push_back(c->label);

  c = c->pcell;

  if (c != 0)
    { 
      set_labels(res, c, boundary_label);
    }
}

#include <cassert>

void TrellisColumn::set_labels(LabelVector &res)
{
  if (word == 0)
    { return; }

  TrellisCell * c = &get_cell(0,0);
  assert(c != 0);
  assert(c->pcell != 0); 

  ::set_labels(res, c, boundary_label);

  std::reverse(res.begin(), res.end());
}


void TrellisColumn::compute_viterbi(const ParamTable &pt)
{ 
  if (word == 0)
    {
      throw WordNotSet();
    }

  if (pcol != 0)
    {
      pcol->compute_viterbi(pt);
    }

  for (unsigned int i = 0; i < label_count; ++i)
    {
      float em = get_emission_score(pt, i); 
      
      if (pcol == 0)
	{
	  set_viterbi_tr_score(pt, 0, i);

	  get_cell(0, i).viterbi += em;
	  get_cell(0, i).fw += em;

	  cells_in_beam.push_back(&get_cell(0,i));
	}
      else
	{ 
	  unsigned int p_col_beam_cell_count = pcol->beam_cell_count();

	  for (unsigned int j = 0; j < beam_width; ++j)
	    {
	      if (j >= p_col_beam_cell_count)
		{ 
		  assert(j != 0);
		  break; 
		}

	      TrellisCell * pcell = pcol->get_beam_cell(j);
	      
	      unsigned int pplabel = (pcell->pcell == 0 ? boundary_label : pcell->pcell->label);
	      unsigned int plabel  = pcell->label;      
	      unsigned int label   = get_label(i);
	      
	      float pcol_score = (pcol == 0 ? 0 : pcell->viterbi);

	      float tr_score = pt.get_all_struct_fw(pplabel, plabel, label, STRUCT_SL);
	      
	      float score = tr_score + pcol_score + em;
	      	      
	      get_cell(pcell->label_index, i).fw = expsumlog(get_cell(pcell->label_index, i).fw,
							     pcell->fw + tr_score + em);
	      if (get_cell(pcell->label_index, i).viterbi == -FLT_MAX)
		{
		  cells_in_beam.push_back(&get_cell(pcell->label_index, i));
		}

	      if (score > get_cell(pcell->label_index, i).viterbi or 
		  get_cell(pcell->label_index, i).pcell == 0)
		{
		  get_cell(pcell->label_index, i).viterbi = score;
		  get_cell(pcell->label_index, i).pcell = pcell;
		}	      
	    }
	}
    }

  std::stable_sort(cells_in_beam.begin(), cells_in_beam.end(), CmpTrellisCell());
}

TrellisCell * TrellisColumn::get_beam_cell(unsigned int i)
{
  return cells_in_beam.at(i);
}

void TrellisColumn::set_beam_mass(float mass)
{
  use_adaptive_beam = 1;
  beam_mass = mass;
}

void TrellisColumn::set_beam(unsigned int beam)
{
  beam_width = beam;
}

unsigned int TrellisColumn::beam_cell_count(void)
{
  if (not use_adaptive_beam)
    { 
      return std::min(size_t(beam_width), cells_in_beam.size()); 
    }

  float tot_mass = -FLT_MAX;

  for (size_t i = 0; i < cells_in_beam.size(); ++i)
    {
      tot_mass = expsumlog(tot_mass, cells_in_beam.at(i)->fw);
    }

  float prefix_mass = -FLT_MAX;

  for (size_t i = 0; i < cells_in_beam.size(); ++i)
    {
      prefix_mass = expsumlog(prefix_mass, cells_in_beam.at(i)->fw);

      if (i > 200 or (exp(prefix_mass - tot_mass) > beam_mass and i > 4))
	{
	  return i + 1; 
	}
    }

  return cells_in_beam.size(); 
}

void TrellisColumn::set_viterbi_tr_score(const ParamTable &pt, 
					 unsigned int plabel_index,
					 unsigned int label_index)
{
  unsigned int max_pplabel_index = -1;
  float max_score = -FLT_MAX;
  float tot_score = -FLT_MAX;

  for (unsigned int k = 0; k < (pcol == 0 ? 1 : pcol->plabel_count); ++k)
    {
      unsigned int pplabel = get_pplabel(k);
      unsigned int plabel  = get_plabel(plabel_index);      
      unsigned int label   = get_label(label_index);
      
      float pcol_score = (pcol == 0 ? 0 : pcol->get_viterbi(k, plabel_index));

      float tr_score = pt.get_all_struct_fw(pplabel, plabel, label, STRUCT_SL);

      float score = tr_score + pcol_score;

      if (score > max_score)
	{
	  max_score = score;
	  max_pplabel_index = k;
	}

      float pcol_fw_score = (pcol == 0 ? 0 : pcol->get_fw(k, plabel_index));
      float fw_score = tr_score + pcol_fw_score;
      tot_score = expsumlog(tot_score, fw_score);
    }
  
  TrellisCell &c = get_cell(plabel_index, label_index);

  c.viterbi = max_score;
  c.fw = tot_score;

  c.pcell = 
    (pcol == 0 ? 0 : &(pcol->get_cell(max_pplabel_index, plabel_index)));
}

#else // TEST_TrellisColumn_cc

#include <cassert>
#include <iostream>
#include <cmath>
#include <cfloat>

bool float_equals(float f1, float f2)
{
  return fabs(f1 - f2) < 0.0001;
}

int main(void)
{
  TrellisColumn trellis_column0(0);

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

  TrellisColumn lbcol(0,4);
  TrellisColumn col0(0,4);
  TrellisColumn col1(0,4);
  TrellisColumn col2(0,4);
  TrellisColumn rbcol1(0,4);
  TrellisColumn rbcol2(0,4);

  lbcol.set_ncol(&col0);
  col0.set_ncol(&col1);
  col1.set_ncol(&col2);
  col2.set_ncol(&rbcol1);
  rbcol1.set_ncol(&rbcol2);
  
  lbcol.set_word(boundary, 0);
  col0.set_word(dog, 0);
  col1.set_word(cat, 0);
  col2.set_word(horse, 0);
  rbcol1.set_word(boundary, 0);
  rbcol2.set_word(boundary, 0);

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
  pt.update_struct1(9,2.275, false);
  pt.update_struct1(1,3.68, false);

  pt.update_struct3(1,1,1,5.206);
  pt.update_struct3(1,1,9,4.958);
  pt.update_struct2(1,1,3.883, false);
  pt.update_struct2(1,9,4.309, false);

  pt.update_struct3(1,9,1,9.494);
  pt.update_struct3(1,9,9,6.355);
  pt.update_struct2(9,1,0.358, false);
  pt.update_struct2(9,9,6.690, false);

  rbcol2.compute_fw(pt);
  rbcol2.compute_viterbi(pt);
  lbcol.compute_bw(pt);

  // Manually compute total score for "dog cat horse".
  float tot_score = -FLT_MAX;
  float max_score = -FLT_MAX;

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
	      f += pt.get_struct1(labels[i], false);

	      f += pt.get_struct3(0, labels[i], labels[j]);
	      f += pt.get_struct2(labels[i], labels[j], false);
	      f += pt.get_struct1(labels[j], false);

	      f += pt.get_struct3(labels[i], labels[j], labels[k]);
	      f += pt.get_struct2(labels[j], labels[k], false);
	      f += pt.get_struct1(labels[k], false);
	      
	      f += pt.get_struct3(labels[j], labels[k], 0);
	      f += pt.get_struct3(labels[k], 0, 0);
	      f += pt.get_struct2(labels[k], 0, false);

	      tot_score = expsumlog(tot_score, f);

	      if (f >= max_score)
		{
		  max_score = f;

		  max_label_indices[0] = i;
		  max_label_indices[1] = j;
		  max_label_indices[2] = k;
		}
	    }
	}
    }

  /*
  // Compute fw + bw score for postions 0, 1, 2 and boundaries. The
  // sums should all equal tot_score.

  float scorelb = -FLT_MAX;

  scorelb = expsumlog(scorelb, lbcol.get_bw(0,0) + lbcol.get_fw(0,0));
  //  scorelb = expsumlog(scorelb, lbcol.get_bw(0,1) + lbcol.get_fw(0,1));

  float score0 = -FLT_MAX;

  score0 = expsumlog(score0, col0.get_bw(0,0) + col0.get_fw(0,0));
  score0 = expsumlog(score0, col0.get_bw(0,1) + col0.get_fw(0,1));

  float score1 = -FLT_MAX;

  score1 = expsumlog(score1, col1.get_bw(0,0) + col1.get_fw(0,0));
  score1 = expsumlog(score1, col1.get_bw(0,1) + col1.get_fw(0,1));
  score1 = expsumlog(score1, col1.get_bw(1,0) + col1.get_fw(1,0));
  score1 = expsumlog(score1, col1.get_bw(1,1) + col1.get_fw(1,1));

  float score2 = -FLT_MAX;

  score2 = expsumlog(score2, col2.get_bw(0,0) + col2.get_fw(0,0));
  score2 = expsumlog(score2, col2.get_bw(0,1) + col2.get_fw(0,1));
  score2 = expsumlog(score2, col2.get_bw(1,0) + col2.get_fw(1,0));
  score2 = expsumlog(score2, col2.get_bw(1,1) + col2.get_fw(1,1));

  float scorerb1 = -FLT_MAX;

  scorerb1 = expsumlog(scorerb1, rbcol1.get_bw(1,0) + rbcol1.get_fw(1,0));
  scorerb1 = expsumlog(scorerb1, rbcol1.get_bw(0,0) + rbcol1.get_fw(0,0));

  float scorerb2 = -FLT_MAX;
  
  scorerb2 = expsumlog(scorerb2, rbcol2.get_bw(0,0) + rbcol2.get_fw(0,0));
  assert(float_equals(tot_score, scorelb));
  assert(float_equals(tot_score, score0));
  assert(float_equals(tot_score, score1));
  assert(float_equals(tot_score, score2));
  assert(float_equals(tot_score, scorerb1));
  assert(float_equals(tot_score, scorerb2));

  assert(float_equals(max_score, rbcol2.get_viterbi(0, 0)));
  */
  ParamTable foo_pt;
  foo_pt.update_unstruct(0,1,10);
  foo_pt.update_struct3(2,2,2,1000);
  foo_pt.update_struct2(2,2,-1, false);

  FeatureTemplateVector foo_feats(1,0);

  LabelVector foo_labels(2);
  foo_labels[0] = 1;
  foo_labels[1] = 2;

  Word foo("foo",
	   foo_feats,
	   foo_labels,
	   "foo");
  
  TrellisColumn foo_col1(0);
  TrellisColumn foo_col2(0);
  TrellisColumn foo_col3(0);
  TrellisColumn foo_col4(0);
  TrellisColumn foo_col5(0);
  TrellisColumn foo_col6(0);

  foo_col1.set_ncol(&foo_col2);
  foo_col2.set_ncol(&foo_col3);
  foo_col3.set_ncol(&foo_col4);
  foo_col4.set_ncol(&foo_col5);
  foo_col5.set_ncol(&foo_col6);

  foo_col1.set_word(boundary, 0);
  foo_col2.set_word(foo, 0);
  foo_col3.set_word(foo, 0);
  foo_col4.set_word(foo, 0);
  foo_col5.set_word(boundary, 0);
  foo_col6.set_word(boundary, 0);

  foo_col6.compute_viterbi(foo_pt);
  assert(float_equals(foo_col6.get_viterbi(0,0), 1000 - 2));

  TrellisColumn bar_col1(0,4);
  TrellisColumn bar_col2(0,4);
  TrellisColumn bar_col3(0,4);
  TrellisColumn bar_col4(0,4);
  TrellisColumn bar_col5(0,4);
  TrellisColumn bar_col6(0,4);

  bar_col1.set_ncol(&bar_col2);
  bar_col2.set_ncol(&bar_col3);
  bar_col3.set_ncol(&bar_col4);
  bar_col4.set_ncol(&bar_col5);
  bar_col5.set_ncol(&bar_col6);

  bar_col1.set_word(boundary, 0);
  bar_col2.set_word(foo, 0);
  bar_col3.set_word(foo, 0);
  bar_col4.set_word(foo, 0);
  bar_col5.set_word(boundary, 0);
  bar_col6.set_word(boundary, 0);

  bar_col6.compute_viterbi(foo_pt);
  assert(float_equals(bar_col6.get_viterbi(0,0), 1000 - 2));

  TrellisColumn baz_col1(0,2);
  TrellisColumn baz_col2(0,2);
  TrellisColumn baz_col3(0,2);
  TrellisColumn baz_col4(0,2);
  TrellisColumn baz_col5(0,2);
  TrellisColumn baz_col6(0,2);

  baz_col1.set_ncol(&baz_col2);
  baz_col2.set_ncol(&baz_col3);
  baz_col3.set_ncol(&baz_col4);
  baz_col4.set_ncol(&baz_col5);
  baz_col5.set_ncol(&baz_col6);

  baz_col1.set_word(boundary, 0);
  baz_col2.set_word(foo, 0);
  baz_col3.set_word(foo, 0);
  baz_col4.set_word(foo, 0);
  baz_col5.set_word(boundary, 0);
  baz_col6.set_word(boundary, 0);

  baz_col6.compute_viterbi(foo_pt);
 
  assert(float_equals(baz_col6.get_viterbi(0,0), 30));
}

#endif // TEST_TrellisColumn_cc
