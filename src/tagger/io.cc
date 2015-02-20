/**
 * @file    io.cc                                                            
 * @Author  Miikka Silfverberg                                               
 * @brief   Helpers for io.                                                  
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

#include "io.hh"
#include "exceptions.hh"

#ifndef TEST_io_cc

Entry::Entry(void) {}

void split(const std::string &str, StringVector &target, char delim)
{
  size_t old_delim_pos = -1;
  size_t new_delim_pos = str.find(delim, old_delim_pos + 1);

  do
    {
      std::string field = str.substr(old_delim_pos + 1, 
				     new_delim_pos - old_delim_pos - 1);

      target.push_back(field);

      old_delim_pos = new_delim_pos;
      new_delim_pos = str.find(delim, old_delim_pos + 1);
    }
  while (old_delim_pos != std::string::npos);
}

Entry get_next_line(std::istream &in)
{
  std::string line;
  std::getline(in, line);

  if (line.empty())
    { throw EmptyLine(); }

  StringVector fields;
  split(line, fields, '\t');

  Entry res;

  if (fields.size() != 5)
    { 
      throw SyntaxError();
    }

  for (unsigned int i = 0; i < fields.size(); ++i)
    {
      if (fields[i].empty())
	{
	  throw SyntaxError();
	}
    }

  res.token = fields.at(0);

  std::string feat_template_string = fields.at(1);
  split(feat_template_string, res.feat_templates, ' ');

  if (fields.at(2) == "_")
    { 
      res.lemma = "";
    }
  else
    { 
      res.lemma = fields.at(2); 
    }

  if (fields.at(3) != "_")
    {
      std::string label_string = fields.at(3);
      split(label_string, res.labels, ' ');      
    } 

  res.annotations = fields.at(4);

  return res;
}

bool check(std::string &fn, std::ostream &out, std::ostream &msg_out)
{
  out << "";

  if (out.fail())
    {
      msg_out <<  "Output file " << fn << " can't be opened for writing."
	      << std::endl;
      
      return 0;
    }
  else
    {
      return 1;
    }
}

bool check(std::string &fn, std::istream &in, std::ostream &msg_out)
{
  static_cast<void>(in.peek());

  if (in.fail())
    {
      msg_out <<  "Input file " << fn << " can't be opened for reading."
	      << std::endl;
      
      return 0;
    }
  else
    {
      return 1;
    }
}

bool homoendian(std::istream &in, int marker)
{
  int marker_in_file;
  read_val<int>(in, marker_in_file, 0);

  if (in.fail())
    { throw ReadFailed(); }

  return marker_in_file == marker;
}

void init_string_stream(std::istream &in, std::istringstream &str_in, bool reverse_bytes)
{
  std::string binary_string;
  read_char_buffer(in, binary_string, reverse_bytes);

  if (in.fail())
    { throw ReadFailed(); }

  str_in.str(binary_string);
}

template<> void read_val(std::istream &in, std::string &str, bool reverse_bytes)
{
  // String byte order never needs to be reversed.
  static_cast<void>(reverse_bytes);

  std::getline(in, str, '\0');

  if (in.fail())
    { throw ReadFailed(); }
}

#include <cassert>

template<> void write_val(std::ostream &out, const std::string &str)
{
  assert(str.find('\0') == std::string::npos);

  out.write(str.c_str(), str.size() + 1);

  if (out.fail())
    { 
      throw WriteFailed();
    }
}

void write_char_buffer(std::ostream &out, const std::string &buffer)
{
  write_val<unsigned int>(out, buffer.size());
  out.write(buffer.c_str(), buffer.size());
  
  if (out.fail())
    { 
      throw WriteFailed();
    }
}

void read_char_buffer(std::istream &in, std::string &buffer, bool reverse_bytes)
{
  unsigned int size;
  read_val<unsigned int>(in, size, reverse_bytes);
  
  char * buf = static_cast<char *>(malloc(size));
  in.read(buf, size);
  buffer = std::string(buf, size);
  free(buf);

  if (in.fail())
  { 
    throw ReadFailed();
  }
}

#else // TEST_io_cc

#include <cassert>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unordered_map>

int main(void)
{
  // Empty input string.
  // Splitting gives ("")
  // get_next_line throws EmptyLine.
  
  std::string str1;
  
  StringVector fields1;
  split(str1, fields1, '\t');
  assert(fields1.size() == 1);

  std::istringstream in1(str1);

  try
    {
      Entry entry = get_next_line(in1);
      static_cast<void>(entry); 

      assert(0);
    }
  catch (const EmptyLine &e)
    { 
      /* EXPECTED FAIL */ 
    }

  // Two field string "foo\tbar".
  // Splitting gives ("foo", "bar")
  // get_next_line throws SyntaxError due to
  // insufficient firld count.

  std::string str2 = "foo\tbar";

  StringVector fields2;
  split(str2,fields2,'\t');

  assert(fields2.size() == 2);
  assert(fields2[0] == "foo");
  assert(fields2[1] == "bar");

  std::istringstream in2(str2);

  try
    {
      Entry entry = get_next_line(in2);
      static_cast<void>(entry); 

      assert(0);
    }
  catch (const SyntaxError &e)
    { 
      /* EXPECTED FAIL */ 
    }

  // Five field string "foo\tbar\tfoo\tbar\tfoo".
  // Splitting gives ("foo", "bar", "foo", "bar", "foo")
  // get_next_line gives Entry with
  // token == "foo"
  // feat_templates == ("bar")
  // lemma == "foo"
  // labels == ("bar")
  // annotations == foo

  std::string str3 = "foo\tbar\tfoo\tbar\tfoo";

  StringVector fields3;
  split(str3,fields3,'\t');

  assert(fields3.size() == 5);
  assert(fields3[0] == "foo");
  assert(fields3[1] == "bar");
  assert(fields3[2] == "foo");
  assert(fields3[3] == "bar");
  assert(fields3[4] == "foo");

  std::istringstream in3(str3);

  try
    {
      Entry entry = get_next_line(in3);

      assert(entry.token == "foo");
      assert(entry.feat_templates.size() == 1);
      assert(entry.feat_templates[0] == "bar");
      assert(entry.lemma == "foo");
      assert(entry.labels.size() == 1);
      assert(entry.labels[0] == "bar");
      assert(entry.annotations == "foo");
    }
  catch (const EmptyLine &e)
    { 
      assert(0);
    }
  catch (const SyntaxError &e)
    {
      assert(0);
    }

  // Five field string "foo\tbar baz\tfoo\tbar quux\tfoo".
  // Splitting gives ("foo", "bar baz", "foo", "bar quux", "foo")
  // get_next_line gives Entry with
  // token == "foo"
  // feat_templates == ("bar", "baz")
  // lemma == "foo"
  // labels == ("bar", "quux")
  // annotations == foo

  std::string str4 = "foo\tbar baz\tfoo\tbar quux\tfoo";

  StringVector fields4;
  split(str4,fields4,'\t');

  assert(fields4.size() == 5);
  assert(fields4[0] == "foo");
  assert(fields4[1] == "bar baz");
  assert(fields4[2] == "foo");
  assert(fields4[3] == "bar quux");
  assert(fields4[4] == "foo");

  std::istringstream in4(str4);

  try
    {
      Entry entry = get_next_line(in4);

      assert(entry.token == "foo");
      assert(entry.feat_templates.size() == 2);
      assert(entry.feat_templates[0] == "bar");
      assert(entry.feat_templates[1] == "baz");
      assert(entry.lemma == "foo");
      assert(entry.labels.size() == 2);
      assert(entry.labels[0] == "bar");
      assert(entry.labels[1] == "quux");
      assert(entry.annotations == "foo");
    }
  catch (const EmptyLine &e)
    { 
      assert(0);
    }
  catch (const SyntaxError &e)
    {
      assert(0);
    }

  // Five field string "foo\tbar\t_\t_\t_".
  // get_next_line gives Entry with
  // token == "foo"
  // feat_templates == ("bar")
  // lemma == ""
  // labels == ()
  // annotations == "_"

  std::string str5 = "foo\tbar\t_\t_\t_";

  std::istringstream in5(str5);

  try
    {
      Entry entry = get_next_line(in5);

      assert(entry.token == "foo");
      assert(entry.feat_templates.size() == 1);
      assert(entry.feat_templates[0] == "bar");
      assert(entry.lemma == "");
      assert(entry.labels.size() == 0);
      assert(entry.annotations == "_");
    }
  catch (const EmptyLine &e)
    { 
      assert(0);
    }
  catch (const SyntaxError &e)
    {
      assert(0);
    }

  std::string f_str = "a";

  for (unsigned int i = 1; i < sizeof(float); ++i)
    { f_str += "b"; }

  std::string rev_f_str = f_str;

  std::reverse(rev_f_str.begin(), rev_f_str.end());

  float f1;
  f1 = *(reinterpret_cast<const float *>(f_str.c_str()));
  
  float f2;
  f2 = *(reinterpret_cast<const float *>(rev_f_str.c_str()));

  assert(reverse_num(f1) == f2);

  // Reading from empty file should throw WriteFailed.
  std::istringstream empty_in;
  try
    {
      int i;
      read_val<int>(empty_in, i, false);
      assert(0);
    }
  catch (ReadFailed &e)
    { /* EXPECTED FAIL */ }

  try
    {
      std::string str;
      read_val<std::string>(empty_in, str, false);
      assert(0);
    }
  catch (ReadFailed &e)
    { /* EXPECTED FAIL */ }
  
  // Writing to closed file or similar should throw WriteFailed.
  //std::ostringstream closed_out_ss;
  std::ofstream closed_out;
  closed_out.close();
  try
    {
      write_val<int>(closed_out, 1);
      assert(0);
    }
  catch (WriteFailed &e)
    { /* EXPECTED FAIL */ }

  try
    {
      write_val<std::string>(closed_out, "foo");
      assert(0);
    }
  catch (WriteFailed &e)
    { /* EXPECTED FAIL */ }

  // Write and read an int.
  std::ostringstream test_int_out;
  write_val<int>(test_int_out,13131);
  std::istringstream test_int_in(test_int_out.str());
  int int_copy;
  read_val<int>(test_int_in, int_copy, false);
  assert(int_copy == 13131);

  // Write an int in reversed endianness and read in system
  // endianness.
  std::ostringstream test_rev_int_out;
  write_val<int>(test_rev_int_out,reverse_num<int>(13131));
  std::istringstream test_rev_int_in(test_rev_int_out.str());
  read_val<int>(test_rev_int_in, int_copy, true);
  assert(int_copy == 13131);

  // Write and read a string.
  std::ostringstream test_string_out;
  write_val<std::string>(test_string_out,"foo");
  std::istringstream test_string_in(test_string_out.str());
  std::string str_copy;
  read_val<std::string>(test_string_in, str_copy, false);
  assert(str_copy == "foo");

  // Make sure that reverse-variable is properly ignored with strings.
  std::ostringstream test_string_out_rev;
  write_val<std::string>(test_string_out_rev,"foo");
  std::istringstream test_string_in_rev(test_string_out_rev.str());
  read_val<std::string>(test_string_in_rev, str_copy, true);
  assert(str_copy == "foo");
  
  // Write and read an empty std::vector<int>.
  std::ostringstream vect_out_1;
  write_vector<int>(vect_out_1, std::vector<int>());
  std::istringstream vect_in_1(vect_out_1.str());
  std::vector<int> int_v;
  read_vector<int>(vect_in_1, int_v, false);
  assert(int_v == std::vector<int>());
  
  // Write and read an empty std::vector<std::string>.
  std::ostringstream vect_out_2;
  write_vector<std::string>(vect_out_2, std::vector<std::string>());
  std::istringstream vect_in_2(vect_out_2.str());
  std::vector<std::string> string_v;
  read_vector<std::string>(vect_in_2, string_v, false);
  assert(string_v == std::vector<std::string>());
  
  // Write and read a non-empty std::vector<int>.
  std::ostringstream vect_out_3;
  std::vector<int> int_v_orig;
  int_v_orig.push_back(0);
  int_v_orig.push_back(1);
  int_v_orig.push_back(2);
  write_vector<int>(vect_out_3, int_v_orig);
  std::istringstream vect_in_3(vect_out_3.str());
  int_v.clear();
  read_vector<int>(vect_in_3, int_v, false);
  assert(int_v == int_v_orig);
  
  // Write and read a non-empty std::vector<std::string>.
  std::ostringstream vect_out_4;
  std::vector<std::string> string_v_orig;
  string_v_orig.push_back("foo");
  string_v_orig.push_back("bar");
  string_v_orig.push_back("baz");
  write_vector<std::string>(vect_out_4, string_v_orig);
  std::istringstream vect_in_4(vect_out_4.str());
  string_v.clear();
  read_vector<std::string>(vect_in_4, string_v, false);
  assert(string_v == string_v_orig);
  
  // Test reading and writing of map<T, U>
  std::ostringstream map_out_1;
  std::unordered_map<int, float> m;
  write_map<int, float>(map_out_1, m);
  std::unordered_map<int, float> m_copy;
  std::istringstream map_in_1(map_out_1.str());
  read_map<int, float>(map_in_1, m_copy, false);
  assert(m == m_copy);

  std::ostringstream map_out_2;
  m.clear();
  m[0] = 0.1;
  m[100] = 5.1;
  write_map<int, float>(map_out_2, m);
  m_copy.clear();
  std::istringstream map_in_2(map_out_2.str());
  read_map<int, float>(map_in_2, m_copy, false);
  assert(m == m_copy);

  // Test reading and writing of map<T, std::vector<U> >
  std::ostringstream map_out_3;
  std::unordered_map<int, std::vector<float> > m1;
  write_map<int, float>(map_out_3, m1);
  std::unordered_map<int, std::vector<float> > m_copy1;
  std::istringstream map_in_3(map_out_3.str());
  read_map<int, float>(map_in_3, m_copy1, false);
  assert(m1 == m_copy1);

  std::ostringstream map_out_4;
  m1.clear();
  m1[0] = std::vector<float>();
  m1[100] = std::vector<float>();
  m1[100].push_back(1);
  m1[100].push_back(2);
  write_map<int, float>(map_out_4, m1);
  m_copy1.clear();
  std::istringstream map_in_4(map_out_4.str());
  read_map<int, float>(map_in_4, m_copy1, false);
  assert(m1 == m_copy1);

  // Test reading and writing of map<T, std::unordered_map<U, V> >
  std::ostringstream map_out_5;
  std::unordered_map<int, std::unordered_map<int, float> > m2;
  write_map<int, int, float>(map_out_5, m2);
  std::unordered_map<int, std::unordered_map<int, float> > m_copy2;
  std::istringstream map_in_5(map_out_5.str());
  read_map<int, int, float>(map_in_5, m_copy2, false);
  assert(m2 == m_copy2);

  std::ostringstream map_out_6;
  m2.clear();
  m2[0][0] = 0;
  m2[100][0] = 5.1;
  m2[100][100] = 16;
  write_map<int, int, float>(map_out_6, m2);
  m_copy2.clear();
  std::istringstream map_in_6(map_out_6.str());
  read_map<int, int, float>(map_in_6, m_copy2, false);
  assert(m2 == m_copy2);

  // Test reading and writing a pair.
  std::ostringstream pair_out_1;
  std::pair<char, float> p('a', 0.55);
  write_pair(pair_out_1, p);
  std::istringstream pair_in_1(pair_out_1.str());
  std::pair<char, float> p_copy;
  read_pair(pair_in_1, p_copy, 0);
  assert(p == p_copy);

  // Test reading and writing of map<T, std::pair<U, V> >
  std::ostringstream map_out_7;
  std::unordered_map<int, std::pair<int, float> > m3;
  write_map<int, int, float>(map_out_7, m3);
  std::unordered_map<int, std::pair<int, float> > m_copy3;
  std::istringstream map_in_7(map_out_7.str());
  read_map<int, int, float>(map_in_7, m_copy3, false);
  assert(m3 == m_copy3);

  std::ostringstream map_out_8;
  m3.clear();
  m3[0].first = 0;
  m3[0].second = 1;
  m3[100].first = 5.1;
  m3[100].second = 16;
  write_map<int, int, float>(map_out_8, m3);
  m_copy3.clear();
  std::istringstream map_in_8(map_out_8.str());
  read_map<int, int, float>(map_in_8, m_copy3, false);
  assert(m3 == m_copy3);

}

#endif // TEST_io_cc
