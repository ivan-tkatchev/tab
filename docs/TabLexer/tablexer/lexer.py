# -*- coding: utf-8 -*-
"""
    pygments.lexers.tab
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Lexer for tab

    :copyright: Copyright 2006-2015 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re

from pygments.lexer import RegexLexer, include, words
from pygments.token import Text, Comment, Operator, Keyword, Name, String, Number, Punctuation

__all__ = ['TabLexer']


class TabLexer(RegexLexer):
    """
    http://tag-lang.xyz
    """

    name = 'Tab'
    aliases = ['tab']
    filenames = ['*.tab']
    mimetypes = ['application/x-tab', 'text/x-tab', ]

    flags = re.DOTALL | re.MULTILINE
    tokens = {
        'root': [
            (r'\s+', Text),
            (r'==|!=|<=?|>=?|[-&|^+*/%!:?]|\*\*|~', Operator),
            (r'\[|\]|\{|\}|\(|\)|<<|>>|\[\.|\[\.|,|;|\.', Operator),
            (r'@', Name.Variable),
            (r'def|=', Name.Variable),
            (r'[a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?', Number.Float),
            (r'-?[0-9]+[uisl]?', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
        ]
    }
