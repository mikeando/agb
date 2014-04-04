env = Environment()
env.Append( CPPPATH=['include'] )
env.Append( CPPPATH=['../../libgit2/include/'] )
src = ( Glob('src/*/*.c') )
lib = env.Library('anbgitbridge', src)

test_utils_env=env.Clone()
test_utils_env.Append( CPPPATH=['tests'] )
test_utils_src = ( Glob('tests/external/*.c'), Glob('tests/utils/*.c') )
test_utils_lib=test_utils_env.Library('anbgitbridge_testutils', test_utils_src)

test_env=env.Clone()
test_env.Append( CPPPATH=['tests'] )
test_env.Append( LIBS=['git2'] )
test_env.Append( LIBPATH=['../../libgit2/build'] )

test_src = ( lib, test_utils_lib, Glob('tests/*.c'), Glob('tests/core/*.c') )
test_env.Program('run_tests', test_src)
