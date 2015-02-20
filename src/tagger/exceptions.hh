/**
 * @file    exceptions.hh                                                    
 * @Author  Miikka Silfverberg                                               
 * @brief   Exceptions.                                                      
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

#ifndef HEADER_exceptions_hh
#define HEADER_exceptions_hh

#include <exception>

struct SyntaxError : public std::exception
{};

struct EmptyLine : public std::exception
{};

struct NumericalRangeError : public std::exception
{};

struct IllegalLabel : public std::exception
{};

struct LabelNotSet : public std::exception
{};

struct UnknownSuffixPair : public std::exception
{};

struct UnknownClass : public std::exception
{};

struct UnlabeledData : public std::exception
{};

struct NoLabel : public std::exception
{};

struct WordNotSet : public std::exception
{};

struct WriteFailed : public std::exception
{};

struct ReadFailed : public std::exception
{};

struct BadBinary : public std::exception
{};

#endif // HEADER_exceptions_hh
