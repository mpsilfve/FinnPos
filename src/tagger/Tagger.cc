/**
 * @file    Tagger.cc                                                        
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

#include "Tagger.hh"

#ifndef TEST_Tagger_cc

#include <sstream>

#include "PerceptronTrainer.hh"
#include "SGDTrainer.hh"
#include "Trellis.hh"

#define FINN_POS_ID_STRING "FinnPosModel"
const int ENDIANNESS_MARKER=1;

Tagger::Tagger(std::ostream &msg_out):
  msg_out(msg_out)
{}

Tagger::Tagger(const TaggerOptions &tagger_options, 
	       std::ostream &msg_out):
  line_counter(0),
  tagger_options(tagger_options),
  label_extractor(tagger_options.suffix_length),
  msg_out(msg_out)
{
  if (tagger_options.estimator == AVG_PERC)
    {
      if (tagger_options.regularization != NONE or 
	  tagger_options.delta != -1            or 
	  tagger_options.sigma != -1)
	{ 
	  msg_out << "Warning! Averaged perceptron doesn't utilize delta," 
		  << std::endl
		  << "sigma or regularization. Options will be discarded." 
		  << std::endl 
		  << std::endl;
	}
    }
}

Tagger::Tagger(std::istream &tagger_opt_in, std::ostream &msg_out):
  line_counter(0),
  tagger_options(tagger_opt_in, line_counter),
  label_extractor(tagger_options.suffix_length),
  msg_out(msg_out)
{
  if (tagger_options.estimator == AVG_PERC)
    {
      if (tagger_options.regularization != NONE or 
	  tagger_options.delta != -1            or 
	  tagger_options.sigma != -1)
	{ 
	  msg_out << "Warning! Averaged perceptron doesn't utilize delta," 
		  << std::endl
		  << "sigma or regularization. Options will be discarded." 
		  << std::endl 
		  << std::endl;
	}
    }

  line_counter = 0;
}

#include <cassert>

void Tagger::train(std::istream &train_in,
		   std::istream &dev_in)
{
  msg_out << "Reading training data." << std::endl;
  Data train_data(train_in, 1, label_extractor, param_table, 
		  tagger_options.degree);
  train_data.clear_label_guesses(); 
  train_data.randomize();
  
  msg_out << "Training label guesser." << std::endl;
  label_extractor.train(train_data);
  param_table.set_label_extractor(label_extractor);

  msg_out << "Reading development data." << std::endl;
  Data dev_data(dev_in, 1, label_extractor, param_table, tagger_options.degree);
  dev_data.clear_label_guesses(); 

  msg_out << "Setting label guesses." << std::endl;
  train_data.set_label_guesses(label_extractor, 0, tagger_options.guess_mass);
  dev_data.set_label_guesses(label_extractor, 1, 
			     0.9999 /*tagger_options.guess_mass*/);

  msg_out << "Estimating lemmatizer parameters." << std::endl;
  lemma_extractor.set_max_passes(tagger_options.max_lemmatizer_passes);
  lemma_extractor.train(train_data, dev_data, label_extractor, msg_out);

  msg_out << "Estimating tagger parameters." << std::endl;
  if (tagger_options.estimator == AVG_PERC)
    {
      PerceptronTrainer trainer(tagger_options.max_train_passes, 
				tagger_options.max_useless_passes, 
				param_table, 
				label_extractor, 
				lemma_extractor,
				msg_out);
      
      trainer.train(train_data, dev_data, tagger_options.beam, 		    
		    tagger_options.beam_mass);
    }
  else
    { throw NotImplemented(); }

  param_table.set_label_extractor(label_extractor);
}

void Tagger::evaluate(std::istream &in)
{
  msg_out << "Evaluating." << std::endl;
  Data data(in, 1, label_extractor, param_table, tagger_options.degree);
  data.clear_label_guesses(); 

  Data data_copy(data);
  data_copy.unset_label();

  data_copy.set_label_guesses(label_extractor, 1, tagger_options.guess_mass);

  TrellisVector trellises;
  
  for (unsigned int i = 0; i < data.size(); ++i)
    {
      trellises.push_back(new Trellis(data_copy.at(i), 
				      label_extractor.get_boundary_label(), 
				      tagger_options.beam));
    }

  // Tag test data.
  for (unsigned int j = 0; j < trellises.size(); ++j)
    {
      trellises[j]->set_maximum_a_posteriori_assignment(param_table);      
    }
  
  data_copy.predict_lemma(lemma_extractor, label_extractor);

  Acc accs = data.get_acc(data_copy, lemma_extractor);
  
  msg_out << "  Final test label acc: " << accs.label_acc * 100.00 << "%" 
	  << std::endl;
  msg_out << "  Final test OOV label acc: " << accs.oov_label_acc * 100.00 
	  << "%" 
	  << std::endl;
  msg_out << "  Final test lemma acc: " << accs.lemma_acc * 100.00 << "%" 
	  << std::endl;
  msg_out << "  Final test OOV lemma acc: " << accs.oov_lemma_acc * 100.00 
	  << "%" 
	  << std::endl;

  for (unsigned int i = 0; i < data.size(); ++i)
    {
      delete trellises[i];
    }
}

void Tagger::label(std::istream &in)
{
  Data data(in, 0, label_extractor, param_table, tagger_options.degree);
  data.set_label_guesses(label_extractor, 1, tagger_options.guess_mass);

  TrellisVector trellises;
  
  for (unsigned int i = 0; i < data.size(); ++i)
    {
      trellises.push_back(new Trellis(data.at(i), 
				      label_extractor.get_boundary_label(), 
				      tagger_options.beam));
    }

  // Tag test data.
  for (unsigned int j = 0; j < trellises.size(); ++j)
    {
      trellises[j]->set_maximum_a_posteriori_assignment(param_table);      
    }
  
  data.predict_lemma(lemma_extractor, label_extractor);

  for (unsigned int i = 0; i < data.size(); ++i)
    {
      for (unsigned int j = 0; j < data.at(i).size(); ++j)
	{
	  if (data.at(i).at(j).get_word_form() == "_#_")
	    { continue; }

	  std::cout << data.at(i).at(j).get_word_form() 
		    << "\t_\t" << data.at(i).at(j).get_lemma() 
		    << "\t" << label_extractor.
	    get_label_string(data.at(i).at(j).get_label()) 
		    << "\t" << data.at(i).at(j).get_annotations() << std::endl;
	}
      std::cout << std::endl;
    }

  for (unsigned int i = 0; i < data.size(); ++i)
    {
      delete trellises[i];
    }
}

void Tagger::label_stream(std::istream &in)
{
  unsigned int line = 0;

  while (in)
    {
      Sentence s(in, 0, label_extractor, param_table, tagger_options.degree, line);

      if (s.size() == 0)
	{ continue; }

      s.set_label_guesses(label_extractor, 1, tagger_options.guess_mass);

      Trellis trellis(s, label_extractor.get_boundary_label(), 
		      tagger_options.beam);
  
      trellis.set_maximum_a_posteriori_assignment(param_table);      
  
      s.predict_lemma(lemma_extractor, label_extractor);

      for (unsigned int j = 0; j < s.size(); ++j)
	{
	  if (s.at(j).get_word_form() == "_#_")
	    { continue; }

	  std::cout << s.at(j).get_word_form() 
		    << "\t_\t" << s.at(j).get_lemma() 
		    << "\t" << label_extractor.
	    get_label_string(s.at(j).get_label()) 
		    << "\t" << s.at(j).get_annotations() << std::endl;
	}
      std::cout << std::endl;
    }
}

StringVector Tagger::labels_to_strings(const LabelVector &v)
{
  StringVector res;

  for (unsigned int i = 0; i < v.size(); ++i)
    { 
      if (v[i] != label_extractor.get_boundary_label())
	{
	  res.push_back(label_extractor.get_label_string(v[i])); 
	}
    }

  return res;
}

#include <cassert>

void Tagger::store(std::ostream &out) const
{
  msg_out << "Storing model." << std::endl;

  write_val<std::string>(out, FINN_POS_ID_STRING);
  write_val<int>(out, ENDIANNESS_MARKER);

  tagger_options.store(out);
  label_extractor.store(out);
  lemma_extractor.store(out);
  param_table.store(out);
}

void Tagger::load(std::istream &in)
{
  std::string id_string;
  read_val<std::string>(in, id_string, false);

  if (id_string != FINN_POS_ID_STRING)
    { 
      throw BadBinary();
    }

  bool reverse_bytes = not homoendian(in, ENDIANNESS_MARKER);

  tagger_options.load(in, msg_out, reverse_bytes);
  label_extractor.load(in, reverse_bytes);
  lemma_extractor.load(in, reverse_bytes);
  param_table.load(in, reverse_bytes); 
  param_table.set_label_extractor(label_extractor);
}

bool Tagger::operator==(const Tagger &another) const
{
  if (this == &another)
    { return 1; }

  bool t =
    (tagger_options == another.tagger_options   and
     label_extractor == another.label_extractor and
     lemma_extractor == another.lemma_extractor and
     param_table == another.param_table);

  return t;
}

LabelExtractor &Tagger::get_label_extractor(void)
{ return label_extractor; }

LemmaExtractor &Tagger::get_lemma_extractor(void)
{ return lemma_extractor; }

Data Tagger::get_data(const std::string &filename, bool tagged)
{
  return Data(filename, 
	      tagged, 
	      label_extractor, 
	      param_table, 
	      tagger_options.degree);
}

#else // TEST_Tagger_cc
#include <sstream>
#include <cassert>

int main(void)
{
  std::string train_contents("\n"
			     "The\tWORD=The\tthe\tDT\t_\n"
			     "dog\tWORD=dog SUF=og\tdog\tNN\t_\n"
			     ".\tWORD=.\t.\t.\t_\n"
			     "\n"
			     "\n"
			     "The\tWORD=The\tthe\tDT\t_\n"
			     "dog\tWORD=dog SUF=og\tdog\tNN\t_\n"
			     ".\tWORD=.\t.\t.\t_\n");
  
  std::istringstream train_in(train_contents);

  std::string dev_contents("\n"
			   "The\tWORD=The\tthe\tDT\t_\n"
			   "dog\tWORD=dog SUF=og\tdog\tNN\t_\n"
			   ".\tWORD=.\t.\t.\t_\n"
			   "\n"
			   "\n"
			   "The\tWORD=The\tthe\tDT\t_\n"
			   "dog\tWORD=dog SUF=og\tdog\tNN\t_\n"
			   ".\tWORD=.\t.\t.\t_\n");
  
  std::istringstream dev_in(dev_contents);

  TaggerOptions tagger_options(AVG_PERC, MAP, 10, 2, 20, 3, 20, -1, -1, NONE, 0, 0);
  std::ostringstream null_stream;
  Tagger tagger(tagger_options, null_stream);
  
  tagger.train(train_in, dev_in);

  std::string test_contents("\n"
			    "The\tWORD=The\tthe\tNN DT\t_\n"
			    "hog\tWORD=hog SUF=og\thog\tNN DT\t_\n"
			    ".\tWORD=.\t.\t.\t_\n"
			    "\n"
			    "\n"
			    "The\tWORD=The\tthe\tNN DT\t_\n"
			    "hog\tWORD=hog SUF=og\thog\tNN DT\t_\n"
			    ".\tWORD=.\t.\t.\t_\n");
  
  std::istringstream test_in(test_contents);
  /*
  StringVector labels = tagger.label(test_in);

  assert(labels[0] == "DT");
  assert(labels[1] == "NN");
  assert(labels[2] == ".");

  labels = tagger.label(test_in);

  assert(labels[0] == "DT");
  assert(labels[1] == "NN");
  assert(labels[2] == ".");
  */

  std::ostringstream tagger_out;
  tagger.store(tagger_out);
  std::istringstream tagger_in(tagger_out.str());
  Tagger tagger_copy(null_stream);
  tagger_copy.load(tagger_in);
  assert(tagger == tagger_copy);
}

#endif // TEST_Tagger_cc
