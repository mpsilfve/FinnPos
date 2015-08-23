/**
 * @file    ParamTable.cc                                                
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

#include "ParamTable.hh"

#ifndef TEST_ParamTable_cc

#include "Word.hh"

ParamTable::ParamTable(void):
  label_extractor(0),
  trained(0)
{}

ParamTable::~ParamTable(void)
{}

ParamTable &ParamTable::operator=(const ParamTable &another)
{
  if (this == &another)
    { return *this; }
  
  trained              = another.trained;
  feature_template_map = another.feature_template_map;
  unstruct_param_table = another.unstruct_param_table;
  struct_param_table   = another.struct_param_table;
  return *this;
}

void ParamTable::set_trained(void)
{
  trained = 1;
}

unsigned int ParamTable::get_feat_template
(const std::string &feat_template_string)
{
  if (feature_template_map.find(feat_template_string) == 
      feature_template_map.end())
    { 
      feature_template_map[feat_template_string] = feature_template_map.size();
    }

  return feature_template_map[feat_template_string];
}

FeatureTemplateVector ParamTable::get_feat_templates
(StringVector &feat_template_strings)
{
  FeatureTemplateVector feat_templates;

  for (unsigned int i = 0; i < feat_template_strings.size(); ++i)
    {
      if (trained and feature_template_map.count(feat_template_strings[i]) == 0)
	{ 
	  continue; 
	}
     
      feat_templates.push_back(get_feat_template(feat_template_strings[i]));
    }

  return feat_templates;
}

long ParamTable::get_unstruct_param_id(unsigned int feature_template, 
				       unsigned int label) const
{
  return (MAX_LABEL + 1) * feature_template + label;
}

void ParamTable::set_label_extractor(const LabelExtractor &le)
{   
  this->label_extractor = &le; 
}

long ParamTable::get_struct_param_id(unsigned int label) const
{
  return 
    (MAX_LABEL + 1) * (MAX_LABEL + 1) * (MAX_LABEL + 1) +
    (MAX_LABEL + 1) * (MAX_LABEL + 1) +
    label;
}

long ParamTable::get_struct_param_id(unsigned int plabel, unsigned int label) 
  const
{
  return 
    (MAX_LABEL + 1) * (MAX_LABEL + 1) * (MAX_LABEL + 1) +
    plabel * (MAX_LABEL + 1) +
    label;
}

long ParamTable::get_struct_param_id(unsigned int pplabel, 
				     unsigned int plabel, 
				     unsigned int label) const
{
  return 
    pplabel * (MAX_LABEL + 1) * (MAX_LABEL + 1) +
    plabel * (MAX_LABEL + 1) +
    label;
}

std::string ParamTable::get_unstruct_feat_repr(long feat_id,
					       const InvFeatureTemplateMap &m)
  const
{
  long label = feat_id % (MAX_LABEL + 1);
  long feat_template = (feat_id - label) / (MAX_LABEL + 1);
  std::string label_string = label_extractor->get_label_string(label);
  std::string feat_template_string = m[feat_template];

  return feat_template_string + " " + label_string;
}

std::string ParamTable::get_struct_feat_repr(long feat_id) const
{
  if (feat_id >= 
      (MAX_LABEL + 1) * (MAX_LABEL + 1) * (MAX_LABEL + 1) +
      (MAX_LABEL + 1) * (MAX_LABEL + 1))
    {
      feat_id -= 
	(MAX_LABEL + 1) * (MAX_LABEL + 1) * (MAX_LABEL + 1) +
	(MAX_LABEL + 1) * (MAX_LABEL + 1);

      return label_extractor->get_label_string(feat_id);
    }
  else if (feat_id >= (MAX_LABEL + 1) * (MAX_LABEL + 1) * (MAX_LABEL + 1))
    {
      feat_id -= (MAX_LABEL + 1) * (MAX_LABEL + 1) * (MAX_LABEL + 1);
      long label1 = feat_id % (MAX_LABEL + 1);
      feat_id -= label1;
      long label2 = feat_id / (MAX_LABEL + 1);
      
      return label_extractor->get_label_string(label1) + " " +
	label_extractor->get_label_string(label2);
    }
  else
    {
      long label1 = feat_id % (MAX_LABEL + 1);
      feat_id -= label1;
      feat_id /= (MAX_LABEL + 1);

      long label2 = feat_id % (MAX_LABEL + 1);
      feat_id -= label2;
      
      long label3 = feat_id / (MAX_LABEL + 1);

      return label_extractor->get_label_string(label1) + " " +
	label_extractor->get_label_string(label2) + " " + 
	label_extractor->get_label_string(label3);
    }
}

float ParamTable::get_unstruct(unsigned int feature_template, 
			       unsigned int label) const
{
  long id = get_unstruct_param_id(feature_template, label);
  
  ParamMap::const_iterator it = unstruct_param_table.find(id);

  if (it == unstruct_param_table.end())
    { return 0; }

  return it->second;
}

float ParamTable::get_struct1(unsigned int label, bool sub_labels) const
{
  long id = get_struct_param_id(label);
  
  ParamMap::const_iterator it = struct_param_table.find(id);

  if (it == struct_param_table.end())
    { return 0; }

  float res = it->second;

  if (label_extractor != 0 and sub_labels)
    {
      const LabelVector &sub_labels = label_extractor->sub_labels(label);

      for (unsigned int j = 0; j < sub_labels.size(); ++j)
	{
	  id = get_struct_param_id(sub_labels[j]);	      
	  it = struct_param_table.find(id);

	  if (it != struct_param_table.end())
	    { res += it->second; };
	}
    }

  return res;
}

float ParamTable::get_struct2(unsigned int plabel, unsigned int label, bool use_sub_labels) const
{
  long id = get_struct_param_id(plabel, label);
  
  ParamMap::const_iterator it = struct_param_table.find(id);

  float res = 0;

  if (it != struct_param_table.end())
    { res += it->second; }

  if (label_extractor != 0 and use_sub_labels)
    {
      const LabelVector &sub_labels = label_extractor->sub_labels(label);
      const LabelVector &psub_labels = label_extractor->sub_labels(plabel);

      for (unsigned int i = 0; i < psub_labels.size(); ++i)
	{
	  for (unsigned int j = 0; j < sub_labels.size(); ++j)
	    {
	      id = get_struct_param_id(psub_labels[i], sub_labels[j]);	      
	      it = struct_param_table.find(id);

	      if (it != struct_param_table.end())
		{ res += it->second; };
	    }
	}
    }

  return res;
}

float ParamTable::get_struct3(unsigned int pplabel,
			      unsigned int plabel, 
			      unsigned int label) const
{
  long id = get_struct_param_id(pplabel, plabel, label);
  
  ParamMap::const_iterator it = struct_param_table.find(id);

  if (it == struct_param_table.end())
    { return 0; }

  return it->second;
}

float ParamTable::get_all_unstruct(const Word &word, unsigned int label, bool sub_labels) const
{
  float res = 0;

  for (unsigned int i = 0; i < word.get_feature_template_count(); ++i)
    {
      res += get_unstruct(word.get_feature_template(i), label);
    }

  if (sub_labels and label_extractor != 0)
    {
      const LabelVector &sub_labels = label_extractor->sub_labels(label);
      
      for (unsigned int i = 0; i < word.get_feature_template_count(); ++i)
	{
	  for (unsigned int j = 0; j < sub_labels.size(); ++j)
	    {
	      res += get_unstruct(word.get_feature_template(i), sub_labels[j]);
	    }
	}
    }

  return res;
}

float ParamTable::get_all_struct_fw(unsigned int pplabel, 
				    unsigned int plabel, 
				    unsigned int label,
				    bool sub_labels) const
{
  return 
    get_struct3(pplabel, plabel, label) +
    get_struct2(plabel, label, sub_labels) +
    get_struct1(label, sub_labels);
}

float ParamTable::get_all_struct_bw(unsigned int pplabel, 
				    unsigned int plabel, 
				    unsigned int label,
				    bool sub_labels) const
{
  return 
    get_struct3(pplabel, plabel, label) +
    get_struct2(plabel, label, sub_labels) +
    get_struct1(label, sub_labels);
}

void ParamTable::update_unstruct(unsigned int feature_template, 
				 unsigned int label, 
				 float ud)
{
  size_t id = get_unstruct_param_id(feature_template, label);
  
  if (unstruct_param_table.count(id) == 0)
    { unstruct_param_table[id] = 0; }

  unstruct_param_table[get_unstruct_param_id(feature_template, label)] += ud;
}

void ParamTable::update_struct1(unsigned int label, float ud, bool use_sub_labels)
{
  size_t id = get_struct_param_id(label);
  
  if (struct_param_table.count(id) == 0)
    { struct_param_table[id] = 0; }

  struct_param_table[get_struct_param_id(label)] += ud;

  if (label_extractor != 0 and use_sub_labels)
    {
      const LabelVector &sub_labels = label_extractor->sub_labels(label);

      for (unsigned int i = 0; i < sub_labels.size(); ++i)
	{
	  size_t id = get_struct_param_id(sub_labels[i]);
  
	  if (struct_param_table.count(id) == 0)
	    { struct_param_table[id] = 0; }

	  struct_param_table[get_struct_param_id(sub_labels[i])] += ud;
	}
    }
}

void ParamTable::update_struct2(unsigned int plabel, 
				unsigned int label, 
				float ud,
				bool use_sub_labels)
{
  size_t id = get_struct_param_id(plabel, label);
  
  if (struct_param_table.count(id) == 0)
    { struct_param_table[id] = 0; }

  struct_param_table[get_struct_param_id(plabel, label)] += ud;

  if (label_extractor != 0 and use_sub_labels)
    {
      const LabelVector &sub_labels = label_extractor->sub_labels(label);
      const LabelVector &psub_labels = label_extractor->sub_labels(plabel);

      for (unsigned int i = 0; i < psub_labels.size(); ++i)
	{
	  for (unsigned int j = 0; j < sub_labels.size(); ++j)
	    {
	      size_t id = get_struct_param_id(psub_labels[i], sub_labels[j]);
	      
	      if (struct_param_table.count(id) == 0)
		{ struct_param_table[id] = 0; }

	      struct_param_table[get_struct_param_id(psub_labels[i], 
						     sub_labels[j])] += ud;
	    }
	}
    }
}

void ParamTable::update_struct3(unsigned int pplabel, 
			       unsigned int plabel, 
			       unsigned int label, 
			       float ud)
{
  size_t id = get_struct_param_id(pplabel, plabel, label);
  
  if (struct_param_table.count(id) == 0)
    { struct_param_table[id] = 0; }

  struct_param_table[get_struct_param_id(pplabel, plabel, label)] += ud;
}

void ParamTable::update_all_struct_fw(unsigned int pplabel, unsigned int plabel, unsigned int label, float update, bool sub_labels)
{
  update_struct1(label, update, sub_labels);
  update_struct2(plabel, label, update, sub_labels);
  update_struct3(pplabel, plabel, label, update); 
}

void ParamTable::update_all_struct_bw(unsigned int pplabel, unsigned int plabel, unsigned int label, float update, bool sub_labels)
{
  update_struct3(pplabel, plabel, label, update);
  update_struct2(plabel, label, update, sub_labels);
  update_struct1(label, update, sub_labels);
}

void ParamTable::update_all_unstruct(const Word &word, unsigned int label, float update, bool sub_label)
{
  for (unsigned int i = 0; i < word.get_feature_template_count(); ++i)
    {
      update_unstruct(word.get_feature_template(i), label, update);
    }

  if (label_extractor != 0 and sub_label)
    {
      const LabelVector &sub_labels = label_extractor->sub_labels(label);
      
      for (unsigned int i = 0; i < word.get_feature_template_count(); ++i)
	{
	  for (unsigned int j = 0; j < sub_labels.size(); ++j)
	    {
	      update_unstruct(word.get_feature_template(i), sub_labels[j], update);
	    }
	}
    }
}

ParamMap::iterator ParamTable::get_unstruct_begin(void)
{
  return unstruct_param_table.begin();
}

ParamMap::iterator ParamTable::get_struct_begin(void)
{
  return struct_param_table.begin();
}

ParamMap::iterator ParamTable::get_unstruct_end(void)
{
  return unstruct_param_table.end();
}

ParamMap::iterator ParamTable::get_struct_end(void)
{
  return struct_param_table.end();
}

#include <cassert>

void ParamTable::store(std::ostream &out) const
{
  write_val(out, trained);
  write_map(out, feature_template_map);
  write_map(out, unstruct_param_table);
  write_map(out, struct_param_table);
}

void ParamTable::load(std::istream &in, bool reverse_bytes)
{
  read_val<bool>(in, trained, reverse_bytes);
  read_map(in, feature_template_map, reverse_bytes);
  read_map(in, unstruct_param_table, reverse_bytes);
  read_map(in, struct_param_table, reverse_bytes);
  label_extractor = 0;
}

bool ParamTable::operator==(const ParamTable &another) const
{
  //  if (this == &another)
  //    { return 1; }

  return
    (label_extractor->operator==(*(another.label_extractor)) and
     trained == another.trained and
     feature_template_map == another.feature_template_map and
     unstruct_param_table == another.unstruct_param_table and
     struct_param_table == another.struct_param_table);
}

std::ostream &operator<<(std::ostream &out, const ParamTable &table)
{
  ParamTable::InvFeatureTemplateMap 
    m(table.feature_template_map.size());

  for (ParamTable::FeatureTemplateMap::const_iterator it = 
	 table.feature_template_map.begin();
       it != table.feature_template_map.end();
       ++it)
    { m[it->second] = it->first; }

  out << "UNSTRUCTURED FEATURES" << std::endl;

  for (ParamMap::const_iterator it = 
	 table.unstruct_param_table.begin();
       it != table.unstruct_param_table.end();
       ++it)
    {
      std::string feat_str = table.get_unstruct_feat_repr(it->first, m);
      out << feat_str << ' ' << it->second << std::endl;
    }

  out << "STRUCTURED FEATURES" << std::endl;

  for (ParamMap::const_iterator it = 
	 table.struct_param_table.begin();
       it != table.struct_param_table.end();
       ++it)
    {
      std::string feat_str = table.get_struct_feat_repr(it->first);
      out << feat_str << ' ' << it->second << std::endl;
    }

  return out;
}

#else // TEST_ParamTable_cc

#include <sstream>
#include <cassert>

#include "LabelExtractor.hh"

int main(void)
{
  ParamTable pt;

  LabelExtractor le;
  static_cast<void>(le.get_label("a"));
  pt.set_label_extractor(le);

  unsigned int label_foo = pt.get_feat_template("FOO");
  unsigned int label_bar = pt.get_feat_template("BAR");
  
  assert(label_foo != label_bar);

  assert(label_foo == pt.get_feat_template("FOO"));
  assert(label_bar == pt.get_feat_template("BAR"));

  assert(pt.get_unstruct(pt.get_feat_template("FOO"), 0) == 0);
  pt.update_unstruct(pt.get_feat_template("FOO"), 0, 1);
  assert(pt.get_unstruct(pt.get_feat_template("FOO"), 0) == 1);

  assert(pt.get_unstruct(pt.get_feat_template("BAR"), 0) == 0);
  pt.update_unstruct(pt.get_feat_template("BAR"), 0, 2);
  assert(pt.get_unstruct(pt.get_feat_template("BAR"), 0) == 2);

  assert(pt.get_unstruct(pt.get_feat_template("FOO"), 1) == 0);
  pt.update_unstruct(pt.get_feat_template("FOO"), 1, 3);
  assert(pt.get_unstruct(pt.get_feat_template("FOO"), 1) == 3);

  assert(pt.get_unstruct(pt.get_feat_template("FOO"), 0) == 1);
  pt.update_unstruct(pt.get_feat_template("FOO"), 0, 1);
  assert(pt.get_unstruct(pt.get_feat_template("FOO"), 0) == 2);

  assert(pt.get_struct1(0,0) == 0);
  pt.update_struct1(0, 1,0);
  assert(pt.get_struct1(0, 0) == 1);

  assert(pt.get_struct1(1,0) == 0);
  pt.update_struct1(1, 2,0);
  assert(pt.get_struct1(1,0) == 2);

  assert(pt.get_struct2(0, 0, false) == 0);
  pt.update_struct2(0, 0, 1, false);
  assert(pt.get_struct2(0, 0, false) == 1);

  assert(pt.get_struct2(0, 1, false) == 0);
  pt.update_struct2(0, 1, 2, false);
  assert(pt.get_struct2(0, 1, false) == 2);

  assert(pt.get_struct3(0, 0, 0) == 0);
  pt.update_struct3(0, 0, 0, 1);
  assert(pt.get_struct3(0, 0, 0) == 1);

  assert(pt.get_struct3(0, 1, 0) == 0);
  pt.update_struct3(0, 1, 0, 2);
  assert(pt.get_struct3(0, 1, 0) == 2);  

  std::ostringstream pt_out;
  pt.store(pt_out);
  std::istringstream pt_in(pt_out.str());
  ParamTable pt_copy;
  pt_copy.load(pt_in, false);
  pt_copy.set_label_extractor(le);
  assert(pt_copy == pt);

  std::cout << pt << std::endl;
}

#endif // TEST_ParamTable_cc
