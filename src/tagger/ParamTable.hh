/**
 * @file    ParamTable.hh                                                
 * @Author  Miikka Silfverberg                                               
 * @brief   Parameter table for structured and unstructured parameters. Used 
 *          by both Tagger and LemmaExtractor.                               
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

#ifndef HEADER_ParamTable_hh
#define HEADER_ParamTable_hh

//#include <unordered_map>
#include "UnorderedMapSet.hh"
#include <vector>
#include <string>

#include "io.hh"

typedef std::vector<unsigned int> FeatureTemplateVector;

const long MAX_LABEL = 50000;

class Word;
class LabelExtractor;

typedef std::unordered_map<long, float> ParamMap;

class ParamTable
{
public:
  ParamTable(void);
  virtual ~ParamTable(void);
  ParamTable &operator=(const ParamTable &another);

  unsigned int get_feat_template(const std::string &feat_template_string);
  FeatureTemplateVector get_feat_templates(StringVector &feat_template_strings);

  float get_unstruct(unsigned int feature_template, unsigned int label) const;
  float get_struct1(unsigned int label, bool sub_labels) const;
  float get_struct2(unsigned int plabel, unsigned int label, bool sub_labels) const;
  float get_struct3(unsigned int pplabel, unsigned int plabel, unsigned int label) const;

  float get_all_struct_fw(unsigned int pplabel, unsigned int plabel, unsigned int label, bool sub_labels = 1) const;
  float get_all_struct_bw(unsigned int pplabel, unsigned int plabel, unsigned int label, bool sub_labels = 1) const;
  float get_all_unstruct(const Word &word, unsigned int label, bool sub_labels = 1) const;

  void update_all_struct_fw(unsigned int pplabel, unsigned int plabel, unsigned int label, float update, bool sub_labels = 1);
  void update_all_struct_bw(unsigned int pplabel, unsigned int plabel, unsigned int label, float update, bool sub_labels = 1);
  void update_all_unstruct(const Word &word, unsigned int label, float update, bool sub_labels = 1);

  void update_unstruct(unsigned int feature_template, unsigned int label, float ud);
  void update_struct1(unsigned int label, float ud, bool sub_labels);
  void update_struct2(unsigned int plabel, unsigned int label, float ud, bool sub_labels);
  void update_struct3(unsigned int pplabel, unsigned int plabel, unsigned int label, float ud);

  ParamMap::iterator get_unstruct_begin(void);
  ParamMap::iterator get_struct_begin(void);
  ParamMap::iterator get_unstruct_end(void);
  ParamMap::iterator get_struct_end(void);

  void set_trained(void);

  void store(std::ostream &out) const;
  void load(std::istream &in, bool reverse_bytes);
  bool operator==(const ParamTable &another) const;

  void set_label_extractor(const LabelExtractor &label_extractor);

  void next(void);

  void p(void) const;

private:
  typedef std::unordered_map<std::string, unsigned int> FeatureTemplateMap;

  const LabelExtractor * label_extractor;
  bool trained;
  FeatureTemplateMap feature_template_map;
  ParamMap unstruct_param_table;
  ParamMap struct_param_table;

  long get_unstruct_param_id(unsigned int feature_template, unsigned int label) const;
  long get_struct_param_id(unsigned int label) const;
  long get_struct_param_id(unsigned int plabel, unsigned int label) const;
  long get_struct_param_id(unsigned int pplabel, unsigned int plabel, unsigned int label) const;
};

#endif // HEADER_ParamTable_hh
