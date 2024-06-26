import os

import lit.formats

# Name of test suite
config.name = 'Pallas Spec2IR test suite'

config.test_format = lit.formats.ShTest(True)

# File-extensions to treat as tests
config.suffixes = ['.c']

# Path to the test-files (i.e. this directory)
config.test_source_root = os.path.dirname(__file__)

# Path where the tests are run (We run them in the build-folder)
config.test_exec_root = os.path.join(config.spec2ir_bin_root, 'test')

# Substitution to refer to the Spec2IR tool
config.substitutions.append(('%Spec2IR', 
                             os.path.join(config.spec2ir_bin_root, 'pallasSpec2ir')))