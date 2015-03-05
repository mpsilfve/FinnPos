# FinnPos - CRF-based Morphological Tagging and Lemmatization

FinnPos ia a morphological tagger and lemmatizer, that is, it takes
sentences as input and provides each word in the sentence with a
disambiguated morphological label and a lemma. For example the word
"dogs" might receive a label "Noun+Singular+Nominative" and lemma
"dog".

FinnPos is a tool for training and using data driven taggers for
morphologically rich languages such as Finnish. It is based on
Conditional random field models and features

* State-of-the-art tagging accuracy.
* Fast estimation and application for large morphological label sets
  exceeding 1000 label types using a cascade model and averaged
  perceptron training.
* The possibility of using the tagger to disambiguate the output of a
  morphological analyzer.
* Data driven lemmatization for OOV words.
* Better support for complex morphological labels
  (e.g. "Noun+Plural+Nominative") that consist of multiple
  sub-labels. Statistics are estimated from both complete labels and
  sub-labels.
* Flexible feature extraction. You can define your own feature sets or
  use a standard set of pre-defined features.

In addition to utilities for training your own morphological taggers,
FinnPos provides a morphological tagger for Finnish which is trained
using the high quality manually constructed Finnish corpus
[FinnTreeBank
1](http://www.ling.helsinki.fi/kieliteknologia/tutkimus/treebank/).

The code for FinnPos is licensed under the Apache Software Lincense
Version 2.0. External resources such as morphological analyzers,
external software and corpora, that are distributed with this code,
may be licensed under different conditions. Their licenses are located
in the appropriate directories.

Help for building, installing and using FinnPos may be found in the
FinnPos wiki

  https://github.com/mpsilfve/FinnPos/wiki