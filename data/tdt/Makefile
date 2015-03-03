FEAT_EXTRACTOR=python3 ../../bin/finnpos-ratna-feats.py freq_words

all:feats

feats:tdt.train.feats

tdt.train.feats:tdt_base.train freq_words
	for d in train dev test train+dev; do \
	  cat tdt_base."$$d" | $(FEAT_EXTRACTOR) > tdt.omorfi."$$d".feats && \
	  cat tdt_base."$$d" | cut -f1,3,4 | $(FEAT_EXTRACTOR)               \
          > tdt."$$d".feats;                                                 \
          done

	cat tdt.test.feats | ../scripts/detag.sh > tdt.test.feats.in
	cat tdt.omorfi.test.feats | ../scripts/detag.sh | \
	python3 ../scripts/set_omorfi_labels.py > tdt.omorfi.test.feats.in

clean:
	rm -f tdt.* tdt.omorfi.* tdt_base.*
	rm -f freq_words

tdt_base.train:tdt_base
	python3 ../scripts/split_data.py $^
	cat tdt_base.train tdt_base.dev > tdt_base.train+dev

freq_words:tdt_base
	cat $^                  |\
	cut -f1                 |\
	egrep "."               |\
	sort                    |\
	uniq -c                 |\
	egrep -v "^ *[0-9] "    |\
	sed 's/^ *[0-9]* *//' > $@

