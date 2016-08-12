    $ python
    
    import finnpos
    
    # Get label guesser that uses maximally suffixes of length 10 
    # in guessing labels.
    guesser = finnpos.get_label_guesser(10)
    
    # Train on two examples. Each example has the form "word\tlabel". 
    # The word and label shouldn't contain tabs.
    finnpos.train(['koira\tN+Sg+Nom', 'koiran\tN+Sg+Gen'], guesser)
    
    # Get label guesses for "kissa". 
    # count_guess('kissa', - input string
    #             1        - use lexicon
    #             1        - get one guess.
    #             guesser) - use the guesser we trained.
    finnpos.count_guess('kissa',1,1,guesser)
    ('N+Sg+Nom',)
    finnpos.count_guess('siilin',0,1,guesser)
    ('N+Sg+Gen',)
    
    # Store guesser in file guesser.fp.
    finnpos.store(guesser, 'guesser.fp')
    
    # Initialize guesser_copy from file guesser.fp
    guesser_copy = finnpos.load('guesser.fp')
    finnpos.count_guess('siilin',0,1,guesser_copy)
    ('N+Sg+Gen',)


