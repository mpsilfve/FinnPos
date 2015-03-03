FEAT_EXTRACTOR=python3 ../../bin/finnpos-ratna-feats.py freq_words

all:feats

feats:ftb.train.feats

ftb.train.feats:ftb_base.train freq_words
	for d in train dev test train+dev; do \
	  cat ftb_base."$$d" | $(FEAT_EXTRACTOR) > ftb.omorfi."$$d".feats && \
	  cat ftb_base."$$d" | cut -f1,3,4 | $(FEAT_EXTRACTOR)               \
          > ftb."$$d".feats;                                                 \
        done

	cat ftb.test.feats | ../scripts/detag.sh > ftb.test.feats.in
	cat ftb.omorfi.test.feats | ../scripts/detag.sh | \
	python3 ../scripts/set_omorfi_labels.py > ftb.omorfi.test.feats.in

clean:
	rm -f ftb.* ftb.omorfi.* ftb_base.*
	rm -f freq_words

ftb_base.train:ftb_base
	python3 ../scripts/split_data.py $^
	cat ftb_base.train ftb_base.dev > ftb_base.train+dev

freq_words:ftb_base.train 
	cat ftb_base.train ftb_base.dev |\
	cut -f1                         |\
	egrep "."                       |\
	sort                            |\
	uniq -c                         |\
	egrep -v "^ *[0-9] "            |\
	sed 's/^ *[0-9]* *//'           > $@

