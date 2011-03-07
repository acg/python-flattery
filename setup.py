#!/usr/bin/env python

from distutils.core import setup, Extension, Command

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
)

