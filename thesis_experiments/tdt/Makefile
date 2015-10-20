TRAINER=../../bin/finnpos-train
TAGGER=../../bin/finnpos-label
EVALER=../../bin/finnpos-eval
TDT=../../data/tdt

CONFIG_FILES=all_sl_config no_sl_config omorfi_all_sl_config omorfi_no_sl_config omorfi_ustr_sl_config ustr_sl_config

all:$(CONFIG_FILES:%=%.log)
	rm *.model

omorfi_%_config.log:omorfi_%_config 
	(time $(TRAINER) $^ $(TDT)/tdt.omorfi.train+dev.feats $(TDT)/tdt.omorfi.dev.feats $^.model 2> $@)
	cat $(TDT)/tdt.omorfi.test.feats.in | (time $(TAGGER) $^.model > $^.tagged &>> $@)
	$(EVALER) $^.tagged $(TDT)/tdt.omorfi.test.feats $^.model &>> $@
	sed -i 's/.*\r//g' $@

%_config.log:%_config 
	(time $(TRAINER) $^ $(TDT)/tdt.train+dev.feats $(TDT)/tdt.dev.feats $^.model 2> $@)
	cat $(TDT)/tdt.test.feats.in | (time $(TAGGER) $^.model > $^.tagged &>> $@)
	$(EVALER) $^.tagged $(TDT)/tdt.test.feats $^.model &>> $@
	sed -i 's/.*\r//g' $@

