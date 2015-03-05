# FinnPos - CRF-based Morphological Tagging and Lemmatization

FinnPos is a tool for training and applying data driven morphological
taggers and lemmatizers for morphologically rich languages such as
Finnish. It is based on the Conditional random field framework and
features

* State-of-the-art tagging accuracy.
* Fast estimation and application for large morphological label sets
  exceeding 1000 label types using a cascade model and averaged
  perceptron training.
* Possibility to use the tagger for disambiguating the output of a
  morphological analyzer.
* Data driven lemmatization for OOV words.
* Built-in support for complex morphological labels
  (e.g. "Noun+Plural+Nominative") that consist of multiple
  sub-labels. Statistics are estimated for both complete labels and
  for sub-labels.
* Flexible feature extraction. You can define your own feature sets or
  use a standard set of pre-defined features.

In addition to utilities for training your own morphological taggers,
FinnPos provides a morphological tagger, FinnTreeBank tagger, for
Finnish, which is trained on a high quality manually annotated
Finnish treebank [FinnTreeBank
1](http://www.ling.helsinki.fi/kieliteknologia/tutkimus/treebank/). FinnTreeBank
tagger utilizes the open-source Finnish morphological analyzer
[OMorFi](https://code.google.com/p/omorfi/) to boost tagging accuracy.

The code for FinnPos is licensed under the Apache Software Lincense
Version 2.0. External resources such as morphological analyzers,
external software and corpora, that are distributed with this code,
may be licensed under different conditions. Their licenses are located
in the appropriate directories.

Help for building, installing and using FinnPos may be found in the
FinnPos wiki

  https://github.com/mpsilfve/FinnPos/wiki