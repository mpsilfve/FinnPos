SHELL=/bin/bash
TRAINER=../../bin/finnpos-train
TAGGER=../../bin/finnpos-label
EVALER=../../bin/finnpos-eval
FTB=../../data/ftb

CONFIG_FILES=all_sl_config beam_0.0_config beam_0.5_config beam_0.7_config beam_0.999_config beam_0.99_config beam_0.9_config beam_10_config beam_15_config beam_1_config beam_20_config beam_5_config beam_infty_config guesses_0.5_config guesses_0.7_config guesses_0.999_config guesses_0.99_config guesses_0.9_config guesses_10_config guesses_15_config guesses_1_config guesses_20_config guesses_25_config guesses_30_config guesses_35_config guesses_40_config guesses_45_config guesses_5_config lemma_config no_sl_config omorfi_all_sl_config omorfi_beam_0.0_config omorfi_beam_0.999_config omorfi_no_sl_config omorfi_ustr_sl_config ustr_sl_config

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

