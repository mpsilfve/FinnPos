SHELL=/bin/bash
INSTALL_DIR=/usr/local
LOGF=lre-experiments.log

all:finnpos

clean:
	make -C ./data clean
	make -C ./src uninstall
	make -C ./data clean
	rm -f ftb.test.sys ftb.omorfi.test.sys
	rm -f tdt.test.sys tdt.omorfi.test.sys
	rm -f share/ftb_model/ftb.model
	rm -f share/ftb_omorfi_model/ftb.omorfi.model
	rm -f share/tdt_model/tdt.model
	rm -f share/tdt_omorfi_model/tdt.omorfi.model

finnpos:
	make -C src
	make -C src install

lre-experiments:ftb-tagger-lre tdt-tagger-lre ftb-omorfi-tagger-lre \
tdt-omorfi-tagger-lre

ftb-tagger:finnpos share/omorfi/morphology.omor.hfst
	make -C data/ftb
	mkdir -p share/ftb_model
	bin/finnpos-train data/ftb/config data/ftb/ftb.train+dev.feats \
        data/ftb/ftb.dev.feats share/ftb_model/ftb.model

ftb-tagger-lre:finnpos
	echo FTB WITHOUT OMORFI > $(LOGF)
	echo >> $(LOGF)
	mkdir -p share/ftb_model
	make -C data/ftb
	(time bin/finnpos-train data/ftb/config data/ftb/ftb.train+dev.feats \
        data/ftb/ftb.dev.feats share/ftb_model/ftb.model 2>&1) | \
	sed "s/.*$$(printf '\r')//" | tee -a $(LOGF)
	echo >> $(LOGF)
	echo Tagging >> $(LOGF)
	cat data/ftb/ftb.test.feats.in | \
        (time bin/finnpos-label share/ftb_model/ftb.model) > ftb.test.sys \
	2>> $(LOGF)
	bin/finnpos-eval data/ftb/ftb.test.feats ftb.test.sys \
	share/ftb_model/ftb.model &>> $(LOGF)
	echo >> $(LOGF)

ftb-omorfi-tagger-lre:finnpos
	echo FTB WITH OMORFI >> $(LOGF)
	echo >> $(LOGF)
	mkdir -p share/ftb_omorfi_model
	make -C data/ftb
	(time bin/finnpos-train data/ftb/omorfi_config \
        data/ftb/ftb.omorfi.train+dev.feats data/ftb/ftb.omorfi.dev.feats \
        share/ftb_omorfi_model/ftb.omorfi.model) 2>&1 | \
	sed "s/.*$$(printf '\r')//" | tee -a $(LOGF)
	echo >> $(LOGF)
	echo Tagging >> $(LOGF)
	cat data/ftb/ftb.omorfi.test.feats.in | \
        (time bin/finnpos-label share/ftb_omorfi_model/ftb.omorfi.model) > ftb.omorfi.test.sys \
	2>> $(LOGF)
	bin/finnpos-eval data/ftb/ftb.omorfi.test.feats ftb.omorfi.test.sys \
	share/ftb_omorfi_model/ftb.omorfi.model &>> $(LOGF)
	echo >> $(LOGF)

tdt-tagger-lre:finnpos
	echo TDT WITHOUT OMORFI >> $(LOGF)
	echo >> $(LOGF)
	mkdir -p share/tdt_model
	make -C data/tdt
	(time bin/finnpos-train data/tdt/config data/tdt/tdt.train+dev.feats \
        data/tdt/tdt.dev.feats share/tdt_model/tdt.model) 2>&1 | \
	sed "s/.*$$(printf '\r')//"  | tee -a $(LOGF)
	echo >> $(LOGF)
	echo Tagging >> $(LOGF)
	cat data/tdt/tdt.test.feats.in | \
        (time bin/finnpos-label share/tdt_model/tdt.model) > tdt.test.sys \
	2>> $(LOGF)
	bin/finnpos-eval data/tdt/tdt.test.feats tdt.test.sys \
	share/tdt_model/tdt.model &>> $(LOGF)
	echo >> $(LOGF)

tdt-omorfi-tagger-lre:finnpos
	echo TDT WITH OMORFI >> $(LOGF)
	echo >> $(LOGF)
	mkdir -p share/tdt_omorfi_model
	make -C data/tdt
	(time bin/finnpos-train data/tdt/omorfi_config\
        data/tdt/tdt.omorfi.train+dev.feats data/tdt/tdt.omorfi.dev.feats \
        share/tdt_omorfi_model/tdt.omorfi.model) 2>&1 | \
	sed "s/.*$$(printf '\r')//"  | tee -a $(LOGF)
	echo >> $(LOGF)
	echo Tagging >> $(LOGF)
	cat data/tdt/tdt.omorfi.test.feats.in | \
        (time bin/finnpos-label share/tdt_omorfi_model/tdt.omorfi.model) > tdt.omorfi.test.sys \
	2>> $(LOGF)
	bin/finnpos-eval data/tdt/tdt.omorfi.test.feats tdt.omorfi.test.sys \
	share/tdt_omorfi_model/tdt.omorfi.model &>> $(LOGF)
	echo >> $(LOGF)

install:finnpos
	mkdir -p $(INSTALL_DIR)/share/finnpos
	mkdir -p $(INSTALL_DIR)/bin

	for b in $(BINARIES);                              \
		do cp "$$b" $(INSTALL_DIR)/bin  &&         \
		chmod u+rwx a+rx $(INSTALL_DIR)/bin/"$$b"; \
	done

	cp -R share $(INSTALL_DIR)/share/finnpos
	chmod u+rwx a+rx $(INSTALL_DIR)/share/finnpos
	chmod u+rwx a+rx $(INSTALL_DIR)/share/finnpos/*

uninstall:
	for b in $(BINARIES);                       \
		do rm -f $(INSTALL_DIR)/bin/"$$b";  \
	done

	rm -Rf $(INSTALL_DIR)/share/finnpos

