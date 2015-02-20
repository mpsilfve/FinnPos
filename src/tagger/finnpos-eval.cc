/**
 * @file    finnpos-eval.cc                       
 * @Author  Miikka Silfverberg                                               
 * @brief   Compare two tagged files.
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

#include <iostream>

#include "Data.hh"
#include "ParamTable.hh"
#include "Tagger.hh"

int main(int argc, char * argv[])
{
  if (argc != 3 or argc != 4)
    {
      std::cerr << "USAGE: " << argv[0] << " sys_tagged_file gold_tagged_file "
		<< "[ model ]"
		<< std::endl;
    }

  bool read_model = 0;

  if (argc == 4)
    { read_model = 1; }

  Acc acc;
  
  // If a model is read, we can use its param_table, label_extractor
  // and lemma_extractor to get better statistics (broken down into
  // statistics for IV and OOV words). Otherwise, use empty variants.
  Tagger tagger(std::cerr);      
  ParamTable param_table;
  LabelExtractor label_extractor;
  LemmaExtractor lemma_extractor;


  Data gold_data;
  Data sys_data;

  if (read_model)
    {
      std::string model_file_name = argv[3];

      std::ifstream model_in(model_file_name);

      if (not check(model_file_name, model_in, std::cerr))
	{ exit(1); }
      
      std::ifstream tagger_in(argv[3]);
      tagger.load(tagger_in);

      Data sys_data  = tagger.get_data(argv[1], 1);
      Data gold_data = tagger.get_data(argv[2], 1);

      acc = gold_data.get_acc(sys_data, tagger.get_lemma_extractor());
    }
  else
    {
      Data sys_data(argv[1], 1, label_extractor, param_table, 2);
      Data gold_data(argv[2], 1, label_extractor, param_table, 2);

      acc = gold_data.get_acc(sys_data, lemma_extractor);
    }

  std::cout << "Comparing " << argv[1] << " and " << argv[2] 
	    << " (gold standard)." << std::endl;
  std::cout << "Label accuracy for all words: " << acc.label_acc << std::endl;
  std::cout << "Label accuracy for IV words:  " 
	    << (read_model ? std::to_string(acc.iv_label_acc) : "-") 
	    << std::endl;
  std::cout << "Label accuracy for OOV words: " 
	    << (read_model ? std::to_string(acc.oov_label_acc) : "-") 
	    << std::endl;
  std::cout << "Lemma accuracy for all words: " << acc.lemma_acc << std::endl;
  std::cout << "Lemma accuracy for IV words:  " 
	    << (read_model ? std::to_string(acc.iv_lemma_acc) : "-") 
	    << std::endl;
  std::cout << "Lemma accuracy for OOV words: " 
	    << (read_model ? std::to_string(acc.oov_lemma_acc) : "-") 
	    << std::endl;

}
