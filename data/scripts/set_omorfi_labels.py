from sys import stdin

for line in stdin:
    line = line.strip()

    if line == '':
        print ('')
    else:
        wf, feats, lemma, label, ann = line.split('\t')

        ann_list = eval(ann)

        if ann_list != []:
            label_list = [ label for label, lemma in ann_list ]

            if label == '_':
                label = ' '.join(label_list)
            else:                
                label_set = set(label_list)

                if label in label_set:
                    label_set.remove(label)
            
                label = label + ' ' + ' '.join(list(label_set))

        print ('%s\t%s\t%s\t%s\t%s' % (wf, feats, lemma, label, ann))
