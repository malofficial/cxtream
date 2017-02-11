# Tuple utilities
#
#  Copyright Filip Matzner 2017
#
#  Use, modification and distribution is subject to the
#  Boost Software License, Version 1.0.
#  (see http://www.boost.org/LICENSE_1_0.txt)
#

from setuptools import setup, Distribution


class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True


setup(name='stream',
      author="Filip Matzner",
      description="Python interface to the C++ stream library.",
      license="Boost Software License, Version 1.0",
      version='0.1',
      distclass=BinaryDistribution,)
      # package_data={'stream': ['build/lib/stream.so','build/lib/stream.dll']},
      # packages=['stream'],)
