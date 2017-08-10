SHELL=/bin/bash
INSTALL_DIR=/usr/local
LOGF=lre-experiments.log
REV=$(shell git describe --tags)


all:finnpos

clean:
	make -C ./data clean
	make -C ./src uninstall
	make -C ./src clean
	rm -f ftb.test.sys ftb.omorfi.test.sys
	rm -f tdt.test.sys tdt.omorfi.test.sys
	rm -Rf experiments
	rm -f lre-experiments.log

finnpos:
	make -C src
	make -C src install

lre-experiments:ftb-tagger-lre tdt-tagger-lre ftb-omorfi-tagger-lre \
tdt-omorfi-tagger-lre

ftb-omorfi-tagger:finnpos share/finnpos/omorfi/morphology.omor.hfst
	make -C data/ftb
	mkdir -p share/finnpos/ftb_omorfi_model
	bin/finnpos-train data/ftb/config data/ftb/ftb.omorfi.train+dev.feats \
        data/ftb/ftb.omorfi.dev.feats share/finnpos/ftb_omorfi_model/ftb.omorfi.model
	cp data/ftb/freq_words share/finnpos/ftb_omorfi_model
	cat src/scripts/ftb-label | sed "s/REVISION/$(REV)/" > bin/ftb-label
	chmod 755 bin/ftb-label

ftb-tagger-lre:finnpos
	echo FTB WITHOUT OMORFI > $(LOGF)
	echo >> $(LOGF)
	mkdir -p experiments/ftb_model
	make -C data/ftb
	(time bin/finnpos-train data/ftb/config data/ftb/ftb.train+dev.feats \
        data/ftb/ftb.dev.feats experiments/ftb_model/ftb.model 2>&1) | \
	sed "s/.*$$(printf '\r')//" | tee -a $(LOGF)
	echo >> $(LOGF)
	echo Tagging >> $(LOGF)
	cat data/ftb/ftb.test.feats.in | \
        (time bin/finnpos-label experiments/ftb_model/ftb.model) > ftb.test.sys \
	2>> $(LOGF)
	bin/finnpos-eval data/ftb/ftb.test.feats ftb.test.sys \
	experiments/ftb_model/ftb.model &>> $(LOGF)
	echo >> $(LOGF)

ftb-omorfi-tagger-lre:finnpos
	echo FTB WITH OMORFI >> $(LOGF)
	echo >> $(LOGF)
	mkdir -p experiments/ftb_omorfi_model
	make -C data/ftb
	(time bin/finnpos-train data/ftb/omorfi_config \
        data/ftb/ftb.omorfi.train+dev.feats data/ftb/ftb.omorfi.dev.feats \
        experiments/ftb_omorfi_model/ftb.omorfi.model) 2>&1 | \
	sed "s/.*$$(printf '\r')//" | tee -a $(LOGF)
	echo >> $(LOGF)
	echo Tagging >> $(LOGF)
	cat data/ftb/ftb.omorfi.test.feats.in | \
        (time bin/finnpos-label experiments/ftb_omorfi_model/ftb.omorfi.model) |\
	python3 bin/finnpos-restore-lemma.py > ftb.omorfi.test.sys \
	2>> $(LOGF)
	bin/finnpos-eval data/ftb/ftb.omorfi.test.feats ftb.omorfi.test.sys \
	experiments/ftb_omorfi_model/ftb.omorfi.model &>> $(LOGF)
	echo >> $(LOGF)

tdt-tagger-lre:finnpos
	echo TDT WITHOUT OMORFI >> $(LOGF)
	echo >> $(LOGF)
	mkdir -p experiments/tdt_model
	make -C data/tdt
	(time bin/finnpos-train data/tdt/config data/tdt/tdt.train+dev.feats \
        data/tdt/tdt.dev.feats experiments/tdt_model/tdt.model) 2>&1 | \
	sed "s/.*$$(printf '\r')//"  | tee -a $(LOGF)
	echo >> $(LOGF)
	echo Tagging >> $(LOGF)
	cat data/tdt/tdt.test.feats.in | \
        (time bin/finnpos-label experiments/tdt_model/tdt.model) > tdt.test.sys \
	2>> $(LOGF)
	bin/finnpos-eval data/tdt/tdt.test.feats tdt.test.sys \
	experiments/tdt_model/tdt.model &>> $(LOGF)
	echo >> $(LOGF)

tdt-omorfi-tagger-lre:finnpos
	echo TDT WITH OMORFI >> $(LOGF)
	echo >> $(LOGF)
	mkdir -p experiments/tdt_omorfi_model
	make -C data/tdt
	(time bin/finnpos-train data/tdt/omorfi_config\
        data/tdt/tdt.omorfi.train+dev.feats data/tdt/tdt.omorfi.dev.feats \
        experiments/tdt_omorfi_model/tdt.omorfi.model) 2>&1 | \
	sed "s/.*$$(printf '\r')//"  | tee -a $(LOGF)
	echo >> $(LOGF)
	echo Tagging >> $(LOGF)
	cat data/tdt/tdt.omorfi.test.feats.in | \
        (time bin/finnpos-label experiments/tdt_omorfi_model/tdt.omorfi.model)  |\
	python3 bin/finnpos-restore-lemma.py > tdt.omorfi.test.sys \
	2>> $(LOGF)
	bin/finnpos-eval data/tdt/tdt.omorfi.test.feats tdt.omorfi.test.sys \
	experiments/tdt_omorfi_model/tdt.omorfi.model &>> $(LOGF)
	echo >> $(LOGF)

install:finnpos
	mkdir -m 755 -p $(INSTALL_DIR)/share/finnpos
	mkdir -m 755 -p $(INSTALL_DIR)/share/finnpos/omorfi
	mkdir -m 755 -p $(INSTALL_DIR)/share/finnpos/ftb_omorfi_model
	mkdir -m 755 -p $(INSTALL_DIR)/bin

	install -m 755 bin/* $(INSTALL_DIR)/bin

install-models:ftb-omorfi-tagger
	install -m 755 share/finnpos/omorfi/* $(INSTALL_DIR)/share/finnpos/omorfi
	install -m 755 share/finnpos/ftb_omorfi_model/* $(INSTALL_DIR)/share/finnpos/ftb_omorfi_model
	install -m 755 share/finnpos/tdt_omorfi_model/* $(INSTALL_DIR)/share/finnpos/tdt_omorfi_model

uninstall:
	rm -f $(INSTALL_DIR)/bin/finnpos-train $(INSTALL_DIR)/bin/finnpos-label
	rm -f $(INSTALL_DIR)/bin/finnpos-eval
	rm -f $(INSTALL_DIR)/bin/finnpos-ratna-feats.py $(INSTALL_DIR)/bin/ftb-label
	rm -f $(INSTALL_DIR)/bin/omorfi2finnpos.py
	rm -f $(INSTALL_DIR)/bin/finnpos-restore-lemma.py
	rm -Rf $(INSTALL_DIR)/share/finnpos
