TRAINER=../../bin/finnpos-train
TAGGER=../../bin/finnpos-label
EVALER=../../bin/finnpos-eval
FTB=../../data/ftb

CONFIG_FILES=all_sl_config no_sl_config omorfi_all_sl_config omorfi_no_sl_config omorfi_ustr_sl_config ustr_sl_config

all:$(CONFIG_FILES:%=%.log)

omorfi_%_config.log:omorfi_%_config 
	$(TRAINER) $^ $(FTB)/ftb.omorfi.train+dev.feats $(FTB)/ftb.omorfi.dev.feats $^.model 2> $@
	cat $(FTB)/ftb.omorfi.test.feats.in | $(TAGGER) $^.model > $^.tagged 2>> $@
	$(EVALER) $^.tagged $(FTB)/ftb.omorfi.test.feats $^.model &>> $@
	rm $^.model
	sed -i 's/.*\r//g' $@

%_config.log:%_config 
	$(TRAINER) $^ $(FTB)/ftb.train+dev.feats $(FTB)/ftb.dev.feats $^.model 2> $@
	cat $(FTB)/ftb.test.feats.in | $(TAGGER) $^.model > $^.tagged 2>> $@
	$(EVALER) $^.tagged $(FTB)/ftb.test.feats $^.model &>> $@
	rm $^.model
	sed -i 's/.*\r//g' $@

