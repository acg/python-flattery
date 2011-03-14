#!/usr/bin/env python

"""
Convert markdown to html.
Use pygments to syntax highlight any code blocks that begin with a shebang.
Wrap html and style it somewhat like docs.python.org.
"""

import sys
import re
from markdown import Markdown
from markdown import TextPreprocessor
from pygments import highlight
from pygments.formatters import HtmlFormatter
from pygments.lexers import get_lexer_by_name, TextLexer

INLINE_STYLES = False
INCLUDE_SHEBANG = False

class CodeBlockPreprocessor(TextPreprocessor):

  pattern = re.compile( r'^(?P<codeblock> {4}#!(?P<shebang>.*)\n(?P<body>(( {4}.*)?\n)*))', re.M)

  formatter = HtmlFormatter(noclasses=INLINE_STYLES)

  def run(self, lines):

    def repl(m):

      # extract program from shebang
      argv = m.group('shebang').split()
      program = argv[0].split('/')[-1]
      if program == 'env' and len(argv) >= 2:
        program = argv[1]

      try:
        lexer = get_lexer_by_name(program)
      except ValueError:
        lexer = TextLexer()

      if INCLUDE_SHEBANG:
        code = m.group('codeblock')
      else:
        code = m.group('body')

      code = highlight(code, lexer, self.formatter)
      code = code.replace('\n\n', '\n&nbsp;\n').replace('\n', '<br />')
      return '\n\n<div class="code">%s</div>\n\n' % code

    return self.pattern.sub(repl, lines)


if len(sys.argv) > 1:
  infile = file(sys.argv[1])
else:
  infile = sys.stdin

md = Markdown()
md.textPreprocessors.insert(0, CodeBlockPreprocessor())
html = md.convert(infile.read())

template = """
<html>
<head>
  <link rel="stylesheet" href="default.css" type="text/css" />
  <link rel="stylesheet" href="pygments.css" type="text/css" />
</head>
<body>
<div class="body">
%s
</div>
</body>
</html>
"""

print template % html

