APPNAME = 'ffilt'
VERSION = '0.0.3'

def options(opt):
    opt.load('compiler_cxx')
    opt.add_option('--target-windows', action='store_true', default=False, help='set up to do a cross-compile to make a Windows package')

def configure(conf):
    conf.load('compiler_cxx')

    conf.env.TARGET_WINDOWS = conf.options.target_windows

    boost_lib_suffix = ''
    if conf.env.TARGET_WINDOWS:
        conf.env.append_value('CXXFLAGS', ['-mconsole'])
        conf.env.append_value('LINKFLAGS', ['-mconsole'])
        boost_lib_suffix = '-mt'

    conf.env.append_value('CXXFLAGS', ['-D__STDC_CONSTANT_MACROS', '-msse', '-mfpmath=sse', '-ffast-math', '-fno-strict-aliasing', '-Wall', '-Wno-attributes'])
    conf.env.append_value('CXXFLAGS', ['-fpermissive'])

    conf.check_cfg(package='libdcp-1.0', atleast_version='1.4.3', args='--cflags --libs', uselib_store='DCP', mandatory=True)
    conf.env.DEFINES_DCP = [f.replace('\\', '') for f in conf.env.DEFINES_DCP]
    conf.check_cxx(fragment="""
    			      #include <boost/filesystem.hpp>\n
    			      int main() { boost::filesystem::copy_file ("a", "b"); }\n
			      """, msg='Checking for boost filesystem library',
                              libpath='/usr/local/lib',
                              lib=['boost_filesystem%s' % boost_lib_suffix, 'boost_system%s' % boost_lib_suffix],
                              uselib_store='BOOST_FILESYSTEM')

def build(bld):
    obj = bld(features='cxx cxxprogram')
    obj.uselib = 'BOOST_FILESYSTEM DCP'
    obj.source = 'ffilt.cc'
    obj.target = 'ffilt'
