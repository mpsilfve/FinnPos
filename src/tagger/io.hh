/**
 * @file    io.hh                                                            
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

#ifndef HEADER_io_hh
#define HEADER_io_hh

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
//#include <unordered_map>
#include "UnorderedMapSet.hh"
#include <sstream>

#include "exceptions.hh"

// Print variable name and value for debugging.
#define PRINT_VAR(V) std::cerr << #V " " << V << std::endl;

typedef std::vector<std::string> StringVector;

/**
 * @brief Holds all the information concerning a single word in a training or
 * test file.
 */
struct Entry
{
  Entry(void);

  std::string token;
  StringVector feat_templates;
  std::string lemma;
  StringVector labels;
  std::string annotations;
};

/**
 * @brief Split @p str at @p delim characters and store the pieces in @p
 * target. Discards all @p delim characters.
 */
void split(const std::string &str, StringVector &target, char delim);

/**
 * @brief Read the next line in stream @p in and store the information
 * in an Entry object. Throws SyntaxError.
 */
Entry get_next_line(std::istream &in);

/**
 * @brief Check that stream @out is okay for writing. Write an error
 * message to @p msg_out if that is not the case.
 */
bool check(std::string &fn, std::ostream &out, std::ostream &msg_out);

/**
 * @brief Check that stream @in is okay for reading. Write an error
 * message to @p msg_out if that is not the case.
 */
bool check(std::string &fn, std::istream &in, std::ostream &msg_out);

/**
 * @brief Return true, if the endianness of @p in and @p marker
 * match. Otherwise, return false. Throws ReadFailed.
 */
bool homoendian(std::istream &in, int marker);

/**
 * @brief Init @p str_in with the contents of @p in. 
 * 
 * First an unsigned int is read from @p in, which tells the number of
 * bytes to be read into @p str_in. If endianness of the stream @p in
 * ans the system differ, @reverse_bytes has to be true.
 */
void init_string_stream(std::istream &in, std::istringstream &str_in, bool reverse_bytes);

/**
 * @brief Return the big endian representation of a little endian
 * number and vice versa.
 */
template<class T> T reverse_num(T numerical_val)
{
  char * start = reinterpret_cast<char *>(&numerical_val);
  char * stop  = start + sizeof(T);
  
  std::reverse(start, stop);

  return numerical_val;
}

/**
 * @brief Read a numerical value from @p in. Reverse the endianness,
 * iff @p reverse_bytes == true. Throws ReadFailed.
 */
template<class T> void read_val(std::istream &in, T &t, bool reverse_bytes)
{
  in.read(reinterpret_cast<char *>(&t), sizeof(T));

  if (in.fail())
    { 
      throw ReadFailed(); 
    }

  if (reverse_bytes)
    { t = reverse_num(t); }

  //  return reverse_bytes ? reverse_num(t) : t;
}

/**
 * @brief Write numerical value @p t to stream @p out. Throws
 * WriteFailed.
 */
template<class T> void write_val(std::ostream &out, const T &t)
{
  out.write(reinterpret_cast<const char *>(&t), sizeof(T));
  
  if (out.fail())
    { 
      throw WriteFailed();
    }
}

/**
 * @brief Read a string from @p in. Throws ReadFailed.
 */
template<> void read_val(std::istream &in, std::string &str, bool reverse_bytes);

/**
 * @brief Write string @p str to stream @p out. Throws
 * WriteFailed. Can only be used to write valid c-strings, not
 * arbitrary char buffers!
 */
template<> void write_val(std::ostream &out, const std::string &str);

/** 
 * @brief Function for writing char buffers.
 */
void write_char_buffer(std::ostream &out, const std::string &buffer);

/** 
 * @brief Function for reading char buffers.
 */
void read_char_buffer(std::istream &in, std::string &buffer, bool reverse_bytes);

/**
 * @brief Read a vector from stream @p in. Store it in @p v. Reverse
 * byte order, iff reverse_bytes == true. Instantiate with std::string
 * and numerical types only! Throws ReadFailed.
 */
template<class T> std::vector<T> &read_vector(std::istream &in, 
					      std::vector<T> &v, 
					      bool reverse_bytes)
{
  unsigned int size;
  read_val<unsigned int>(in, size, reverse_bytes);

  v.insert(v.end(), size, T());

  for (unsigned int i = 0; i < size; ++i)
    {
      //     T t;
      //      v.push_back(read_val<T>(in, reverse_bytes));
      read_val<T>(in, v[i], reverse_bytes);
    }

  return v;
}

/**
 * @brief Write vector @p v to stream @p out. Instantiate with
 * std::string and numerical types only! Throws WriteFailed.
 */
template<class T> void write_vector(std::ostream &out, 
				    const std::vector<T> &v)
{
  write_val<unsigned int>(out, v.size());
				
  for (unsigned int i = 0; i < v.size(); ++i)
    {
      write_val<T>(out, v.at(i));
    }
}

/**
 * @brief Read a pair from stream @p in. Store it in @p p. Reverse
 * byte order, iff reverse_bytes == true. Instantiate with std::string
 * and numerical types only! Throws ReadFailed.
 */
template<class T, class U> std::pair<T, U> &read_pair(std::istream &in, 
						      std::pair<T, U> &p, 
						      bool reverse_bytes)
{
  read_val<T>(in, p.first, reverse_bytes);
  read_val<U>(in, p.second, reverse_bytes);
  
  return p;
}

/**
 * @brief Write pair @p p to stream @p out. Instantiate with
 * std::string and numerical types only! Throws WriteFailed.
 */
template<class T, class U> void write_pair(std::ostream &out, 
					   const std::pair<T, U> &p)
{
  write_val<T>(out, p.first);
  write_val<U>(out, p.second);	
}

/**
 * @brief Read a map from stream @p in. Store it in @p m. Reverse byte
 * order, iff reverse_bytes == true. Instantiate with std::string or
 * numerical types only! Throws ReadFailed.
 */
template<class T, class U> 
std::unordered_map<T, U> &read_map(std::istream &in,
				   std::unordered_map<T, U> &m,
				   bool reverse_bytes)
{
  unsigned int size;
  read_val<unsigned int>(in, size, reverse_bytes);

  for (unsigned int i = 0; i < size; ++i)
    {
      T t;
      read_val<T>(in, t, reverse_bytes);
      //U u = read_val<U>(in, reverse_bytes);

      read_val<U>(in, m[t], reverse_bytes);
    }

  return m;
}

/**
 * @brief Write map @p m to stream @p out. Instantiate with
 * std::string and numerical types only! Throws WriteFailed.
 */
template<class T, class U> void write_map(std::ostream &out,
					  const std::unordered_map<T, U> &m)
{
  write_val<unsigned int>(out, m.size());

  for (typename std::unordered_map<T, U>::const_iterator it = m.begin();
       it != m.end();
       ++it)
    {
      write_val<T>(out, it->first);
      write_val<U>(out, it->second);
    }
}

/**
 * @brief Read a map from stream @p in. Store it in @p m. Reverse byte
 * order, iff reverse_bytes == true. Instantiate with std::string or
 * numerical types only! Throws ReadFailed.
 */
template<class T, class U> std::unordered_map<T, std::vector<U> > &
read_map(std::istream &in,
	 std::unordered_map<T, std::vector<U> > &m,
	 bool reverse_bytes)
{
  unsigned int size;
  read_val<unsigned int>(in, size, reverse_bytes);

  for (unsigned int i = 0; i < size; ++i)
    {
      T t;
      read_val<T>(in, t, reverse_bytes);

      //      std::vector<U> v;
      read_vector<U>(in, m[t], reverse_bytes);

      //      m[t] = v;
    }

  return m;
}

/**
 * @brief Write map @p m to stream @p out. Instantiate with
 * std::string and numerical types only! Throws WriteFailed.
 */
template<class T, class U> 
void write_map(std::ostream &out,
	       const std::unordered_map<T, std::vector<U> > &m)
{
  write_val<unsigned int>(out, m.size());

  for (typename std::unordered_map<T, std::vector<U> >::const_iterator it = 
	 m.begin();
       it != m.end();
       ++it)
    {
      write_val<T>(out, it->first);
      write_vector<U>(out, it->second);
    }
}

/**
 * @brief Read a map from stream @p in. Store it in @p m. Reverse byte
 * order, iff reverse_bytes == true. Instantiate with std::string or
 * numerical types only! Throws ReadFailed.
 */
template<class T, class U, class V> std::unordered_map<T, std::pair<U, V> > &
read_map(std::istream &in,
	 std::unordered_map<T, std::pair<U, V> > &m,
	 bool reverse_bytes)
{
  unsigned int size;
  read_val<unsigned int>(in, size, reverse_bytes);

  for (unsigned int i = 0; i < size; ++i)
    {
      T t;
      read_val<T>(in, t, reverse_bytes);

      //      std::pair<U, V> p;
      read_pair<U, V>(in, m[t], reverse_bytes);

      //      m[t] = p;
    }

  return m;
}

/**
 * @brief Write map @p m to stream @p out. Instantiate with
 * std::string and numerical types only! Throws WriteFailed.
 */
template<class T, class U, class V> 
void write_map(std::ostream &out,
	       const std::unordered_map<T, std::pair<U, V> > &m)
{
  write_val<unsigned int>(out, m.size());

  for (typename std::unordered_map<T, std::pair<U, V> >::const_iterator it = 
	 m.begin();
       it != m.end();
       ++it)
    {
      write_val<T>(out, it->first);
      write_pair<U, V>(out, it->second);
    }
}

/**
 * @brief Read a map from stream @p in. Store it in @p m. Reverse byte
 * order, iff reverse_bytes == true. Instantiate with std::string or
 * numerical types only! Throws ReadFailed.
 */
template<class T, class U, class V> std::unordered_map<T, std::unordered_map<U, V> > &
read_map(std::istream &in,
	 std::unordered_map<T, std::unordered_map<U, V> > &m,
	 bool reverse_bytes)
{
  unsigned int size;
  read_val<unsigned int>(in, size, reverse_bytes);

  for (unsigned int i = 0; i < size; ++i)
    {
      T t;
      read_val<T>(in, t, reverse_bytes);

      //      std::unordered_map<U, V> mm;
      read_map<U, V>(in, m[t], reverse_bytes);

      //      m[t] = mm;
    }

  return m;
}

/**
 * @brief Write map @p m to stream @p out. Instantiate with
 * std::string and numerical types only! Throws WriteFailed.
 */
template<class T, class U, class V> 
void write_map(std::ostream &out,
	       const std::unordered_map<T, std::unordered_map<U, V> > &m)
{
  write_val<unsigned int>(out, m.size());

  for (typename std::unordered_map<T, std::unordered_map<U, V> >::const_iterator it = 
	 m.begin();
       it != m.end();
       ++it)
    {
      write_val<T>(out, it->first);
      write_map<U, V>(out, it->second);
    }
}

#endif // HEADER_io_hh
