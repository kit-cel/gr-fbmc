from libcpp.vector cimport vector


cdef extern from 'gnuradio/gr_complex.h':
    cppclass gr_complex:
        pass

# this exposes a C++ class
cdef extern from "fbmc/rx_sdft_kernel.h" namespace "gr::fbmc":
    cppclass rx_sdft_kernel:
        rx_sdft_kernel(vector[float] taps, int L) except +
        int L()
        int overlap()
        int fft_size()
        vector[float] taps()


# This is a wrapper.
cdef class py_rx_sdft_kernel:
    cdef rx_sdft_kernel *thisptr

    def __cinit__(self, vector[float] taps, int L):
        self.thisptr = new rx_sdft_kernel(taps, L)

    def __dealloc__(self):
        del self.thisptr

    def L(self):
        return self.thisptr.L()

    def overlap(self):
        return self.thisptr.overlap()

    def taps(self):
        self.thisptr.taps()

    def fft_size(self):
        self.thisptr.fft_size()


def get_proto_taps(int num):
    cdef vector[float] taps
    for i in range(num):
        # taps.push_back(1.0 + float(i) / 10.0)
        taps.push_back(1.0)
    taps[0] = 0.0
    taps[num - 1] = 0.0
    return taps


def my_main():
    print "Hello Wo'ld my fl'iend"

    proto_taps = get_proto_taps(5)
    print proto_taps

    kernel = py_rx_sdft_kernel(proto_taps, 2)
    print
    print "L = ", kernel.L()
    print "overlap = ", kernel.overlap()
    print "taps: ", kernel.taps()
    print "fft_size = ", kernel.fft_size()

    # cdef rx_sdft_kernel *kernel = new rx_sdft_kernel(proto_taps, 2)
    # L = kernel.L()
    # print "L=", L
    # overlap = kernel.overlap()
    # print "overlap=", overlap
    # taps = kernel.taps()
    # print "taps: ", taps
    # # fft_size = kernel.fft_size()
    # # print "fft_size=", fft_size



my_main()




