/**
 * @file    finnpos-label.cc                                                 
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
#include <cstdlib>
#include <string>
#include <fstream>

#include "io.hh"
#include "Tagger.hh"

int main(int argc, char * argv[])
{
  if (argc != 2)
    {
      std::cerr <<  "USAGE: " << argv[0] << " model_file"
		<< std::endl;

      exit(1);
    }

  std::string model_fn = argv[1];  
  std::ifstream model_in(model_fn.c_str());

  if (not check(model_fn, model_in, std::cerr))
    { exit(1); }

  std::cerr << argv[0] << ": Loading tagger." << std::endl;

  Tagger tagger(std::cerr);
  tagger.load(model_in);

  std::cerr 
    << argv[0]
    << ": Reading from STDIN. Writing to STDOUT." 
    << std::endl;
  
  tagger.label_stream(std::cin);
}
