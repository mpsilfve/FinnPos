from sys import stdin, argv, stderr, stdout
from re import findall

def get_lemma(string, convert_type):
    if convert_type == 'ftb':
        word_id_strs = findall('\[WORD_ID=[^\]]*\]', string)
        lemma_parts = [ word_id_str[9:][:-1] for word_id_str in word_id_strs ]
        return '#'.join(lemma_parts)
    else:
        return string[:string.find('\t')]

def get_label(string, convert_type):
    if convert_type == 'ftb':
        # Remove everything up to the start of the last lemma.
        string = string[string.rfind('[WORD_ID=') + len('[WORD_ID='):]
    
        # Remove the last lemma.
        label = string[string.find(']') + 1:]

        # Add sub label separators.
        label = label.replace('][',']|[')

        sub_labels = label.split('|')

        sub_labels = filter(lambda x: x.find("STYLE=") == -1, sub_labels)
        sub_labels = filter(lambda x: x.find("DRV=") == -1, sub_labels)
        
        label = '|'.join(sub_labels)

        return label

    else:
        return string[string.find('\t'):]

def get_lemmas(analyses, convert_type):
    return [(get_label(a, convert_type), get_lemma(a, convert_type)) 
            for a in analyses]

def get_labels(analyses, convert_type):
    return [get_label(a, convert_type) for a in analyses]

def filter_ftb_analyses(analyses):
    min_wbs = min(map(lambda x: x.count('[WORD_ID='), analyses))
    return list(filter(lambda x: x.count('[WORD_ID=') == min_wbs, analyses))

def convert(pname, ifile, convert_type):
    wf       = ''
    analyses = []

    stderr.write("%s: Reading from STDIN. Writing to STDOUT\n" % (pname))

    for line in ifile:
        line = line.strip()

        if line == '' and wf != '':

            if convert_type == 'ftb' and len(analyses) > 0:
                analyses = filter_ftb_analyses(analyses)

            lemmas = get_lemmas(analyses, convert_type)
            lemmas = list(set(lemmas))

            lemma_str = str(lemmas).replace(' ','')

            labels = get_labels(analyses, convert_type)

            feats = '_'

            if labels != []:
                label_feats = map(lambda x: "OMORFI_FEAT:" + x, labels)
                feats = ' '.join(label_feats)

            label_str = '_' 

            # Make sure that finnpos disambiguates if we got some
            # analyses from OMorFi.
            if labels != []:
                label_str = ' '.join(labels)
                
            print('%s\t%s\t%s\t%s\t%s' % (wf, feats, '_', label_str, lemma_str))

            wf, analyses = '', []
            
        elif line == '':
            stdout.flush()
            continue

        elif (convert_type == 'ftb' and 
              line == 'OMORFI_VERSION_≥_14_©_GNU_GPL_V3'):
            print('')
            entry = ''

        elif convert_type == 'tdt' and line.find('<END>') != -1:
            print('')
            entry = ''

        else:
            if convert_type == 'ftb':
                wf, analysis = line.split('\t')

                if analysis == '+?':
                    analyses = []
                else:
                    analyses.append(analysis)
            else:
                wf, lemma, label = line.split('\t')

                if label == '+?':
                    analyses = []
                else:
                    analyses.append(lemma + '\t' + label)

if __name__=='__main__':

    convert_type = 'ftb'

    if len(argv) == 2:
        convert_type = argv[1]
    elif len(argv) != 1:
        stderr.write('USE: cat indata | %s (ftb|tdt) > outdata\n' % argv[0])
        exit(1)
        
    if not convert_type in ['ftb','tdt']:
        stderr.write('Unknown conversion type %s. Should be ftb or tdt.' % 
                     convert_type)
        exit(1)

    convert(argv[0], stdin, convert_type)
