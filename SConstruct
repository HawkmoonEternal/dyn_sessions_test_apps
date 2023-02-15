#! /usr/bin/python3

import os
import sys

#
# set possible variables
#
vars = Variables()

env = Environment(variables=vars, ENV = { 'PATH' : os.environ['PATH'],
                                          'HOME' : os.environ['HOME']
                                          })

# SWE specific variables
vars.AddVariables(
  PathVariable( 'buildDir', 'where to build the code', 'build', PathVariable.PathIsDirCreate ),

  EnumVariable( 'example', 'which application to build', 'mpidynres2d',
                allowed_values=(
                    'mpidynres2d',
                    'DynMPISessions_v2a', 'DynMPISessions_v2a_nb',
                    'DynMPISessions_v2b', 'DynMPISessions_v2b_nb',
                    'DynMPISessions_v2a_get', 'DynMPISessions_v2a_nb_get',
                    'DynMPISessions_v2b_get', 'DynMPISessions_v2b_nb_get'
                    )
              ),

  EnumVariable( 'compileMode', 'whether to compile with debug options', 'release',
                allowed_values=('debug', 'release')
              ),

  BoolVariable( 'netCDF', 'whether to use the netCDF library (required for reading scenario data from files)', 0),

  PathVariable( 'netCDFDir', 'location of netCDF', None)
)


# set environment
env = Environment(ENV = {'PATH': os.environ['PATH']},
        variables=vars)

# generate help text
Help("""Compile the example applications for dynamic resource management on p4est.
The following options are available:\n""" + vars.GenerateHelpText(env))

# handle unknown, maybe misspelled variables
unknownVariables = vars.UnknownVariables()

# exit in the case of unknown variables
if unknownVariables:
  print >> sys.stderr, "*** The following build variables are unknown:", unknownVariables.keys()
  Exit(1)

#
# precompiler, compiler and linker flags
#

env['CXX'] = env['LINKERFORPROGRAMS'] = env.Detect(['mpiCC', 'mpicxx'])
env.Append(HOME = ['/home/mpiuser'])
env['CXX'] = env['LINKERFORPROGRAMS'] = '/usr/bin/g++'
# set (pre-)compiler flags for the compile modes
if env['compileMode'] == 'debug':
  env.Append(CPPDEFINES=['DEBUG'])
  env.Append(CCFLAGS=['-O0','-g3','-Wall', '-fno-inline', '-std=gnu++11'])

elif env['compileMode'] == 'release':
  env.Append(CPPDEFINES=['NDEBUG'])
  env.Append(CCFLAGS=['-O3','-mtune=native', '-std=gnu++11'])
  #env.Append(CCFLAGS=['-O1','-mtune=native', '-std=gnu++11'])

# path of project root
project_root = os.getcwd()

# Add source directory to include path (important for subdirectories)
env.Append(CPPPATH=['.'])
env.Append(CPPPATH=['include'])
env.Append(CPPPATH=[project_root + '/../../../../install/ompi/include'])
env.Append(CPPPATH=[project_root + '/../../../build/ompi/opal/include'])

try:
	CPATH = list(filter(None, os.environ['C_PATH'].split(':')))
	print(CPATH)
	for path in CPATH:
    		env.Append(CPPPATH=[path])
except: 
	print('CPATH not set')


#env.Append(CPPPATH=os.environ.get('C_INLCUDE_PATH'))

env.Append(LIBPATH=['.'])
env.Append(LIBPATH=['include'])
env.Append(LIBPATH=['/opt/hpc/build/lib/lib'])

LD_LIBRARY_PATH = list(filter(None, os.environ.get('LD_LIBRARY_PATH').split(':')))
print(LD_LIBRARY_PATH)
for path in LD_LIBRARY_PATH:
    env.Append(LIBPATH=[path])

# link with p4est, libsc, and libmpidynres
env.Append(LINKFLAGS=['-lmpi']) #,'-lmpidynres'



#
# setup the program name and the build directory
#
#program_name = 'SWE_p4est'

program_name = env['example']

program_name += '_'+env['compileMode']

# build directory
build_dir = env['buildDir']+'/build_'+program_name

# get the src-code files
env.src_files = []

if env['example'] == 'DynMPISessions_v2a':
  sourceFiles = ['examples/dyn_mpi_sessions_v2a.cpp']
elif env['example'] == 'DynMPISessions_v2a_nb':
  sourceFiles = ['examples/dyn_mpi_sessions_v2a_nb.cpp']
elif env['example'] == 'DynMPISessions_v2b':
  sourceFiles = ['examples/dyn_mpi_sessions_v2b.cpp']
elif env['example'] == 'DynMPISessions_v2b_nb':
  sourceFiles = ['examples/dyn_mpi_sessions_v2b_nb.cpp']
elif env['example'] == 'DynMPISessions_v2a_get':
  sourceFiles = ['examples/dyn_mpi_sessions_v2a_get.cpp']
elif env['example'] == 'DynMPISessions_v2a_nb_get':
  sourceFiles = ['examples/dyn_mpi_sessions_v2a_nb_get.cpp']
elif env['example'] == 'DynMPISessions_v2b_get':
  sourceFiles = ['examples/dyn_mpi_sessions_v2b_get.cpp']
elif env['example'] == 'DynMPISessions_v2b_nb_get':
  sourceFiles = ['examples/dyn_mpi_sessions_v2b_nb_get.cpp']


for i in sourceFiles:
  env.src_files.append(env.Object(i))

# build the program
env.Program('build/'+program_name, env.src_files)
