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

#include "Trainer.hh"

#ifndef TEST_Trainer_cc

Trainer::Trainer(unsigned int max_passes,
		 unsigned int max_useless_passes,
		 ParamTable &pt,
		 const LabelExtractor &label_extractor,
		 const LemmaExtractor &lemma_extractor,
		 std::ostream &msg_out):
  max_passes(max_passes),
  max_useless_passes(max_useless_passes),
  pt(pt),
  label_extractor(label_extractor),
  lemma_extractor(lemma_extractor),
  boundary_label(label_extractor.get_boundary_label()),
  msg_out(msg_out)
{}

#else // TEST_Trainer_cc

int main (void)
{

}

#endif // TEST_Trainer_cc
