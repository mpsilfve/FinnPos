#! /usr/bin/env python3

from sys import argv, stderr, stdout
from collections import defaultdict
import operator

def fix_casing(label):
    label = label.lower()

    return label[0].upper() + label[1:]

def print_err_mat(err_mat, sorted_errs):
    total_errors = sum([x[1] for x in sorted_errs]) * 1.0

    print('\\begin{table}[!h]')
    print('\\begin{center}')
    print('\\begin{tabular}{l|%s}' % ('c' * 5)) 

    stdout.write(' & ')
    for i, l_et_c in enumerate(sorted_errs):        
        gold_label, _count = l_et_c
        if i == 4:
            stdout.write(fix_casing(gold_label) + '\\\\\n')
            break
        else:
            stdout.write(fix_casing(gold_label) + ' & ')

    print('\\hline')
    print('\\noalign{\smallskip}')
    row_count = 0
    for gold_label, _gold_count in sorted_errs:
        if row_count == 5:
            break
        stdout.write('%s & ' % (fix_casing(gold_label)))
        for i, l_et_c in enumerate(sorted_errs):
            sys_label, _sys_count = l_et_c
            cell = '%.1f' % (err_mat[gold_label][sys_label] 
                             * 100.0 / total_errors)
            if i == 4:
                stdout.write(cell + '\\\\\n')
                break
            else:
                stdout.write(cell + ' & ')
        row_count += 1
   
    print('\\end{tabular}')
    print('\\caption{}')
    print('\\end{center}')
    print('\\end{table}')    
    
def print_tabular_errs(err_list):
    total_errors = sum([x[1] for x in err_list]) * 1.0

    print('\\begin{table}[!h]')
    print('\\begin{center}')
    print('\\begin{tabular}{lcc}') 

    print('\\noalign{\smallskip}')
    print('         & absolute & relative\\\\')
    print('main POS & frequency    & frequency (\\%)\\\\')
    print('\\hline')
    print('\\noalign{\smallskip}')

    for i, l_et_c in enumerate(err_list):
        label, count = l_et_c
        line = "%s & %u & %.1f" % (fix_casing(label), count, 100 * count / total_errors)
        if i + 1 == len(err_list):
            print(line)
        else:
            print(line + '\\\\')

    print('\\end{tabular}')
    print('\\caption{}')
    print('\\end{center}')
    print('\\end{table}')    

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
    
def get_labels(f):
    labels = []

    for line in f:
        line = line.strip()

        if line == '':
            continue

        _wf, _feats, _lemma, label, _ann = line.split('\t')
        labels.append(label)
        
    return labels

if __name__=='__main__':
    if len(argv) != 5:
        stderr.write("%s ftb_sys_file ftb_gold_file tdt_sys_file tdt_gold_file\n" % argv[0])
        exit(1)

    ftb_sys_labels  = get_labels(open(argv[1]))
    ftb_gold_labels = get_labels(open(argv[2]))
    tdt_sys_labels  = get_labels(open(argv[3]))
    tdt_gold_labels = get_labels(open(argv[4]))

    assert(len(ftb_sys_labels) == len(ftb_gold_labels))
    assert(len(tdt_sys_labels) == len(tdt_gold_labels))

    ftb_errs = defaultdict(lambda : 0.0)
    tdt_errs = defaultdict(lambda : 0.0)
    ftb_err_mat = defaultdict(lambda : defaultdict(lambda : 0.0))
    tdt_err_mat = defaultdict(lambda : defaultdict(lambda : 0.0))

    tot = len(ftb_sys_labels)
    corr = 0.0

    for i, sys_label in enumerate(ftb_sys_labels):
        gold_label = ftb_gold_labels[i]

        if sys_label != gold_label:
            mpos_sys = get_main_pos(sys_label, 0)
            mpos_gold = get_main_pos(gold_label, 0)

            ftb_errs[mpos_gold] += 1
            ftb_err_mat[mpos_gold][mpos_sys] += 1
        else:
            corr += 1

    print("FTB ACC: %.2f%%" % (100 * corr / tot))
    
    tot = len(tdt_sys_labels)
    corr = 0

    for i, sys_label in enumerate(tdt_sys_labels):
        gold_label = tdt_gold_labels[i]

        if sys_label != gold_label:
            mpos_sys = get_main_pos(sys_label, 1)
            mpos_gold = get_main_pos(gold_label, 1)

            tdt_errs[mpos_gold] += 1
            tdt_err_mat[mpos_gold][mpos_sys] += 1
        else:
            corr += 1

    print("TDT ACC: %.2f%%" % (100 * corr / tot))

    sorted_ftb_errs = sorted(ftb_errs.items(), 
                             key=operator.itemgetter(1))
    sorted_ftb_errs.reverse()
    

    sorted_tdt_errs = sorted(tdt_errs.items(), 
                             key=operator.itemgetter(1))
    sorted_tdt_errs.reverse()

    print_tabular_errs(sorted_tdt_errs)
    print()
    print_tabular_errs(sorted_ftb_errs)
    print()
    print_err_mat(tdt_err_mat, sorted_tdt_errs)
    print()
    print_err_mat(ftb_err_mat, sorted_ftb_errs)


