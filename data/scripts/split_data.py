#! /usr/bin/env python3


from sys import argv, stderr

def make_splits(dataf, trainf, devf, testf):
    i = 1

    for line in dataf:
        if i % 10 == 9:
            devf.write(line)
        elif i % 10 == 0:
            testf.write(line)
        else:
            trainf.write(line)

        line = line.strip()

        if line == '':
            i += 1

if __name__=='__main__':
    if len(argv) != 2:
        stderr.write("USE: %s data_file\n" % argv[0])
        exit(1)

    dataf  = open(argv[1])
    trainf = open(argv[1] + ".train", "w")
    devf   = open(argv[1] + ".dev", "w")
    testf  = open(argv[1] + ".test", "w")

    make_splits(dataf, trainf, devf, testf)
