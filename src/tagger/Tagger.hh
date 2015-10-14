/**
 * @file    Tagger.hh                                                        
 * @Author  Miikka Silfverberg                                               
 * @brief   Class that can label a Sentence.                                  
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

#ifndef HEADER_Tagger_hh
#define HEADER_Tagger_hh

#include <iostream>
#include <exception>

#include "Sentence.hh"
#include "Data.hh"
#include "ParamTable.hh"
#include "LabelExtractor.hh"
#include "LemmaExtractor.hh"
#include "TaggerOptions.hh"

struct NotImplemented : public std::exception
{};

class Tagger
{
public:
  Tagger(std::ostream &msg_out);

  Tagger(const TaggerOptions &tagger_options,
	 std::ostream &msg_out);

  Tagger(std::istream &tagger_opt_in, std::ostream &msg_out);

  void train(std::istream &train_in,
	     std::istream &dev_in);

  void label(std::istream &in);
  void label_stream(std::istream &in);

  void store(std::ostream &out) const;
  void load(std::istream &in);

  void evaluate(std::istream &in);
  bool operator==(const Tagger &another) const;
  LabelExtractor &get_label_extractor(void);
  LemmaExtractor &get_lemma_extractor(void);

  Data get_data(const std::string &filename, bool tagged);

  void p(void) const;

  void print_params(std::ostream &out) const;

  void set_options(const TaggerOptions &tagger_options);

private:
  unsigned int line_counter;

  TaggerOptions tagger_options;

  LabelExtractor label_extractor;
  LemmaExtractor lemma_extractor;

  ParamTable param_table;

  std::ostream &msg_out;

  StringVector labels_to_strings(const LabelVector &v);
};

#endif // HEADER_Tagger_hh
