/**
 * @file    Trainer.hh                                                       
 * @Author  Miikka Silfverberg                                               
 * @brief   Base class for parameter estimation classes.                     
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

#ifndef HEADER_Trainer_hh
#define HEADER_Trainer_hh

#include <iostream>

#include "Trainer.hh"
#include "Data.hh"
#include "ParamTable.hh"

class Trainer
{
public:
  Trainer(unsigned int max_passes,
	  unsigned int max_useless_passes,
	  ParamTable &pt,
	  const LabelExtractor &label_extractor,
	  const LemmaExtractor &lemma_extractor,
	  std::ostream &msg_out);
  
  virtual void train(const Data &train_data, 
		     const Data &dev_data,
		     unsigned int beam,
		     float beam_mass) = 0;

  void train_lemmatizer(const Data &train_data, 
			const Data &dev_data,
			LemmaExtractor &lemma_e,
			const LabelExtractor &label_e);
  
protected:
  unsigned int max_passes;
  unsigned int max_useless_passes;

  ParamTable &pt;

  const LabelExtractor &label_extractor;
  const LemmaExtractor &lemma_extractor;

  unsigned int boundary_label;

  std::ostream &msg_out;
};

#endif // HEADER_Trainer_hh
