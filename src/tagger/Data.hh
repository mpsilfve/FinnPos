/**
 * @file    Data.hh                                                          
 * @Author  Miikka Silfverberg                                               
 * @brief   Container for Sentence-objects.                                  
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

#ifndef HEADER_Data_hh
#define HEADER_Data_hh

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "Sentence.hh"
#include "LabelExtractor.hh"
#include "ParamTable.hh"

typedef std::vector<Sentence> SentenceVector;

#include "process_aux.hh"
using finnposaux::StringPairVector;

struct Acc
{
  float label_acc;
  float iv_label_acc;
  float oov_label_acc;
  float lemma_acc;
  float iv_lemma_acc;
  float oov_lemma_acc;
};

struct IncompatibleData
{};

/*

  Input data format:

  Example:

    These	WORD=The SF=e	the	DT	_
    dogs	WORD=dog SF=g	dog	NN	_
    sleep	WORD=sleep SF=p	sleep	VBN	_

    Those	WORD=Those SF=e	those	PRON	_
    don't	WORD=don't SF=t	do	VBN	_

  Each line should be empty or should have the form:

  "TOKEN\tFEAT1 ... FEATm\tLEMMA\tLABEL1 ... LABELn\tANNOTATIONS"

  TOKEN is a non-empty word form, punctuation or such.
  FEATi is a non-empty features template name
  LEMMA can be a word or "_" for empty.
  LABELi can be a word or "_" for empty.
  ANNOTATONS are additional annotations or "_" for empty.

  None of the fields can contain tabs. Moreover, FEATi and LABELi
  can't contain spaces.

  Two sentences are separated by an empty line.
 */

class Data
{
 public:
  static bool silent;
  
  Data(void);
  Data(const std::string &fn, bool is_gold, LabelExtractor &extractor, ParamTable &pt, unsigned int degree);
  Data(std::istream &in, bool is_gold, LabelExtractor &extractor, ParamTable &pt, unsigned int degree);

  const Sentence &at(unsigned int i) const;
  Sentence &at(unsigned int i);

  void push_back(const Sentence &s);

  unsigned int size(void) const;
  void clear_label_guesses(void);
  void set_label_guesses(const LabelExtractor &g, 
			 bool use_label_dict,
			 float mass,
			 int candidate_count = -1);

  void predict_lemma(LemmaExtractor &g, const LabelExtractor &e);

  void unset_lemma(void);
  void unset_label(void);

  Acc get_acc(const Data &other, const LemmaExtractor &lemma_extractor) const;

  void print(std::ostream &out, LabelExtractor &label_extractor);

  void randomize(void);

  void parse_aux_data(StringPairVector &p) const;

 private:
  SentenceVector data;

  void init(const std::string &fn, std::istream &in, bool is_gold, LabelExtractor &extractor, ParamTable &pt, unsigned int degree);
};


#endif // HEADER_Data_hh
