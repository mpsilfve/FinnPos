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

##Build and Installation
###Requirements

Installation is straightforward on a Debian based Linux distribution, such as Ubuntu or Linux Mint (tested in Ubuntu 12.04.5 LTS), or Mac OS X. There are very few requirements, so installation should be fairly easy on other systems as well, e.g. Windows or RedHat Linux.

You need a C++ compiler (tested with Ubuntu clang version 3.0-6ubuntu3). Compatible versions of clang++ and g++ should work out of the box.

You also need a Python 3 interpreter (tested with Python 3.2.3).

Finally, you need git for cloning the FinnPos repository.

###Getting the source code

You first need to clone the FinnPos repository

'git clone https://github.com/mpsilfve/FinnPos'

###Building and installing FinnPos

If you want to use g++ instead of clang++, change the line

'CXX=clang++'

in 
 * 'FinnPos/src/tagger/Makefile' and
 * 'FinnPos/src/hfst-optimized-lookup-1.3/Makefile' into

'CXX=g++'

In the directory FinnPos, run

make

By default, FinnPos utilities are installed in /usr/local/bin and FinnPos models in /usr/local/share/finnpos. If you are happy with these default directories, install by running

'sudo make install'

in the directory FinnPos. If you would rather install under another directory, run

'sudo make install -e INSTALL_DIR=path_to_directory'

### Building and installing FinnTreeBank tagger

FinnPos includes training data for a statistical tagger and lemmatizer for Finnish called FinnTreeBank tagger. FinnTreeBank tagger requires the OMorFi morphological analyzer. You can get the morphological analyzer as an HFST transducer from the FinnPos release page.

Download morphology.omor.hfst.gz from the release page, extract the gz archive and place the binary morphology.omor.hfst in directory FinnPos/share/finnpos/omorfi/

The run make, make ftb-omorfi-tagger and finally sudo make install. If you do not want to install in the default install directory /usr/local/, use the -e INSTALL_DIR=path_to_directory option with sudo make install.
