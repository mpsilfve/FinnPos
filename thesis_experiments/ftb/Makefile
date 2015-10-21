SHELL=/bin/bash
TRAINER=../../bin/finnpos-train
TAGGER=../../bin/finnpos-label
EVALER=../../bin/finnpos-eval
FTB=../../data/ftb

CONFIG_FILES=all_sl_config no_sl_config omorfi_all_sl_config omorfi_no_sl_config omorfi_ustr_sl_config ustr_sl_config lemma_config

all:$(CONFIG_FILES:%=%.log)
	rm *.model

clean:
	rm -f *.model *.tagged *.log

omorfi_%_config.log:omorfi_%_config 
	echo TRAINING LOG >> $@
	(time $(TRAINER) $^ $(FTB)/ftb.omorfi.train+dev.feats $(FTB)/ftb.omorfi.dev.feats $^.model) 2>> $@
	echo LOAD MODEL LOG >> $@
	echo | (time $(TAGGER) $^.model) > $^.tagged 2>> $@
	echo LABEL LOG >> $@
	cat $(FTB)/ftb.omorfi.test.feats.in | (time $(TAGGER) $^.model) > $^.tagged 2>> $@
	echo EVAL LOG >> $@
	$(EVALER) $^.tagged $(FTB)/ftb.omorfi.test.feats $^.model &>> $@
	sed -i 's/.*\r//g' $@

%_config.log:%_config 
	echo TRAINING LOG >> $@
	(time $(TRAINER) $^ $(FTB)/ftb.train+dev.feats $(FTB)/ftb.dev.feats $^.model) 2> $@
	echo LOAD MODEL LOG >> $@
	echo | (time $(TAGGER) $^.model) > $^.tagged 2>> $@
	echo LABEL LOG >> $@
	cat $(FTB)/ftb.test.feats.in | (time $(TAGGER) $^.model) > $^.tagged 2>> $@
	echo EVAL LOG >> $@
	$(EVALER) $^.tagged $(FTB)/ftb.test.feats $^.model &>> $@
	sed -i 's/.*\r//g' $@

lemma_config.log:lemma_config
	echo TRAINING LEMMATIZER LOG >> $@
	(time $(TRAINER) $^ $(FTB)/ftb.train+dev.feats $(FTB)/ftb.dev.feats $^.model) 2> $@
	sed -i 's/.*\r//g' $@

