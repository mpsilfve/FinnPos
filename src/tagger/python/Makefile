finnpos_label_guesser.py:_finnpos_label_guesser.so

_finnpos_label_guesser.so:LabelExtractorWrapper.o LabelExtractorWrapper_wrap.o \
Data.o io.o LabelExtractor.o ParamTable.o process_aux.o Sentence.o SuffixLabelMap.o \
TrellisCell.o Word.o
	clang++ -shared $^ -o $@ -lpython2.7 

LabelExtractorWrapper_wrap.o:LabelExtractorWrapper_wrap.cxx
	clang++ -fPIC -c $^ -o $@ -I.. -I/usr/include/python2.7

%.o:../%.cc
	clang++ -fPIC -c $^ -o $@ -I.. -I/usr/include/python2.7

LabelExtractorWrapper_wrap.cxx:LabelExtractorWrapper.i
	swig -c++ -python $^