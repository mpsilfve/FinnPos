/**
 * @file    finnpos-filter-params.cc                                                 
 * @Author  Miikka Silfverberg                                               
 * @brief   Use a tagger to label data.                                     
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

#include "io.hh"
#include "Tagger.hh"

int main(int argc, char * argv[])
{
  if (argc != 4)
    {
      std::cerr <<  "USAGE: " << argv[0] << "config_file model_file model_output_file"
		<< std::endl;

      exit(1);
    }

  std::string config_fn = argv[1];
  std::ifstream config_in(config_fn.c_str());

  std::string model_fn = argv[2];  
  std::ifstream model_in(model_fn.c_str());

  if (not check(model_fn, model_in, std::cerr))
    { exit(1); }

  std::cerr << argv[0] << ": Reading config file." << std::endl;
  unsigned int line_counter;
  TaggerOptions options(config_in, line_counter);
  
  std::cerr << argv[0] << ": Loading tagger." << std::endl;

  Tagger tagger(std::cerr);
  tagger.load(model_in);

  if (options.filter_type == AVG_VALUE)
    { tagger.set_param_filter(options); }
  else
    { std::cerr << argv[0] 
		<< ": Warning! Parameter filter is not set to AVG_VALUE."
		<< std::endl
		<< "           No filtering will happen."
		<< std::endl; }

  std::cerr << argv[0] << ": Storing model." << std::endl;
  std::ofstream model_out(argv[3]);
  tagger.store(model_out);
}
