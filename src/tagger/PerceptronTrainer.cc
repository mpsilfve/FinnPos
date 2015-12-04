/**
 * @file    PerceptronTrainer.cc                                             
 * @Author  Miikka Silfverberg                                               
 * @brief   An instance of Trainer that performs averaged perceptron.        
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

#include "PerceptronTrainer.hh"

#ifndef TEST_PerceptronTrainer_cc

#include <cassert>

#include "Trellis.hh"

#define STRUCT_SL 1
#define USTRUCT_SL 1

PerceptronTrainer::PerceptronTrainer(unsigned int max_passes,
				     unsigned int max_useless_passes,
				     ParamTable &pt,
				     const LabelExtractor &label_extractor,
				     const LemmaExtractor &lemma_extractor,
				     std::ostream &msg_out,
				     const TaggerOptions &options):  
  Trainer(max_passes, max_useless_passes, pt, label_extractor, lemma_extractor, msg_out, options),
  iter(0), 
  //use_unstruct_sub_labels(options.use_unstructured_sublabels), 
  //use_struct_sub_labels(options.use_structured_sublabels)
  sublabel_order(options.sublabel_order),
  model_order(options.model_order)
{
  pos_params = pt;
  neg_params = pt;

  pos_params.set_label_extractor(label_extractor);
  neg_params.set_label_extractor(label_extractor);
}

void PerceptronTrainer::train(const Data &train_data, 
			      const Data &dev_data,
			      unsigned int beam,
			      float beam_mass)
{
  Data train_data_copy(train_data);

  Data dev_data_copy(dev_data);
  dev_data_copy.unset_label();

  TrellisVector train_trellises;
  std::cerr << "SL: " << sublabel_order << std::endl;
  for (unsigned int i = 0; i < train_data.size(); ++i)
    {      
      train_trellises.push_back(new Trellis(train_data_copy.at(i), boundary_label,
					    sublabel_order, model_order, beam));

      if (beam != static_cast<unsigned int>(-1))
	{ train_trellises.back()->set_beam(beam); }
      if (beam_mass != -1)
	{ train_trellises.back()->set_beam_mass(beam_mass); }
    }

  TrellisVector dev_trellises;

  for (unsigned int i = 0; i < dev_data.size(); ++i)
    {
      dev_trellises.push_back(new Trellis(dev_data_copy.at(i), boundary_label, sublabel_order, model_order, beam));

      if (beam != static_cast<float>(-1))
	{ dev_trellises.back()->set_beam(beam); }
      if (beam_mass != -1)
	{ dev_trellises.back()->set_beam_mass(beam_mass); }
    }

  float best_dev_acc = -1;
  ParamTable best_params;
  best_params.set_label_extractor(label_extractor);

  unsigned int useless_passes = 0;

  for (unsigned int i = 0; i < max_passes; ++i)
    {
      if (useless_passes >= max_useless_passes)
	{ break; }

      msg_out << "  Train pass " << i + 1 << std::endl;

      // Train pass.
      for (unsigned int j = 0; j < train_trellises.size(); ++j)
	{
	  train_trellises[j]->set_maximum_a_posteriori_assignment
	    (pos_params);

	  update(train_data.at(j), train_data_copy.at(j));
	  
	  std::cerr << j << " of " << train_trellises.size() - 1 << "\r";
	}

      std::cerr << std::endl;

      // Average.
      set_avg_params();

      // Tag dev data.
      for (unsigned int j = 0; j < dev_trellises.size(); ++j)
	{
	  dev_trellises[j]->set_maximum_a_posteriori_assignment(pt);
	}

      float acc = dev_data.get_acc(dev_data_copy, lemma_extractor).label_acc;

      msg_out << "    Dev acc: " << acc * 100.00 << "%" << std::endl;

      if (acc > best_dev_acc)
	{
	  useless_passes = 0;
	  best_dev_acc = acc;
	  best_params = pt;
	}
      else
	{
	  ++useless_passes;
	}
    }

  msg_out << "  Final dev acc: " << best_dev_acc * 100.00 << "%" << std::endl;

  pt = best_params;
  pt.set_label_extractor(label_extractor);
  pt.set_trained();

  for (TrellisVector::const_iterator it = train_trellises.begin();
       it != train_trellises.end();
       ++it)
    { delete *it; }

  for (TrellisVector::const_iterator it = dev_trellises.begin();
       it != dev_trellises.end();
       ++it)
    { delete *it; }
}

void PerceptronTrainer::train_lemmatizer(const Data &train_data, 
					 const Data &dev_data,
					 LemmaExtractor &lemma_e,
					 const LabelExtractor &label_e)
{
  std::vector<Word> train_words;
  std::vector<Word> dev_words;
  
  std::unordered_map<unsigned int , unsigned int> class_counts;

  for (unsigned int i = 0; i < train_data.size(); ++i)
    {
      for (unsigned int j = 0; j < train_data.at(i).size(); ++j)
	{
	  Word w = train_data.at(i).at(j);

	  Word * ww = lemma_e.extract_feats(w.get_word_form(),
					    label_e.get_label_string(w.get_label()));

	  unsigned int klass = lemma_e.get_class_number(w.get_word_form(), 
							w.get_lemma());

	  ww->set_label(klass);
	  
	  class_counts[klass] += 1;

	  train_words.push_back(*ww);

	  delete ww;

	  for (size_t i = 0; i < w.analyzer_lemmas.size(); ++i)
	    {
	      if (w.analyzer_lemmas[i].first == w.get_label() and
		  w.analyzer_lemmas[i].second == w.get_lemma())
		{ continue; }

	      ww = lemma_e.extract_feats
		(w.get_word_form(),
		 label_e.get_label_string(w.analyzer_lemmas[i].first));
	      
	      ww->set_label(lemma_e.get_class_number(w.get_word_form(), 
						     w.analyzer_lemmas[i].second));

	      train_words.push_back(*ww);

	      delete ww;
	      }
	}
    }
 
  std::vector<Word> train_words_copy;

  for (unsigned int i = 0; i < train_words.size(); ++i)
    {
      if (class_counts[train_words[i].get_label()] > 0)
      	{ train_words_copy.push_back(train_words[i]); }
    }

  train_words = train_words_copy;
  
  std::srand(0);
  std::random_shuffle(train_words.begin(), train_words.end());

  for (unsigned int i = 0; i < dev_data.size(); ++i)
    {
      for (unsigned int j = 0; j < dev_data.at(i).size(); ++j)
	{
	  const Word &w = dev_data.at(i).at(j);

	  Word * ww = lemma_e.extract_feats(w.get_word_form(),
					    label_e.get_label_string(w.get_label()));

	  ww->set_label(lemma_e.get_class_number(w.get_word_form(), 
						 w.get_lemma()));

	  dev_words.push_back(*ww);
	  delete ww;
	}
    }

  float best_dev_acc = -1;
  ParamTable best_params;
  unsigned int useless_passes = 0;

  for (unsigned int i = 0; i < max_passes; ++i)
    {
      if (useless_passes >= max_useless_passes)
	{ break; }

      msg_out << "  Train pass " << i + 1 << ":" << std::endl;

      // Train pass.
      for (unsigned int j = 0; j < train_words.size(); ++j)
	{
	  Word &w = train_words.at(j);

	  unsigned int gold_class = w.get_label();

	  unsigned int sys_class = 
	    lemma_e.get_lemma_candidate_class(w, &pos_params);

	  lemmatizer_update(w, sys_class, gold_class, lemma_e, label_e);
	}
      
      // Average.
      set_avg_params();

      // Tag dev data.
      float correct = 0;
      float total = 0; 

      for (unsigned int j = 0; j < dev_words.size(); ++j)
	{
	  Word &w = dev_words.at(j);

	  unsigned int gold_class = w.get_label();

	  unsigned int sys_class = 
	    lemma_e.get_lemma_candidate_class(w);

	  correct += (sys_class == gold_class) ? 1 : 0;
	  ++total;
	}

      float acc = (total == 0 ? 0 : correct / total);

      msg_out << "    Dev acc: " << acc * 100.00 << "%" << std::endl;

      if (acc > best_dev_acc)
	{
	  useless_passes = 0;
	  best_dev_acc = acc;
	  best_params = pt;
	}
      else
	{
	  ++useless_passes;
	}
    }

  msg_out << "  Final dev acc: " << best_dev_acc * 100.0 << "%" << std::endl;
  
  pt = best_params;
  pt.set_trained();
}

void PerceptronTrainer::update(const Sentence &gold_s, 
			       const Sentence &sys_s)
{
  ++iter;

  for (unsigned int i = 0; i /*<= max_violation_id*/ < sys_s.size() ; ++i)
    {
      unsigned int gold_label = gold_s.at(i).get_label();
      unsigned int sys_label = sys_s.at(i).get_label();

      unsigned int pgold_label = (i < 1 ? boundary_label : gold_s.at(i - 1).get_label());
      unsigned int psys_label = (i < 1 ? boundary_label : sys_s.at(i - 1).get_label());

      unsigned int ppgold_label = (i < 2 ? boundary_label : gold_s.at(i - 2).get_label());
      unsigned int ppsys_label = (i < 2 ? boundary_label : sys_s.at(i - 2).get_label());

      // Unstruct params.
      //pos_params.update_all_unstruct(gold_s.at(i), gold_label, 1, use_unstruct_sub_labels);
      //neg_params.update_all_unstruct(gold_s.at(i), gold_label, -iter, use_unstruct_sub_labels);
      pos_params.update_all_unstruct(gold_s.at(i), gold_label, 1, sublabel_order);
      neg_params.update_all_unstruct(gold_s.at(i), gold_label, -iter, sublabel_order);

      //pos_params.update_all_unstruct(sys_s.at(i), sys_label, -1, use_unstruct_sub_labels);
      //neg_params.update_all_unstruct(sys_s.at(i), sys_label, iter, use_unstruct_sub_labels);
      pos_params.update_all_unstruct(sys_s.at(i), sys_label, -1, sublabel_order);
      neg_params.update_all_unstruct(sys_s.at(i), sys_label, iter, sublabel_order);

      // Struct params.
      //pos_params.update_all_struct_fw(ppgold_label, pgold_label, gold_label, 1, use_struct_sub_labels);
      //neg_params.update_all_struct_fw(ppgold_label, pgold_label, gold_label, -iter, use_struct_sub_labels);
      pos_params.update_all_struct_fw(ppgold_label, pgold_label, gold_label, 1, sublabel_order, model_order);
      neg_params.update_all_struct_fw(ppgold_label, pgold_label, gold_label, -iter, sublabel_order, model_order);
      
      //pos_params.update_all_struct_fw(ppsys_label, psys_label, sys_label, -1, use_struct_sub_labels);
      //neg_params.update_all_struct_fw(ppsys_label, psys_label, sys_label, iter, use_struct_sub_labels);
      pos_params.update_all_struct_fw(ppsys_label, psys_label, sys_label, -1, sublabel_order, model_order);
      neg_params.update_all_struct_fw(ppsys_label, psys_label, sys_label, iter, sublabel_order, model_order);
    }
}

void PerceptronTrainer::lemmatizer_update(const Word &w, 
					  unsigned int sys_class, 
					  unsigned int gold_class,
					  LemmaExtractor &lemma_e,
					  const LabelExtractor &label_e)
{
  ++iter;

  static_cast<void>(lemma_e);
  static_cast<void>(label_e);

  pos_params.update_all_unstruct(w, gold_class, 1, NODEG);
  neg_params.update_all_unstruct(w, gold_class, -iter, NODEG);

  pos_params.update_all_unstruct(w, sys_class, -1, NODEG);
  neg_params.update_all_unstruct(w, sys_class, iter, NODEG);

  //delete lemma_feature_word;
}

void PerceptronTrainer::set_avg_params(void)
{
  pt = pos_params;
  
  ParamMap::iterator pt_it = pt.get_unstruct_begin();
  ParamMap::iterator pos_it = pos_params.get_unstruct_begin();
  ParamMap::iterator neg_it = neg_params.get_unstruct_begin();

  ParamMap::iterator pt_end = pt.get_unstruct_end();
  ParamMap::iterator pos_end = pos_params.get_unstruct_end();
  ParamMap::iterator neg_end = neg_params.get_unstruct_end();

  while (pt_it != pt_end)
    {
      assert(pos_it != pos_end);
      assert(neg_it != neg_end);
      pt_it->second = (iter + 1) * pos_it->second + neg_it->second;      
      ++pt_it;
      ++pos_it;
      ++neg_it;
    }

  pt_it = pt.get_struct_begin();
  pos_it = pos_params.get_struct_begin();
  neg_it = neg_params.get_struct_begin();

  pt_end = pt.get_struct_end();
  pos_end = pos_params.get_struct_end();
  neg_end = neg_params.get_struct_end();

  while (pt_it != pt_end)
    {
      pt_it->second = (iter + 1) * pos_it->second + neg_it->second;
      ++pt_it;
      ++pos_it;
      ++neg_it;
    }

  pt.set_label_extractor(label_extractor);
}

#else // TEST_PerceptronTrainer_cc

#include "Data.hh"

class SillyLabelExtractor : public LabelExtractor
{
public:
  SillyLabelExtractor(void):
    LabelExtractor(1)
  {}

  void set_label_candidates(const std::string &word_form, 
			    bool use_label_dict,
			    float mass, 
			    LabelVector &target,
			    int candidate_count) const
  {
    static_cast<void>(use_label_dict);
    static_cast<void>(mass);

    if (word_form == BOUNDARY_WF)
      { 
	target.push_back(get_boundary_label());
      }
    else
      {
	target.clear();
	
	for (unsigned int i = 0; i < static_cast<unsigned int>(candidate_count); ++i)
	  {
	    target.push_back(i);
	  }
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

#include <sstream>
#include <cassert>

#include "Trellis.hh"

int main(void)
{
  TaggerOptions options;

  std::string contents("\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN\t_\n"
		       ".\tWORD=.\t.\t.\t_\n"
		       "\n"
		       "\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN\t_\n"
		       ".\tWORD=.\t.\t.\t_\n");

  std::istringstream in(contents);

  SillyLabelExtractor label_extractor;

  ParamTable pt;

  Data train_data(in, 1, label_extractor, pt, 2);
  train_data.set_label_guesses(label_extractor, 
			       0,
			       0,
			       label_extractor.label_count());

  Data dev_data(train_data);
  Data dev_data_copy(dev_data);

  dev_data.set_label_guesses(label_extractor,
			     0,
			     0,
			     label_extractor.label_count());

  dev_data_copy.unset_label();

  dev_data_copy.set_label_guesses(label_extractor, 
				  0, 
				  0,
				  label_extractor.label_count());

  std::ostringstream null_out;
  PerceptronTrainer trainer(5,3,pt, label_extractor, SillyLemmaExtractor(), null_out, options);
  trainer.train(train_data, dev_data);

  for (unsigned int i = 0; i < dev_data_copy.size(); ++i)
    {
      Trellis trellis(dev_data_copy.at(i), label_extractor.get_boundary_label(), NODEG, SECOND);
      trellis.set_maximum_a_posteriori_assignment(pt);      
    }

  assert(dev_data.get_acc(dev_data_copy, SillyLemmaExtractor()).label_acc == 1);
}

#endif // TEST_PerceptronTrainer_cc
