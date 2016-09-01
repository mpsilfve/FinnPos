/**
 * @file    SGDTrainer.hh                                                    
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

#ifndef HEADER_SGDTrainer_hh
#define HEADER_SGDTrainer_hh

#include "Trainer.hh"
#include "Trellis.hh"

class SGDTrainer : public Trainer
{
public:
  SGDTrainer(unsigned int max_passes,
	     unsigned int max_useless_passes,
	     ParamTable &pt,
	     const LabelExtractor &label_e,
	     const LemmaExtractor &le,
	     std::ostream &msg_out,
	     const TaggerOptions &options);
  
  void train(const Data &train_data, const Data &dev_data, 
	     unsigned int beam = -1,
	     float beam_mass = -1);

private:
  ParamTable pos_params;
  unsigned int iter;
  Degree sublabel_order;
  Degree model_order;
  const TaggerOptions &options;
  float delta;
  float sigma;
  Word bw;

  void update(const Sentence &gold_s, 
	      const Sentence &sys_s,
	      const Trellis &trellis);

};

#endif // HEADER_SGDTrainer_hh
