/**
 * @file    finnpos-train.cc                                                 
 * @Author  Miikka Silfverberg                                               
 * @brief   Train a Tagger and store it.                                     
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
#include <cstdlib>
#include <string>
#include <fstream>
#include <cassert>

#include "io.hh"
#include "Tagger.hh"

int main(int argc, char * argv[])
{
  if (argc != 5)
    {
      std::cerr <<  "USAGE: " << argv[0] 
		<< " config_file train_file dev_file output_file"
		<< std::endl;

      exit(1);
    }

  std::string config_fn = argv[1];  
  std::string train_fn  = argv[2];  
  std::string dev_fn    = argv[3];  
  std::string output_fn = argv[4]; 

  std::ifstream config_in(config_fn.c_str());
  std::ifstream train_in(train_fn.c_str());
  std::ifstream dev_in(dev_fn.c_str());

  std::ofstream out(output_fn);

  if (not check(config_fn, config_in, std::cerr) or
      not check(train_fn,   train_in, std::cerr) or
      not check(dev_fn,       dev_in, std::cerr) or
      not check(output_fn,       out, std::cerr))
    { exit(1); }

  Tagger tagger(config_in, std::cerr);
  tagger.train(train_in, dev_in);
  tagger.store(out);
}
