CXX=g++
CXXFLAGS=-Wall -Wextra -g -O3 -Wfatal-errors -Werror -std=c++0x

MODULES=io Word LemmaExtractor LabelExtractor Sentence ParamTable \
Data TrellisColumn Trellis Trainer PerceptronTrainer SGDTrainer \
TrellisCell Tagger TaggerOptions SuffixLabelMap process_aux

TESTS=$(MODULES:%=TEST_%)
OBJS=$(MODULES:%=%.o)
PROGS=finnpos-train finnpos-label finnpos-eval finnpos-print-params finnpos-filter-params finnpos-lemmatize

all:$(PROGS)

install:all
	cp $(PROGS) ../../bin

uninstall:
	rm -f $(PROGS:%=../../bin/%)

clean:
	rm -f $(OBJS) $(TESTS) $(PROGS)

doc-clean:
	rm -Rf html latex

doc: html

html:
	doxygen Doxyfile

test:$(TESTS)
	for t in $(TESTS); \
	do                 \
	echo $$t && ./$$t || echo "FAILED"; \
	done

TEST_%:$(OBJS) %.cc
	$(CXX) $(CXXFLAGS) -DTEST_$*_cc -o $@ $^

finnpos-train:finnpos-train.cc $(OBJS)
finnpos-label:finnpos-label.cc $(OBJS)
finnpos-lemmatize:finnpos-lemmatize.cc $(OBJS)
finnpos-eval:finnpos-eval.cc $(OBJS)
finnpos-print-params:finnpos-print-params.cc $(OBJS)
finnpos-filter-params:finnpos-filter-params.cc $(OBJS)
