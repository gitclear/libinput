prg_install = find_program('install')

doxygen = find_program('doxygen', required : false)
if not doxygen.found()
	error('Program "doxygen" not found or not executable. Try building with -Ddocumentation=false')
endif
dot = find_program('dot', required : false)
if not dot.found()
	error('Program "dot" not found or not executable. Try building with -Ddocumentation=false')
endif

doxygen_version_cmd = run_command(doxygen.path(), '--version')
if doxygen_version_cmd.returncode() != 0
	error('Command "doxygen --version" failed.')
endif
doxygen_version = doxygen_version_cmd.stdout()
if doxygen_version.version_compare('< 1.8.3')
	error('doxygen needs to be at least version 1.8.3 (have @0@)'.format(doxygen_version))
endif
grep = find_program('grep')
dot_version_cmd = run_command(dot.path(), '-V')
if dot_version_cmd.returncode() != 0
	error('Command "dot -V" failed.')
endif
# dot -V output is (to stderr):
# 	dot - graphviz version 2.38.0 (20140413.2041)
dot_version = dot_version_cmd.stderr().split(' ')[4]
if dot_version.version_compare('< 2.26')
	error('Graphviz dot needs to be at least version 2.26 (have @0@)'.format(dot_version))
endif

mainpage = vcs_tag(command : ['git', 'log', '-1', '--format=%h'],
		 fallback : 'unknown',
		 input : 'mainpage.dox',
		 output : 'mainpage.dox',
		 replace_string: '__GIT_VERSION__')

src_doxygen = files(
	# source files
	'../../src/libinput.h',
	# style files
	'style/header.html',
	'style/footer.html',
	'style/customdoxygen.css',
	'style/bootstrap.css',
	'style/libinputdoxygen.css',
)

config_noop = configuration_data()
# Set a dummy replacement to silence meson warnings:
# meson.build:487: WARNING: Got an empty configuration_data() object and
# 		   found no substitutions in the input file 'foo'. If you
# 		   want to copy a file to the build dir, use the 'copy:'
# 		   keyword argument added in 0.47.0
config_noop.set('dummy', 'dummy')

doxyfiles = []
foreach f : src_doxygen
	df = configure_file(input: f,
			    output: '@PLAINNAME@',
			    configuration : config_noop)
	doxyfiles += [ df ]
endforeach

doc_config = configuration_data()
doc_config.set('PACKAGE_NAME', meson.project_name())
doc_config.set('PACKAGE_VERSION', meson.project_version())
doc_config.set('builddir', meson.current_build_dir())

doxyfile = configure_file(input : 'libinput.doxygen.in',
			  output : 'libinput.doxygen',
			  configuration : doc_config)

custom_target('doxygen',
	      input : [ doxyfiles, doxyfile, mainpage ] + src_doxygen,
	      output : [ '.' ],
	      command : [ doxygen, doxyfile ],
	      install : false,
	      depends: [ mainpage ],
	      build_by_default : true)
