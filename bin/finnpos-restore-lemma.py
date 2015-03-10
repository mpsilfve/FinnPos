#! /usr/bin/env python3

from sys import stdin

for line in stdin:
    line = line.strip()

    if line == '':
        print('')
    else:
        wf, feats, lemma, label, ann = line.split('\t')

        if ann.find(' ') != -1:
            ann = ann[:ann.find(' ')]

        lemma_dict = dict(eval(ann))
        
        if label in lemma_dict:
            lemma = lemma_dict[label]
            lemma = lemma.lower()

        print('%s\t%s\t%s\t%s\t%s' % (wf, feats, lemma, label, ann))
