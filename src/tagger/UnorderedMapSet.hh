#ifndef HEADER_UnorderedMapSet_hh
#define HEADER_UnorderedMapSet_hh

#include <tr1/unordered_map>
#include <tr1/unordered_set>

// Ugly gum to make unordered_map and unordered_set work the same on
// all platforms.

namespace std
{
  using std::tr1::unordered_map;
  using std::tr1::unordered_set;
}

template<class T, class U> bool operator!=(const std::unordered_map<T,U> &m1,
					   const std::unordered_map<T,U> &m2);

template<class T> bool operator!=(const std::unordered_set<T> &s1,
				  const std::unordered_set<T> &s2);

template<class T, class U> bool operator==(const std::unordered_map<T,U> &m1,
					   const std::unordered_map<T,U> &m2)
{
  for (typename std::unordered_map<T,U>::const_iterator it = m1.begin();
       it != m1.end();
       ++it)
    {
      if (m2.count(it->first) == 0)
	{ return false; }
      
      if (m2.find(it->first)->second != it->second)
	{ return false; }
    }

  for (typename std::unordered_map<T,U>::const_iterator it = m2.begin();
       it != m2.end();
       ++it)
    {
      if (m1.count(it->first) == 0)
	{ return false; }
      
      if (m1.find(it->first)->second != it->second)
	{ return false; }
    }

  return true;
}

template<class T, class U> bool operator!=(const std::unordered_map<T,U> &m1,
					   const std::unordered_map<T,U> &m2)
{ return not (m1 == m2); }

template<class T> bool operator==(const std::unordered_set<T> &s1,
				  const std::unordered_set<T> &s2)
{
  for (typename std::unordered_set<T>::const_iterator it = s1.begin();
       it != s1.end();
       ++it)
    {
      if (s2.count(*it) == 0)
	{ return false; }
    }

  for (typename std::unordered_set<T>::const_iterator it = s2.begin();
       it != s2.end();
       ++it)
    {
      if (s2.count(*it) == 0)
	{ return false; }
    }

  return true;
}

template<class T> bool operator!=(const std::unordered_set<T> &s1,
				  const std::unordered_set<T> &s2)
{ return not (s1 == s2); }

#endif // HEADER_UnorderedMapSet_hh
