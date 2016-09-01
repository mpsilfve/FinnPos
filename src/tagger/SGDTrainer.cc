/**
 * @file    SGDTrainer.cc                                                    
 * @Author  Miikka Silfverberg                                               
 * @brief   An instance of Trainer that performs SGD.                        
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

#include "SGDTrainer.hh"

#ifndef TEST_SGDTrainer_cc


#define STRUCT_SL 1
#define USTRUCT_SL 1

SGDTrainer::SGDTrainer(unsigned int max_passes,
		       unsigned int max_useless_passes,
		       ParamTable &pt,
		       const LabelExtractor &label_extractor,
		       const LemmaExtractor &lemma_extractor,
		       std::ostream &msg_out,
		       const TaggerOptions &options):  
  Trainer(max_passes, max_useless_passes, pt, label_extractor, lemma_extractor, msg_out, options),
  iter(0), 
  sublabel_order(options.sublabel_order),
  model_order(options.model_order),
  options(options),
  delta(options.delta),
  sigma(options.sigma),
  bw(label_extractor.get_boundary_label())
{
  std::cerr << delta << std::endl;
  pt.set_param_filter(options);
  pos_params = pt;

  pos_params.set_label_extractor(label_extractor);
}

void SGDTrainer::train(const Data &train_data, 
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
  best_params.set_param_filter(options);
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
	  //	  train_trellises[j]->set_maximum_a_posteriori_assignment
	  //  (pos_params);
	  train_trellises[j]->set_marginals(pos_params);

	  update(train_data_copy.at(j), train_data.at(j), *train_trellises[j]);
	  
	  std::cerr << j << " of " << train_trellises.size() - 1 << "\r";
	}

      std::cerr << std::endl;

      // Tag dev data.
      for (unsigned int j = 0; j < dev_trellises.size(); ++j)
	{
	  dev_trellises[j]->set_maximum_a_posteriori_assignment(pos_params);
	}

      float acc = dev_data.get_acc(dev_data_copy, lemma_extractor).label_acc;

      msg_out << "    Dev acc: " << acc * 100.00 << "%" << std::endl;

      if (acc > best_dev_acc)
	{
	  useless_passes = 0;
	  best_dev_acc = acc;
	  best_params = pos_params;
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
  pt.set_train_iters(iter);

  for (TrellisVector::const_iterator it = train_trellises.begin();
       it != train_trellises.end();
       ++it)
    { delete *it; }

  for (TrellisVector::const_iterator it = dev_trellises.begin();
       it != dev_trellises.end();
       ++it)
    { delete *it; }
}

void SGDTrainer::update(const Sentence &gold_s,
			const Sentence &sys_s,
			const Trellis &trellis) 
{
  ++iter;

  for (unsigned int i = 0; i < sys_s.size() ; ++i)
    {
      unsigned int gold_label = gold_s.at(i).get_label();
      unsigned int pgold_label = (i < 1 ? boundary_label : gold_s.at(i - 1).get_label());
      unsigned int ppgold_label = (i < 2 ? boundary_label : gold_s.at(i - 2).get_label());

      pos_params.update_all_unstruct(gold_s.at(i), gold_label, delta, sublabel_order);
      pos_params.update_all_struct_fw(ppgold_label, pgold_label, gold_label, delta, sublabel_order, model_order);

      const Word * word = &(sys_s.at(i));
      const Word * pword = &(i < 1 ? bw : sys_s.at(i - 1));
      const Word * ppword = &(i < 2 ? bw : sys_s.at(i - 2));

      for (unsigned int j = 0; j < word->get_label_count(); ++j)
	{
	  unsigned int j_label = word->get_label(j);
	  
	  float ug_marginal = trellis.get_marginal(i, j);

	  pos_params.update_all_unstruct(sys_s.at(i), j_label, -ug_marginal*delta, sublabel_order);
	  pos_params.update_struct1(j_label, -ug_marginal*delta, sublabel_order);

	  pos_params.regularize_all_unstruct(sys_s.at(i), j_label, sigma, sublabel_order);
	  pos_params.regularize_struct1(j_label, sigma, sublabel_order);

	  for (unsigned int k = 0; k < pword->get_label_count(); ++k)
	    {
	      unsigned int k_label = pword->get_label(k);

	      float bg_marginal = 0;	      
	      if (i < 1)
		{ bg_marginal = ug_marginal; }
	      else
		{ bg_marginal = trellis.get_marginal(i, k, j); }
	      //	      std::cerr << bg_marginal << std::endl;
	      pos_params.update_struct2(k_label, j_label, -bg_marginal*delta, sublabel_order);
	      pos_params.regularize_struct2(k_label, j_label, sigma, sublabel_order);

	      for (unsigned int l = 0; l < ppword->get_label_count(); ++l)
		{
		  unsigned int l_label = ppword->get_label(l);

		  float tg_marginal = 0;
		  if (i < 2)
		    { tg_marginal = bg_marginal; }
		  else
		    { 
		      tg_marginal = trellis.get_marginal(i, l, k, j); 
		    }
		  //std::cerr << tg_marginal << std::endl;
		  pos_params.update_struct3(l_label, k_label, j_label, -tg_marginal*delta, sublabel_order);
		  pos_params.regularize_struct3(l_label, k_label, j_label, sigma, sublabel_order);
		}
	    }
	}
    }
}


#else // TEST_SGDTrainer_cc

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

#include <cassert>

int main(void)
{
  TaggerOptions options;
  options.delta = 0.1;
  options.sigma = 0.01;

  std::string contents("\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN\t_\n"
		       ".\tWORD=.\t.\t.\t_\n"		   
		       "\n"
		       "\n"
		       "The\tWORD=The\tthe\tDT\t_\n"
		       "dog\tWORD=dog\tdog\tNN\t_\n"
		       "sleeps\tWORD=sleeps\tsleep\tVB\t_\n"
		       "but\tWORD=but\tbut\tCC\t_\n"
		       "they\tWORD=they\tthey\tPNP\t_\n"
		       "dog\tWORD=dog\tdog\tVB\t_\n"
		       "me\tWORD=me\tme\tPNP\t_\n"
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
  SillyLemmaExtractor sle;
  SGDTrainer trainer(5,3,pt, label_extractor, sle, null_out, options);

  std::cerr << "Initial label acc: " << dev_data.get_acc(dev_data_copy, SillyLemmaExtractor()).label_acc << std::endl;

  trainer.train(train_data, dev_data);

  for (unsigned int i = 0; i < dev_data_copy.size(); ++i)
    {
      Trellis trellis(dev_data_copy.at(i), label_extractor.get_boundary_label(), NODEG, SECOND);
      trellis.set_maximum_a_posteriori_assignment(pt);      
    }

  std::cerr << "Final label acc: " << dev_data.get_acc(dev_data_copy, SillyLemmaExtractor()).label_acc << std::endl;

  std::cerr << pt << std::endl;
  assert(dev_data.get_acc(dev_data_copy, SillyLemmaExtractor()).label_acc == 1);
}
#endif // TEST_SGDTrainer_cc
