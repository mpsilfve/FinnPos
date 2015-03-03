#! /bin/bash

# Throw away the gold label and lemma but preserve annotations.

sed 's/	[^	]*	[^	]*	\([^	]*\)$/	_	_	\1/'