#! /bin/bash 

# This script was used to produce tdt_base from
# tdt-2013-07-18/tdt-2013-07-18.conllx
# (http://bionlp.utu.fi/fintreebank-datafiles.html) using the Turku
# Treebank Parser software Finnish-dep-parser-alpha
# (http://bionlp.utu.fi/tdtsoftware.html).
#
# Author: Miikka Silfverberg

PATH_TO_TDT_PARSER=$1

egrep -v "^#" |
cut -f2,3,6,7 |
sed 's/	\([^	]*$\)/|\1/' > wf_lemma_label

cut -f1 wf_lemma_label |
sed 's/^|$/<DIR>/'     |
sed 's/^_$/<US>/'      > wf_file

cut -f2 wf_lemma_label |
sed 's/^|$/<DIR>/'     |
sed 's/^_$/<US>/'      |
tr -d '|'              > lemma_file

cut -f3 wf_lemma_label > label_file

cat wf_file                               |
sed 's/^$/<END>/'                         |
python2 $PATH_TO_TDT_PARSER/omorfi_pos.py |
sed 's/\(	[^ ]*\) /\1	/'        |
sed 's/[+][?]/&	&/'                       |
tr ' ' '|'                                |
python3 ../../bin/omorfi2finnpos.py tdt   |
cut -f5                                   > tdt_lemma_dicts

paste wf_file lemma_file label_file tdt_lemma_dicts |
sed 's/^	*$//'                               |
sed 's/^[^	]*	/&_	/'

rm wf_lemma_label wf_file lemma_file label_file tdt_lemma_dicts