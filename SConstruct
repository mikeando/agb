env = Environment()
env.Append( CPPPATH=['include'] )
env.Append( CPPPATH=['../../libgit2/include/'] )
#env.Append( CFLAGS=['-Weverything','-g'] )
#env.Append( CFLAGS=['-Weverything','-Wno-documentation','-g'] )
env.Append( CFLAGS=['-Wall','-g'] )
env.Append( LINKFLAGS=['-g'] )
src = ( Glob('src/*/*.c') )
lib = env.Library('anbgitbridge', src)


test_utils_env=env.Clone()
test_utils_env.Append( CPPPATH=['tests'] )
test_utils_src = ( Glob('tests/external/*.c'), Glob('tests/utils/*.c') )
test_utils_lib=test_utils_env.Library('anbgitbridge_testutils', test_utils_src)
clar_suite = test_utils_env.Command('tests/clar.suite', Glob('src/core/*.c'), "python ./tests/clar/generate.py tests") 
AlwaysBuild(clar_suite)


test_env=env.Clone()
test_env.Append( CPPPATH=['tests'] )
test_env.Append( LIBS=['git2'] )
test_env.Append( LIBPATH=['../../libgit2/build'] )
clar_obj = test_env.Object('tests/clar/clar.c')
Depends(clar_obj,File('tests/clar.suite'))

test_src = [lib, test_utils_lib, clar_obj] + Glob('tests/*.c') + Glob('tests/core/*.c')
test_env.Program('run_tests', test_src)

util_env = env.Clone()
util_env.Append( LIBS=['git2'] )
util_env.Append( LIBPATH=['../../libgit2/build'] )

get_commit_info = util_env.Program('utils/bin/get_commit_info', 'utils/src/get_commit_info.c')
