#!/usr/bin/env python

"""flattery - fast flattening and unflattening of nested Python data

This library exposes a fast C implementation for flattening and unflattening hierarchical Python data structures. A unit test suite is included. See README.md for usage examples.
"""

classifiers = """\
Development Status :: 4 - Beta
Intended Audience :: Developers
License :: OSI Approved :: BSD License
Programming Language :: Python
Topic :: Text Processing :: General
Topic :: Software Development :: Libraries :: Python Modules
Operating System :: OS Independent
"""

from distutils.core import setup, Extension

doclines = __doc__.split("\n")

setup(
  name='flattery',
  version='0.1',
  ext_modules=[
    Extension(
      'flattery.cext',
      [ 'flattery.c' ],
      extra_compile_args=['-fPIC'],
      define_macros=[],
    ),
  ],
  packages=['flattery'],
  maintainer="Alan Grow",
  maintainer_email="alangrow+flattery@gmail.com",
  url="https://github.com/acg/python-flattery",
  license = "https://github.com/acg/python-flattery/blob/master/LICENSE",
  platforms = ["any"],
  description = doclines[0],
  classifiers = filter(None, classifiers.split("\n")),
  long_description = "\n".join(doclines[2:]),
)

