from sys import stdin, argv, stderr
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

def convert(ifile, convert_type):
    wf      = ''
    labels  = ''
    lemmas  = []

    for line in ifile:
        line = line.strip()

        if line == '' and wf != '':
            lemmas = list(set(lemmas))
            lemma_str = str(lemmas).replace(' ','')
            print('%s\t%s\t%s\t%s\t%s' % (wf, '_', '_', labels, lemma_str))

            wf, labels, lemmas = '', '', []
            
        elif line == '':
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
                    labels = '_'
                else:
                    label = get_label(analysis, convert_type)
                    lemma = get_lemma(analysis, convert_type)
            
                    if labels != '':
                        labels += ' '
                
                    labels += label
                    lemmas.append((label, lemma))
            else:
                wf, lemma, label = line.split('\t')

                lemma = lemma.replace('|','')

                if label == '+?':
                    labels = '_'

                else:
                    if labels != '':
                        labels += ' '
                
                    labels += label
                    lemmas.append((label, lemma))

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

    convert(stdin, convert_type)
