# FinnPos - CRF-based Morphological Tagging and Lemmatization

FinnPos is a morphological tagging and lemmatization toolkit for
morphologically rich languages such as Finnish. It is based on the
[CRF](http://en.wikipedia.org/wiki/Conditional_random_field) framework
and features

* State-of-the-art tagging accuracy.
* Fast estimation and application.
* Support for large label sets exceeding 1000 label types.
* An option to use taggers for morphological disambiguation.
* Data driven lemmatization for OOV words.
* Feature extraction using sub-labels of structured morphological labels
  (e.g. "Noun+Plural+Nominative").
* Customizable feature extraction.

FinnPos also provides an accurate morphological tagger for Finnish:
FinnTreeBank tagger is trained on a high quality manually annotated
Finnish treebank [FinnTreeBank
1](http://www.ling.helsinki.fi/kieliteknologia/tutkimus/treebank/). FinnTreeBank
tagger boosts its tagging accuracy using the open-source Finnish
morphological analyzer [OMorFi](https://code.google.com/p/omorfi/).

FinnPos is licensed under the [Apache Software Lincense
v2.0](http://www.apache.org/licenses/LICENSE-2.0). External resources
such as morphological analyzers, external software and corpora, that
are distributed with this code, may be licensed under different
conditions. Their licenses are located in the appropriate directories.

Help for building, installing and using FinnPos may be found in the
FinnPos wiki

  https://github.com/mpsilfve/FinnPos/wiki
  
The toolkit was developed by Miikka Silfverberg and Teemu Ruokolainen.

References:

Miikka Silfverberg, Teemu Ruokolainen, Krister Linden. Mikko Kurimo, FinnPos: An Open-Source Morphological Tagging and Lemmatization Toolkit for Finnish. In Language Resources and Evaluation. 
