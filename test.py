#!/usr/bin/env python

import unittest
from flattery import flatten, unflatten


class FunctionTestCase(unittest.TestCase):

  def __init__(self, **keywords):
    unittest.TestCase.__init__(self)
    self.args = []
    self.kwargs = {}

    for k, v in keywords.items():
      setattr(self, k, v)

  def runTest(self):
    self.assertEqual( self.func(*self.args,**self.kwargs), self.expected )

  def shortDescription(self):
    return self.name


# TODO add more coverage
# TODO add bi-directional tests of flatten()

TEST_UNFLATTEN = 0x1
TEST_FLATTEN = 0x2
TEST_BOTH = TEST_UNFLATTEN | TEST_FLATTEN

TRUTH = [
  ("empty", TEST_BOTH, {}, {}),
#  ("empty key", TEST_BOTH, {'':1}, {'':1}),
  ("depth 2 path x 1", TEST_BOTH, {"a.b":42}, {"a":{"b":42}}),
  ("depth 2 path x 2", TEST_BOTH, {"a.b":42,"c.d":"x"}, {"a":{"b":42},"c":{"d":"x"}}),
  ("depth 2 path x 2, overlap", TEST_BOTH, {"a.b":42,"a.c":"x"}, {"a":{"b":42,"c":"x"}}),
  ("simple list", TEST_BOTH, {"a.0":1,"a.1":2}, {"a":[1,2]}),
  ("sparse list", TEST_BOTH, {"a.0":1,"a.9":10}, {"a":[1,None,None,None,None,None,None,None,None,10]}),
  ("depth 6", TEST_BOTH, {"a.b.c.d.e":1}, {"a":{"b":{"c":{"d":{"e":1}}}}}),
  ("list under dict", TEST_BOTH, {"a.b.0":1,"a.b.1":2}, {"a":{"b":[1,2]}}),
  ("dict under list x 1", TEST_BOTH, {"a.0.b":1}, {"a":[{"b":1}]}),
  ("dict under list x 2, overlap", TEST_BOTH, {"a.0.b":1,"a.0.c":2}, {"a":[{"b":1,"c":2}]}),
]


def main():

  suite = unittest.TestSuite()
  i = 0

  for name, mode, flat, unflat in TRUTH:

    if mode & TEST_FLATTEN:

      suite.addTest(FunctionTestCase(
        name="test %d %s" % (i,name),
        func=flatten,
        args=[unflat],
        expected=flat,
      ))
      i += 1

    if mode & TEST_UNFLATTEN:

      suite.addTest(FunctionTestCase(
        name="test %d %s" % (i,name),
        func=unflatten,
        args=[flat],
        expected=unflat,
      ))
      i += 1

  runner = unittest.TextTestRunner(verbosity=2)
  results = runner.run(suite)

  if len(results.failures) or len(results.errors):
    return 1
  else:
    return 0


if __name__ == '__main__':
  import sys
  sys.exit(main())

