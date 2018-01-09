#! /usr/bin/env python3

from sys import stdin, stdout, stderr, argv

HASH="<HASH>"
all_lemmas = 0

def part_count(lemma):
    return lemma.count('#')

def compile_dict(label_lemma_pairs):
    res = {}

    for label, lemma in label_lemma_pairs:
        if label in res:
            old_lemma = res[label]
            if old_lemma != lemma:
                if part_count(old_lemma) > part_count(lemma):
                    res[label] = lemma
                elif all_lemmas and part_count(old_lemma) == part_count(lemma):
                    res[label] = "%s|%s" % (old_lemma.replace("#",HASH),lemma)
        else:
            res[label] = lemma

    return res

if "--all-lemmas" in argv:
    all_lemmas = 1
elif len(argv) > 1:
    stderr.write("ERROR: Argument %s unknown.\n" % argv[1])
    exit(1)

for line in stdin:
    line = line.strip()

    if line == '':
        print('')
        stdout.flush()
    else:
        wf, feats, lemma, label, ann = line.split('\t')

        lemmas = ann
        if ann.find(' ') != -1:
            lemmas = ann[:ann.find(' ')]
            ann = ann[ann.find(' ') + 1:]
        else:
            ann = '_'
        
        lemma_dict = {}
        if lemmas != '_':
            lemma_dict = compile_dict(eval(lemmas))
        
        if label in lemma_dict:
            lemma = lemma_dict[label]
            lemma = lemma.lower()
            lemma = lemma.replace('#','')

        print('%s\t%s\t%s\t%s\t%s' % (wf, feats, lemma.replace(HASH,"#"), label, ann))
