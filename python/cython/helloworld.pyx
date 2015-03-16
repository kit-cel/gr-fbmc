# distutils: language = c++
from libcpp.vector cimport vector

print "Hello World right?"

cdef vector[float] vec

cdef int i
for i in range(5):
    vec.push_back(i)

for i in range(vec.size()):
    print "it:", vec[i]


def welcome():
    print "Hello Wo'ld my fl'iend"

def test():
    print "moin"