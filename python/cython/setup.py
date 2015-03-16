from distutils.core import setup
from Cython.Build import cythonize
from distutils.extension import Extension

extensions = [
    Extension("rx_domain_kernel_wrapper", ["rx_domain_kernel_wrapper.pyx"],
        include_dirs = ['/home/johannes/install/gnuradio/include/',
                        '/usr/include', ],
        libraries = ['boost_system',
                     'boost_filesystem',
                     'gnuradio-fbmc'],
        library_dirs = ['/home/johannes/install/gnuradio/lib/',
                        '/usr/lib/x86_64-linux-gnu', ],
        language='c++'
    ),
]

setup(
    ext_modules=cythonize(extensions)
)