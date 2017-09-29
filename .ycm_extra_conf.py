# You Complete Me
import os
import ycm_core

flags = ['-x', 'c++', '-Wall', '-Wextra', '-Werror', '-std=c++17']

compilation_database_folder = 'build'

if os.path.exists( compilation_database_folder ):
    database = ycm_core.CompilationDatabase( compilation_database_folder )
else:
    database = None

SOURCE_EXTENSIONS = [ '.cpp',  '.c' ]

def DirectoryOfThisScript():
    return os.path.dirname( os.path.abspath( __file__ ) )


def IsHeaderFile( filename ):
    extension = os.path.splitext( filename )[ 1 ]
    return extension in [ '.h', '.hxx', '.hpp', '.hh' ]

def GetCompilationInfoForFile( filename ):
  # The compilation_commands.json file generated by CMake does not have entries
  # for header files. So we do our best by asking the db for flags for a
  # corresponding source file, if any. If one exists, the flags for that file
  # should be good enough.
  if IsHeaderFile( filename ):
    basename = os.path.splitext( filename )[ 0 ]
    for extension in SOURCE_EXTENSIONS:
      replacement_file = basename + extension
      if os.path.exists( replacement_file ):
        compilation_info = database.GetCompilationInfoForFile(
          replacement_file )
        if compilation_info.compiler_flags_:
          return compilation_info
    return None
  return database.GetCompilationInfoForFile( filename )


def FlagsForFile( filename, **kwargs ):
  if not database:
    return {
      'flags': flags,
      'include_paths_relative_to_dir': DirectoryOfThisScript()
    }

  compilation_info = GetCompilationInfoForFile( filename )
  if not compilation_info:
    return None

  # Bear in mind that compilation_info.compiler_flags_ does NOT return a
  # python list, but a "list-like" StringVec object.
  final_flags = list( compilation_info.compiler_flags_ )

  return {
    'flags': final_flags,
    'include_paths_relative_to_dir': compilation_info.compiler_working_dir_
  }

