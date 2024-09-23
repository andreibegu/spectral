import os

import lit.formats

# Name of test suite
config.name = 'Pallas Spec2IR test suite'

config.test_format = lit.formats.ShTest(True)

# File-extensions to treat as tests
config.suffixes = ['.c', '.cpp', '.swift']

# Path to the test-files (i.e. this directory)
config.test_source_root = os.path.dirname(__file__)

# Path where the tests are run (We run them in the build-folder)
config.test_exec_root = os.path.join(config.spec2ir_bin_root, 'test')

# Substitution to refer to the Spec2IR tool
config.substitutions.append(('%Spec2IR', 
                             os.path.join(config.spec2ir_bin_root, 
                             os.path.join('bin', 'pallasSpec2ir'))))

# Substitution to get path of C specification library
config.substitutions.append(('%CSpecLib', 
                             os.path.join(config.spec2ir_src_root, 'res/spec_libs/c')))

# Substitution to get path of C++ specification library
config.substitutions.append(('%CppSpecLib', 
                             os.path.join(config.spec2ir_src_root, 'res/spec_libs/cpp')))

# Substitution to get path of Swift specification library
config.substitutions.append(('%SwiftSpecLib', 
                             os.path.join(config.spec2ir_src_root, 'res/spec_libs/swift/PallasSpec/.build/debug')))