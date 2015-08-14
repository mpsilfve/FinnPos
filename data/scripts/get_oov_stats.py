#! /usr/bin/env python3

from sys import argv, stderr, stdout
from collections import defaultdict
import operator

def fix_casing(label):
    label = label.lower()

    return label[0].upper() + label[1:]

def get_main_pos(label, is_tdt):
    if is_tdt:
        if label.find('|') == -1:
            return label
        label = label[:label.find('|')]
    else:
        if label.find('[POS=') == -1:
            return None

        label = label[label.find('[POS=') + 5:]
        label = label[:label.find(']')]

    return label

def get_wls(f, is_tdt):
    wls = []

    for line in f:
        line = line.strip()

        if line == '':
            continue

        wf, _feats, _lemma, label, _ann = line.split('\t')
        wls.append((wf, get_main_pos(label, is_tdt)))
        
    return wls

def print_counts(sorted_all_counts, oov_counts):
    total_all = sum([x[1] for x in sorted_all_counts])
    total_oov = sum([x[1] for x in oov_counts.items()])

    print('\\begin{table}[!h]')
    print('\\begin{center}')
    print('\\begin{tabular}{l|cc|cc}') 

    print('\\noalign{\smallskip}')
    print('      & \multicolumn{2}{|c|}{all words} & \multicolumn{2}{c}{oov words}\\\\')
    print('      & absolute  & relative & absolute & relative\\\\')
    print('label & frequency & frequency (\\%) & frequency & frequency (\\%)\\\\')
    print('\\hline')
    print('\\noalign{\smallskip}')
    
    for i, l_et_c in enumerate(sorted_all_counts):
        label, count = l_et_c
        oov_count = oov_counts[label]
        line = "%s & %u & %.1f & %u & %.1f" % (
            fix_casing(label),
            count, 
            count * 100.0 / total_all,
            oov_count,
            oov_count * 100.0 / total_oov)

        if i + 1 == len(sorted_all_counts):
            print(line)
        else:
            print("%s\\\\" % line)
    print('\\end{tabular}')
    print('\\caption{}')
    print('\\end{center}')
    print('\\end{table}')    

if __name__=='__main__':
    if len(argv) != 5:
        stderr.write("%s ftb_train_set ftb_test_set tdt_train_set tdt_test_set\n" % argv[0])
        exit(1)

    ftb_train_wls  = get_wls(open(argv[1]), 0)
    ftb_test_wls = get_wls(open(argv[2]), 0)
    tdt_train_wls  = get_wls(open(argv[3]), 1)
    tdt_test_wls = get_wls(open(argv[4]), 1)

    ftb_train_wf_set = set([x[0] for x in ftb_train_wls])

    ftb_oov_counts = defaultdict(lambda : 0.0)
    ftb_all_counts = defaultdict(lambda : 0.0)

    ftb_not_found = 0.0
    ftb_total_test = len(ftb_test_wls)

    for wf, label in ftb_test_wls:
        if not wf in ftb_train_wf_set:
            ftb_not_found += 1
            ftb_oov_counts[label] += 1
        ftb_all_counts[label] += 1

    print("FTB OOV: %u (%.1f)" % 
          (ftb_not_found, ftb_not_found * 100.0 / ftb_total_test))

    ftb_sorted_all_counts = sorted(ftb_all_counts.items(), 
                                   key=operator.itemgetter(1))
    ftb_sorted_all_counts.reverse()

    print_counts(ftb_sorted_all_counts, ftb_oov_counts)

    tdt_train_wf_set = set([x[0] for x in tdt_train_wls])

    tdt_oov_counts = defaultdict(lambda : 0.0)
    tdt_all_counts = defaultdict(lambda : 0.0)

    tdt_not_found = 0.0
    tdt_total_test = len(tdt_test_wls)

    for wf, label in tdt_test_wls:
        if not wf in tdt_train_wf_set:
            tdt_not_found += 1
            tdt_oov_counts[label] += 1
        tdt_all_counts[label] += 1

    print("TDT OOV: %u (%.1f)" % 
          (tdt_not_found, tdt_not_found * 100.0 / tdt_total_test))

    tdt_sorted_all_counts = sorted(tdt_all_counts.items(), 
                                   key=operator.itemgetter(1))
    tdt_sorted_all_counts.reverse()

    print_counts(tdt_sorted_all_counts, tdt_oov_counts)
