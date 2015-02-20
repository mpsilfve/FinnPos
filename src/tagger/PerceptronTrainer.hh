/**
 * @file    PerceptronTrainer.hh                                             
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

#ifndef HEADER_PerceptronTrainer_hh
#define HEADER_PerceptronTrainer_hh


#include <iostream>

#include "Trainer.hh"
#include "ParamTable.hh"
#include "Sentence.hh"
#include "Data.hh"
#include "exceptions.hh"

class PerceptronTrainer : public Trainer
{
public:
  PerceptronTrainer(unsigned int max_passes,
		    unsigned int max_useless_passes,
		    ParamTable &pt,
		    const LabelExtractor &label_e,
		    const LemmaExtractor &le,
		    std::ostream &msg_out);
  
  void train(const Data &train_data, const Data &dev_data, 
	     unsigned int beam = -1,
	     float beam_mass = -1);

  void train_lemmatizer(const Data &train_data, 
			const Data &dev_data,
			LemmaExtractor &lemma_e,
			const LabelExtractor &label_e);

private:
  float iter;

  ParamTable pos_params;
  ParamTable neg_params;

  void update(const Sentence &gold_s, 
	      const Sentence &sys_s);

  void lemmatizer_update(const Word &w, 
			 unsigned int sys_class, 
			 unsigned int gold_class,
			 LemmaExtractor &lemma_e,
			 const LabelExtractor &label_e);

  void set_avg_params(void);
};

#endif // HEADER_PerceptronTrainer_hh
