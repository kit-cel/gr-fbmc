#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2014 Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from operator import add

from numpy import empty_like, append, zeros, flipud, empty, roll
import numpy as np
from numpy.dual import ifft, fft
from scipy.signal import convolve
import matplotlib.pyplot as plt


def generate_vector(L):
    d = [complex(i, i + L) for i in range(0, L)]
    return d


def serialize_vector(d):
    L = int(len(d))
    di = [complex(d[i].real, 0) for i in range(L)]
    dq = [complex(d[i].imag, 0) for i in range(L)]
    di.extend(dq)
    return di


def map_to_channel(d, inlen, outlen, channel_map):
    assert (len(d) == inlen)
    assert (len(channel_map) == outlen)
    v = range(outlen)
    num = 0
    for i in range(outlen):
        if channel_map[i] == 1:
            v[i] = d[num]
            num += 1
        else:
            v[i] = complex(0, 0)
    return v


def unmap_from_channel(d, inlen, outlen, channel_map):
    assert (len(d) == inlen)
    assert (len(channel_map) == inlen)
    assert (len([i for i in channel_map if i == 1]) == outlen)

    num = 0
    v = range(outlen)
    for i in range(inlen):
        if channel_map[i] == 1:
            v[num] = d[i]
            num += 1
    return v


def commutate_output(d, L):
    assert (len(d) == L)
    assert (L % 2 == 0)
    Ld2 = int(L / 2)
    return map(add, d[0:Ld2], d[Ld2:])


def commutate_input(d, buf, L):
    # careful not exactly the inverse to commutate output!
    assert (len(buf) >= L / 2 - 1)  # make sure enough history is provided!
    assert (len(d) > L - 2)  # require one symbol!
    offset = L / 2 - 1
    buf = buf[0:offset] + d
    res = []
    for k in range(2):
        revbuf = buf[0:L / 2]
        revbuf = revbuf[::-1]
        revbuf *= 2
        res += revbuf
        buf = buf[L / 2:]
    return [res, buf]


def commutate_input_stream(d, L):
    assert (len(d) % L == 0)  # otherwise we may produce unexpected results.
    buf = [0, ] * 2 * L
    res = []
    while len(d) >= L - 1:
        [r, buf] = commutate_input(d[0:L], buf, L)
        res = res + r
        d = d[L:]
    return res


def rx(samples, prototype, osr):
    # 0. prep
    coeffs = _polyphase_filter_coeffs(prototype[::-1], osr)
    # print coeffs

    # 1. input commutator
    if coeffs.size / 2 == len(prototype) - 1:
        # PHYDYAS filter: first and last sample is zero and therefore cut
        # --> samples must be cut as well
        samples = flipud(samples[1:].reshape((-1, osr // 2)).T)

    else:
        # other filters use the full OSR*overlap + 1 samples
        # --> prefix some zeros to also get transient behavior right
        samples = flipud(append(
            zeros(osr - 1, dtype=samples[0].dtype), samples).reshape((-1, osr // 2)).T)
    # print samples
    # 2. polyphase filters
    samples = append(samples, empty_like(samples), axis=0)
    # out = empty((samples.shape[0], samples.shape[1] + coeffs.shape[1] - 1), dtype=samples.dtype)
    out = samples[:, :-(coeffs.shape[1] - 1)]
    # print np.round(samples)
    # reverse iteration to not override samples for l >= osr/2
    for l in reversed(range(osr)):
    # run filter (draw samples from the first half for l >= osr/2
        out[l, :] = convolve(samples[l % (osr // 2), :], coeffs[l, :], 'valid')
        # print "\n", np.round(samples)
        # print "out\n", np.round(out)

    # 3. spin polyphase signals
    out[:] = osr * ifft(out, osr, 0)

    return out


def _polyphase_filter_coeffs(prototype, osr):
    # fit length
    if prototype[-1] == 0.0: prototype = prototype[:-1]
    prototype = append(prototype, zeros(-len(prototype) % osr,
                                        dtype=prototype.dtype))
    # allocate and fill matrix
    coeffs = zeros((osr, 2 * len(prototype) / osr), dtype=prototype.dtype)
    for l in range(osr):
        # upsample branches by two, delay second half by one sample
        coeffs[l, (l >= osr / 2)::2] = prototype[l::osr]
    return coeffs


_OPT_FILTER_COEFFS = (
    None, None, None,  # K = 0/1/2
    (0.91143783, ),  # K = 3
    (0.97195983, ),  # K = 4
    None,  # K = 5 not available
    (0.99722723, 0.94136732),  # K = 6
    None,  # K = 7 not available
    (0.99988389, 0.99315513, 0.92708081))  # K = 8


def _get_freq_sample(k, K):
    """Get / Calculate filter coeff"""
    # symmetric
    k = abs(k)

    if k == 0:  # "DC"
        Gk = 1.0
    elif k < K / 2:  # get pre calculated values
        Gk = _OPT_FILTER_COEFFS[K][k - 1]
    elif k == K / 2:  # "middle" tap
        Gk = 1 / np.sqrt(2)
    elif k < K:  # satisfy x[k] = sqrt(1 - x[K-k]^2)
        Gk = np.sqrt(1 - _OPT_FILTER_COEFFS[K][K - k - 1] ** 2)
    else:  # all others zero = only neighbouring channels overlap
        Gk = 0.0

    return Gk


def prototype_fsamples(overlap, normalize=True):
    K = overlap
    freq_samples = np.array([
        (-1)**k * _get_freq_sample(k, K) for k in range(-K+1, K)
    ], dtype=np.float32)

    if normalize:
        freq_samples /= np.sqrt(K)
        assert 0.99 < (freq_samples * freq_samples.conj()).sum() < 1.01
    return freq_samples


def generate_phydyas_filter(L, K, min_grp_delay=False):
    g = np.zeros((L * K + 1, ))
    for m in range(L * K + 1):
        if m == 0:
            g[m] = 0.0  # Nyquist pulse
            # note: below formula works too, but not exact: | g[0] | < 1e-9

        elif m <= L * K / 2:
            g[m] = _get_freq_sample(0, K)

            for k in range(1, K): # note: H[K][k]=0 for all k>=K
                g[m] += 2 * (-1) ** k * _get_freq_sample(k, K) * \
                    np.cos(2 * np.pi * (k / float(K)) * (m / float(L)))
            g[m] /= 2.0

        else:
            # symmetric impulse response
            # note: this is optional, the above formula works for all m
            g[m] = g[len(g) - m - 1]

    if min_grp_delay:
        # trim leading and tailing zero
        g = g[1:-1]

    return g


def _spreading_matrix(prototype_freq, osr):
    overlap = (len(prototype_freq) + 1) // 2
    # user map
    num_carriers = osr
    subcarrier_map = list(range(num_carriers))

    # create a template row with zero shift
    G0 = zeros(osr * overlap, dtype=prototype_freq.dtype)
    G0[list(range(-(overlap-1), overlap))] = prototype_freq

    # construct a convolution matrix and shuffle rows
    G = zeros((len(subcarrier_map), osr * overlap), dtype=G0.dtype)
    for index, shift in enumerate(subcarrier_map):
        G[index, :] = roll(G0, shift * overlap)
    return G


def rx_fdomain(samples, prototype_freq, osr, oqam='SIOHAN', drop_in=None, drop_in2=None):
    """SMT receiver using a frequency domain filtering
    """
    K = (len(prototype_freq) + 1) // 2

    num_symbols = (len(samples) - (osr * K )) // (osr // 2) + 1

    R_hat = zeros((num_symbols, osr * K), dtype=samples.dtype)
    for i, offset in zip(range(num_symbols), range(0, len(samples), osr // 2)):
        R_hat[i,:] = fft(samples[offset:offset + K * osr]) #/ np.sqrt(osr * K)

    G_hat = _spreading_matrix(prototype_freq, osr)
    d_hat = np.dot(G_hat, R_hat.T)

    return d_hat


def _get_padding_zeros(total_subcarriers, overlap):
    return np.zeros(total_subcarriers * overlap, dtype=float)


def _get_payload_frame(payload, total_subcarriers, channel_map, payload_symbols):
    if payload_symbols % 2 != 0:
        raise ValueError('payload_symbols MUST be even!')
    invsqrt = 1.0 / np.sqrt(2.0)
    pf = np.zeros((payload_symbols, total_subcarriers), dtype=float)
    smt_scheme = _get_smt_scheme(total_subcarriers)
    ctr = 0
    for i, vec in enumerate(pf):
        for e, el in enumerate(vec):
            if channel_map[e] != 0 and smt_scheme[i % 2][e] != 0:
                val = payload[ctr]
                pf[i][e] = (2 * val - 1) * invsqrt
                ctr += 1
    return pf.flatten()


def _get_smt_scheme(total_subcarriers):
    scheme = np.zeros((2, total_subcarriers), dtype=int)
    isInphase = True
    for i in range(total_subcarriers):
        if isInphase:
            scheme[0][i] = 1
        if not isInphase:
            scheme[1][i] = 1
        if i % 2 == 0:
            isInphase = not isInphase
    return scheme


def get_frame(payload, total_subcarriers, channel_map, payload_symbols, overlap):
    preamble = get_preamble(total_subcarriers)
    zero_pad = _get_padding_zeros(total_subcarriers, overlap)
    payload = _get_payload_frame(payload, total_subcarriers, channel_map, payload_symbols)
    return np.concatenate((preamble, zero_pad, payload, zero_pad))


def get_payload(payload_symbols, used_subcarriers):
    num_pl_vals = payload_symbols * used_subcarriers // 2
    payload = np.random.randint(0, 2, num_pl_vals)
    return payload


def get_channel_map(used_subcarriers, total_subcarriers):
    # channel_map = [1, 1, 0, 0, 1, 1, 0, 0]  # original!
    channel_map = np.concatenate(
        (np.zeros(total_subcarriers - used_subcarriers, dtype=int), np.ones(used_subcarriers, dtype=int)))
    np.random.shuffle(channel_map)  # careful! it's an in-place shuffle!
    return channel_map


def get_preamble(total_subcarriers):
    preamble = np.ones(total_subcarriers, dtype=float) * 1
    preamble = np.concatenate((preamble, np.ones(total_subcarriers, dtype=float) * 2))
    preamble = np.concatenate((preamble, np.ones(total_subcarriers, dtype=float) * 3))
    preamble = np.concatenate((preamble, np.ones(total_subcarriers, dtype=float) * 4))
    return preamble


def main():
    np.set_printoptions(2, linewidth=150)
    total_subcarriers = 8
    used_subcarriers = 4
    channel_map = get_channel_map(used_subcarriers, total_subcarriers)
    payload_symbols = 8
    overlap = 4

    preamble = get_preamble(total_subcarriers)
    payload = get_payload(payload_symbols, used_subcarriers)
    frame = get_frame(payload, total_subcarriers, channel_map, payload_symbols, overlap)
    print np.reshape(frame, (-1, total_subcarriers)).T


if __name__ == '__main__':
    main()