    >>> import finnpos
    >>> guesser = finnpos.get_label_guesser(10)
    >>> finnpos.train(['koira\tN+Sg+Nom', 'koiran\tN+Sg+Gen'], guesser)
    >>> finnpos.count_guess('kissa',1,1,guesser)
    ('N+Sg+Nom',)
    >>> finnpos.count_guess('siilin',0,1,guesser)
    ('N+Sg+Gen',)
    >>> finnpos.store(guesser, 'guesser.fp')
    >>> guesser_copy = finnpos.load('guesser.fp')
    >>> finnpos.count_guess('siilin',0,1,guesser_copy)
    ('N+Sg+Gen',)


