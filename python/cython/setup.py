from distutils.core import setup
from Cython.Build import cythonize

setup(
    ext_modules=cythonize("rx_domain_kernel_wrapper.pyx")
)