#! /usr/bin/env python3

from sys import stderr

SENTENCE_BOUNDARY = 'finnpos-sb'

def get_last_analysis(analysis):
    start = analysis.rfind('[WORD_ID=')
    return analysis[start:]

def fix_tag(tag):
    if tag == '' or tag[-1] == ']':
        return tag
    return tag + ']'

def split_analysis(analysis):
    tag_list = [x for x in analysis.split(']') if x != '']
    
    return [fix_tag(x) for x in tag_list]

def is_lemma_tag(tag):
    return tag.find('[WORD_ID=') != -1

def get_lemma(tag_list):    
    lemma_tags = [x for x in tag_list if is_lemma_tag(x)]
    lemmas = [x[len('[WORD_ID='):][:-len(']')] for x in lemma_tags]
    if lemmas == []:
        return '_'
    else:
        return '#'.join(lemmas)

def is_sem_tag(tag):
    return tag.find('[SEM=') != -1 or tag.find('[PROP=') != -1

def get_semantic_tags(tag_list):
    return ''.join([x for x in tag_list if is_sem_tag(x)])

def is_derivative(analysis):
    return analysis.find('[DRV=') != -1

def get_morph_label(tag_list):
    return '|'.join([x for x in tag_list if 
                     not is_sem_tag(x) and
                     not is_lemma_tag(x)])

def get_analysis_lines(stream):
    analysis_lines = []
    for line in stream:
        line = line.strip()
        if line == '':
            return analysis_lines, 0
        else:
            analysis_lines.append(line)
    return analysis_lines, 1

def split_omorfi_line(line):
    wf, analysis = '_', '_'

    if line.find('OMORFI_VERSION') != -1:
        wf = SENTENCE_BOUNDARY
    elif line.find('+?') == -1:
        wf, analysis, _weight = line.split('\t')
    elif line.find('+?') != -1:
        wf, _analysis = line.split('\t')
    return wf, analysis

from sys import stdin

if __name__=='__main__':
    while 1:
        lines, eof = get_analysis_lines(stdin)
        
        if eof:
            break

        wf = '_'
        morph_labels = []
        lemma_list = []
        sem_list = []
        
        for line in lines:
            wf, analysis = split_omorfi_line(line)
            
            if wf != '_' and analysis != '_' and not is_derivative(analysis):
                lemma = get_lemma(split_analysis(analysis))
                tags = split_analysis(get_last_analysis(analysis))
                morph_label = get_morph_label(tags)

                if morph_label == []:
                    continue

                sem_tags = get_semantic_tags(tags)
                
                lemma_list.append((morph_label, lemma))
                
                if sem_tags != '':
                    sem_list.append((morph_label, sem_tags))

                morph_labels.append(morph_label)
            elif wf != '_' and not is_derivative(analysis):
                morph_labels.append('_')

        if wf == '_':
            continue
        elif wf == SENTENCE_BOUNDARY:
            print()
        elif morph_labels == []:
            print("%s\t_\t_\t_\t_" % wf)
        else:
            annotations = '_'

            if lemma_list != []:
                annotations = str(lemma_list).replace(' ','')
                if sem_list != []:
                    annotations += ' ' + str(sem_list).replace(' ','')

            print("%s\t%s\t%s\t%s\t%s" %
                  (wf, 
                   '_', 
                   '_',
                   ' '.join(morph_labels),                   
                   annotations))

        if eof:
            break
