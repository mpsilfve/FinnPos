TRAINER=../../bin/finnpos-train
TAGGER=../../bin/finnpos-label
EVALER=../../bin/finnpos-eval
TDT=../../data/tdt

CONFIG_FILES=all_sl_config no_sl_config omorfi_all_sl_config omorfi_no_sl_config omorfi_ustr_sl_config ustr_sl_config

all:$(CONFIG_FILES:%=%.log)

omorfi_%_config.log:omorfi_%_config 
	$(TRAINER) $^ $(TDT)/tdt.omorfi.train+dev.feats $(TDT)/tdt.omorfi.dev.feats $^.model 2> $@
	cat $(TDT)/tdt.omorfi.test.feats.in | $(TAGGER) $^.model > $^.tagged 2>> $@
	$(EVALER) $^.tagged $(TDT)/tdt.omorfi.test.feats $^.model &>> $@
	rm $^.model
	sed -i 's/.*\r//g' $@

%_config.log:%_config 
	$(TRAINER) $^ $(TDT)/tdt.train+dev.feats $(TDT)/tdt.dev.feats $^.model 2> $@
	cat $(TDT)/tdt.test.feats.in | $(TAGGER) $^.model > $^.tagged 2>> $@
	$(EVALER) $^.tagged $(TDT)/tdt.test.feats $^.model &>> $@
	rm $^.model
	sed -i 's/.*\r//g' $@

