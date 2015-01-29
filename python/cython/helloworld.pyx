# distutils: language = c++
from libcpp.vector cimport vector

print "Hello Wo'ld my fl'iend"

cdef vector[float] vec

cdef int i
for i in range(5):
    vec.push_back(i)

for i in range(vec.size()):
    print vec[i]