from setuptools import setup, find_packages

setup (
  name='tablexer',
  version='0.1',
  description='',
  author='',
  packages=find_packages(),
  entry_points =
  """
[pygments.lexers]
tab = tablexer.lexer:TabLexer
  """,
)


