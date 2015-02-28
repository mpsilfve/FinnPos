FEAT_EXTRACTOR=python3 ../../bin/finnpos-ratna-feats.py freq_words

all:feats

feats:splits freq_words
	for d in train dev test train+dev; do \
	  cat ftb_base."$$d" | $(FEAT_EXTRACTOR) > ftb.omorfi."$$d".feats && \
	  cat ftb_base."$$d" | cut -f1,3,4 | $(FEAT_EXTRACTOR)               \
          > ftb."$$d".feats;                                                 \
                done
clean:
	rm -f ftb.* ftb.omorfi.* ftb_base.*
	rm -f freq_words

splits:ftb_base
	python3 ../scripts/split_data.py $^
	cat ftb_base.train ftb_base.dev > ftb_base.train+dev

freq_words:ftb_base
	cat $^                  |\
	cut -f1                 |\
	egrep "."               |\
	sort                    |\
	uniq -c                 |\
	egrep -v "^ *[0-9] "    |\
	sed 's/^ *[0-9]* *//' > $@

ftb_base:ftb_base.gz
	gunzip ftb_base.gz