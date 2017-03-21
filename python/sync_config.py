#
# Copyright 2004,2009,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#
#
# Chung, Wonsuk ; Kim, Chanhong ; Choi, Sooyong ; Hong, Daesik:
# Synchronization Sequence Design for FBMC/OQAM Systems Bd. 15, IEEE (2016), Nr. 10, S. 7199 - 721]

import numpy as np
import matplotlib.pyplot as plt

class sync_config:
    def __init__(self, N, overlap, L, pilot_A, pilot_timestep, pilot_carriers, subbands=1, bits=1, pos=4, u=1, q=4, A=1.0,
                 fft_len=2**13, guard=3, order=1):
        """
        Calculates preamble with independent Zadoff-Chu sequence
        :param taps: Filter taps of length O*N
        :param pos: Block position of preamble in first symbol
        :param N: Subcarrier number
        :param L: Periodic length of ZC sequence
        :param u: Parameter for ZC sequence
        :param q: Parameter for ZC sequence
        :param A: Amplitude of ZC sequence
        :param fft_len: Length of zero-padded FFT during frequency sync
        """
        assert pilot_timestep >= 4, "Min. pilot timstep is 4 when compensation with aux pilots is used"
        assert (order == 2 or order == 4 or order == 8), "Modulation order has to be 2, 4 or 8"
        self.N = N
        self.guard = guard
        self.k = pos
        self.bits = bits
        self.L = L
        self.pilot_A = pilot_A
        self.overlap = overlap
        self.pilot_timestep = pilot_timestep
        self.pilot_carriers = pilot_carriers
        self.u = u
        self.order = order
        self.q = q
        self.fft_len = fft_len
        self.A = A
        self.subbands = subbands
        self.h = np.reshape(self.get_taps_time()[1:]/np.sqrt(self.get_taps_time().dot(self.get_taps_time()))/10, (-1, N//2))
        self.c = self.build_preamble_symbols()
        self.Z_fft = np.fft.fft(self.get_zadoff_chu(self.N), fft_len)


    def get_phydyas_frequency_tap(self, k, overlap):
        optimal_filter_coeffs = [[],
                                 [],
                                 [],
                                 [0.91143783],
                                 [0.97195983],
                                 [],
                                 [0.99722723, 0.94136732],
                                 [],
                                 [0.99988389, 0.99315513, 0.92708081]]

        if len(optimal_filter_coeffs[overlap]) == 0:
            print("WARNING: Unsupported overlap size. Returning 0.0f")
            return 0.0

        k = abs(k)
        tap = 0.0
        if k == 0:
            tap = 1.0
        elif k < overlap//2:
            tap = optimal_filter_coeffs[overlap][k-1]
        elif k == overlap//2:
            tap = 1.0/np.sqrt(2.0)
        elif k < overlap:
            tap = np.sqrt(1.0 - optimal_filter_coeffs[overlap][overlap - k - 1] ** 2)
        else:
            tap = 0.0
        return tap

    def phydyas_frequency_taps(self):
        overlap = self.overlap
        taps = []
        for i in range(2*overlap-1):
            k = i-overlap+1
            tap = (-1.0)**k * self.get_phydyas_frequency_tap(k, overlap)
            taps.append(tap)
        return taps

    def phydyas_impulse_taps(self):
        # adapted from previous versions, sorry for bad variable names
        L = self.N
        overlap = self.overlap
        num_taps = L * overlap + 1
        taps = [0.0] * num_taps
        for m in range(num_taps):
            if m == 0:
                taps[m] = 0.0
            elif m <= L * overlap//2:
                taps[m] = self.get_phydyas_frequency_tap(0, overlap)
                for k in range(1,overlap):
                    tap = self.get_phydyas_frequency_tap(k, overlap)
                    taps[m] += 2.0 * (-1.0)**k * tap * np.cos(2*np.pi*float(k*m)/float(overlap*L))
                taps[m] /= 2.0
            else:
                taps[m] = taps[num_taps - m - 1]
        return taps

    def get_taps_time(self):
        phydyas_taps_time = np.array(self.phydyas_impulse_taps())
        return phydyas_taps_time#/np.sqrt(phydyas_taps_time.dot(phydyas_taps_time))

    def get_zadoff_chu(self, length):
        """ Returns Zadoff-Chu sequence of length """
        zc = self.A*np.array([np.exp(-1j*np.pi*self.u*n*(n+1+2*self.q)/self.L) for n in range(length)])
        return zc

    def get_compensated_zadoff_chu(self, length):
        """ Returns Zadoff-Chu sequence with interference compensation """
        Z = self.get_zadoff_chu(length) - length//2 * (sum(self.h[:][:self.k-1]))
        return Z

    def get_c_sequence(self):
        h = self.h
        k = self.k
        Z_tilde = self.get_compensated_zadoff_chu(self.N//2)
        C1 = np.empty(self.N//2, dtype=complex)
        C2 = np.empty(self.N//2, dtype=complex)

        for i in range(self.N//4 + 1):
            H = np.matrix([[    h[k][i],    h[k][-i]],
                           [   -h[k-1][i], h[k-1][-i]]])
            H = np.linalg.inv(H)
            Z = np.matrix([Z_tilde[i], np.conj(Z_tilde[-i])])
            C = np.array(Z*H)[0]
            C1[i] = C[0]
            C2[i] = 1j*C[1]
        C1[self.N//4+1:] = (np.conj(C1[1:self.N//4]))[::-1]
        C2[self.N//4+1:] = (np.conj(C2[1:self.N//4]))[::-1]
        return np.array([C1, C2])

    def build_preamble_symbols(self):
        """
        Return final preamble symbols of length N
        Map like this:
            a_{2n, 2m'} = c_n^R
            a_{2n, 2m'+1} = c_n^I
        """
        C = self.get_c_sequence()
        c = 2.0/self.N * np.fft.fft(C[0] + 1j*C[1])  # c_2n^R
        return c

    def get_fir_sequences(self):
        """Returns list of lists containing FIR filter taps for time correlation with time input signal"""
        zc = self.get_zadoff_chu(self.N//2)/self.A
        zc_freq = np.fft.fftshift(np.fft.fft(np.conj(zc)))
        zc_freq = np.concatenate((zc_freq, np.zeros((self.subbands-1)*self.N//2))) # time interpolation

        zc_freq_vec = []
        for i in range(0, self.subbands):
            zc_freq_vec.append(np.fft.ifft(np.fft.ifftshift(np.roll(zc_freq, self.N//2 * i))))
        return zc_freq_vec

    def get_preamble_symbols(self):
        return self.c

    def get_fft_len(self):
        return self.fft_len

    def get_cazac_ffts(self):
        zc = self.get_zadoff_chu(self.N//2)/self.A
        zc_freq = np.fft.fftshift(np.fft.fft(zc, self.fft_len))
        #zc_freq = np.concatenate((zc_freq, np.zeros((self.subbands-1)*self.fft_len))) # time interpolation

        zc_freq_vec = np.tile(zc_freq, self.subbands)
        #for i in range(0, self.subbands):
        #    zc_freq_vec.append(np.roll(zc_freq, self.fft_len * i))
        #plt.plot(abs(zc_freq_vec))
        #plt.show()
        return zc_freq_vec

    def get_pilot_amplitude(self):
        return self.pilot_A

    def get_pilot_timestep(self):
        return self.pilot_timestep

    def get_pilot_carriers(self):
        return self.pilot_carriers

    def get_moving_average_taps(self, length):
        """Taps for moving average FIR filter with given length"""
        return [1.0/length for n in range(length)]

    def get_syms_frame(self):
        bits_rem = int(self.bits/np.log2(self.order))
        syms = 2
        while bits_rem > 0:
            if (syms-2) % self.pilot_timestep == 0 or (syms-2) % self.pilot_timestep == 1:
                bits_rem -= (self.N - 2*self.guard) - len(self.pilot_carriers)
            else:
                bits_rem -= (self.N - 2*self.guard)
            syms += 1
        return syms

    def get_guard_bands(self):
        return self.guard

    def get_frame_samps(self, zeropad):
        syms = self.get_syms_frame()
        if zeropad:
            samps = (syms-1)*self.N//2 + self.N * self.overlap
        else:
            samps = (syms)*self.N//2
        return samps*self.subbands

    def get_bps(self):
        return int(np.log2(self.order));

    def get_subcarriers(self):
        return self.N

    def get_payload_bits(self):
        return self.bits

    def get_overlap(self):
        return self.overlap

    def get_guard_bands(self):
        return self.guard

#a = sync_config(taps=np.ones(32*4), N=32, L=31, pilot_A=1.0, pilot_timestep=4, pilot_carriers=range(0,32,5), pos=4, u=1, q=4, A=1.0, fft_len=2**13)
