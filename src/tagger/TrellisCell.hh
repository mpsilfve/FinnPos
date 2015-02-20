/**
 * @file    TrellisCell.hh                                                   
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

#ifndef HEADER_TrellisCell_hh
#define HEADER_TrellisCell_hh

#include <vector>

struct TrellisCell
{
  TrellisCell(void);

  float fw;
  float bw;
  float viterbi;
  float marginal;
  unsigned int label;
  unsigned int label_index;
  TrellisCell * pcell;
};

struct CmpTrellisCell
{
  bool operator() (const TrellisCell * tc1, const TrellisCell * tc2) const;
};

#endif // HEADER_TrellisCell_hh
