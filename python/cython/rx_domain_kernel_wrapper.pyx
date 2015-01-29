# distutils: language = c++
from libcpp.vector cimport vector

print "Hello Wo'ld my fl'iend"

cdef vector[float] vec

cdef int i
for i in range(5):
    vec.push_back(i)

for i in range(vec.size()):
    print vec[i]


cdef extern from "fbmc/rx_domain_kernel.h" namespace "gr/fbmc":
    cppclass rx_domain_kernel:
        rx_domain_kernel(vector[float] taps, int L)
        int overlap()

# cdef class mykernel:
#     cdef rx_domain_kernel* obj
#
#     def __init__(self, vector[float] taps, int L):
#         self.cobj = new rx_domain_kernel(taps, L)


