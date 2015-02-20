/**
 * @file    TrellisCell.cc                                                   
 * @Author  Miikka Silfverberg                                               
 * @brief   Represents one entry in a TrellisColumn.                        
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

#include "TrellisCell.hh"

#ifndef TEST_TrellisCell_cc

#include <cfloat>

bool CmpTrellisCell::operator() (const TrellisCell * tc1, 
				 const TrellisCell * tc2) const
{
  return (tc1->viterbi > tc2->viterbi);
}

TrellisCell::TrellisCell(void):
  fw         (-FLT_MAX),
  bw         (-FLT_MAX),
  viterbi    (-FLT_MAX),
  label      (-1),
  label_index(-1),
  pcell      (0)
{}

#else // TEST_TrellisCell_cc

#include <cassert>

#include <vector>
#include <algorithm>

int main(void)
{
  TrellisCell c1;
  c1.viterbi = 1;

  TrellisCell c2;
  c2.viterbi = 100;

  TrellisCell c3;
  c3.viterbi = -1000;

  std::vector<TrellisCell*> v;
    
  v.push_back(&c1);
  v.push_back(&c2);
  v.push_back(&c3);

  std::sort(v.begin(), v.end(), CmpTrellisCell());

  assert(v[0] == &c2);
  assert(v[1] == &c1);
  assert(v[2] == &c3);
}

#endif // TEST_TrellisCell_cc
