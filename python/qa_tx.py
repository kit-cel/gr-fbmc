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

from gnuradio import gr, gr_unittest
from gnuradio import blocks, fft
import fbmc_swig as fbmc
import pylab as pl

class qa_tx (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
    	L = 8
    	num_items = L; # does not refer to symbols but to the number of items the head block let's through
    	overlap = 4 # this is hardcoded and not changeable at the moment
    	symbols = [-0.70710677-0.70710677j, -0.70710677+0.70710677j, -0.70710677-0.70710677j, \
    	           0.70710677+0.70710677j, -0.70710677+0.70710677j,  0.70710677-0.70710677j, \
    	           0.70710677+0.70710677j,  0.70710677-0.70710677j, -0.70710677-0.70710677j, \
    	           0.70710677-0.70710677j,  0.70710677-0.70710677j, -0.70710677-0.70710677j, \
    	           0.70710677+0.70710677j,  0.70710677-0.70710677j, -0.70710677+0.70710677j, \
    	          -0.70710677+0.70710677j, -0.70710677-0.70710677j,  0.70710677+0.70710677j, \
    	          -0.70710677-0.70710677j,  0.70710677-0.70710677j,  0.70710677+0.70710677j, \
    	           0.70710677+0.70710677j,  0.70710677-0.70710677j, -0.70710677+0.70710677j, \
    	           0.70710677+0.70710677j, -0.70710677+0.70710677j, -0.70710677-0.70710677j,\
    	           0.70710677-0.70710677j, -0.70710677+0.70710677j,  0.70710677-0.70710677j,\
    	           0.70710677+0.70710677j, -0.70710677-0.70710677j]
    	self.src = blocks.vector_source_c(symbols, vlen=1, repeat=False)
		#self.head = blocks.head(gr.sizeof_gr_complex,num_items +  L*(4 - ((L/num_items + overlap)%4)))
    	self.s2p = fbmc.serial_to_parallel_cvc(L,L)		
    	self.frame_gen = fbmc.frame_generator_vcvc(sym_len=L,frame_len=len(symbols)/L+4) 
    	self.serialize_iq = fbmc.serialize_iq_vcvc(L)
    	self.apply_betas = fbmc.apply_betas_vcvc(L)
    	self.fft = fft.fft_vcc(L, False, (()), False, 1)
		#self.mult_const = blocks.multiply_const_vcc(([L for i in range(L)]))
    	self.ppfb = fbmc.polyphase_filterbank_vcvc(L=L)
    	self.output_commutator = fbmc.output_commutator_vcc(L)
    	self.snk = blocks.vector_sink_c(vlen=1)
    	
    	self.tb.connect(self.src, self.s2p, self.frame_gen)
    	self.tb.connect(self.frame_gen, self.serialize_iq, self.apply_betas)
    	self.tb.connect(self.apply_betas, self.fft, self.ppfb)
    	self.tb.connect(self.ppfb, self.output_commutator, self.snk)
        self.tb.run ()
        
        # check data
        data = self.snk.data()
        ref = ((-0.00044857551619-0.00044857551619j) ,
        (0.000229785631692+9.51803250852e-05j) ,
        (0.000235008650809+0j) ,
        (-0.000786418595356+0.000325745247899j) ,
        (0.0029583569193-0.0029583569193j) ,
        (0.00458181268753-0.0110614743305j) ,
        0j ,
        (0.0101093232545+0.0244060653074j) ,
        (0.0134400202724+0.01254286924j) ,
        (0.00975847802034+0.00423245459431j) ,
        (-0.0215717198951+0j) ,
        (0.00448376796271-0.00120574700489j) ,
        (-0.137716406886+0.132248268564j) ,
        (-0.215114799807+0.499294519548j) ,
        -0.000117504325405j ,
        (-0.488114734649-1.13673473818j) ,
        (-0.676335831811-0.651965572669j) ,
        (-0.25964502846-0.121691428548j) ,
        (-0.999416226968+0.0107757996582j) ,
        (1.81418119521-0.802989724788j) ,
        (-0.151156427158+0.417901879642j) ,
        (0.515256420303-0.261694046575j) ,
        (-0.0107757996582+0.0107858599476j) ,
        (0.456298458913-0.898936890206j) ,
        (-0.121766605211-1.36154673438j) ,
        (0.583015566091+1.07714755376j) ,
        (0.0107657393689-0.47821526633j) ,
        (0.0756685134429+0.743323552321j) ,
        (0.668363949861+0.121318029694j) ,
        (0.651072260902+0.347989657838j) ,
        (0.499766865647+0.499766865647j) ,
        (-1.56545373081-0.117589633267j) ,
        (-0.27973689724+0.84684899308j) ,
        (0.0319435766181+0.682387824446j) ,
        (-0.499766865647-0.988767991924j) ,
        (-0.650533312684-1.5559741121j) ,
        (0.67929418873+0.548652512762j) ,
        (-0.452941798149-0.462707273885j) ,
        (0.0107858599476-0.0107757996582j) ,
        (0.480818026103+0.518103217719j) ,
        (0.0029583569193-0.0143371713047j) ,
        (-1.00977937991-0.55398060774j) ,
        (0.0107757996582-1.02098794686j) ,
        (-1.79142396516+0.770310989561j) ,
        (-0.692285633486-0.689775852083j) ,
        (0.222089662503-0.504623533939j) ,
        (-0.000117504325405+0j) ,
        (-0.0937032563404-0.212396961827j) ,
        (0.141123339322-0.140674763805j) ,
        (0.125196261535+0.0540823171728j) ,
        -0.0213367112443j ,
        (0.0288361498603-0.0125942562723j) ,
        (0.0138885957885+0.0138885957885j) ,
        (-0.00445727499753+0.0107608137503j) ,
        0j ,
        (0.00197669891847+0.0047721733377j) ,
        (-0.0029583569193+0.0029583569193j) ,
        (-0.00189858243861-0.000786418595356j) ,
        0.000235008650809j ,
        (-0.000554751588469+0.000229785631692j) ,
        (-0.00044857551619-0.00044857551619j) ,
        0j ,
        0j ,
        0j )
        max_diff = max(abs(pl.asarray(ref) - data))
        self.assertTrue(max_diff < 0.00001)      	           


if __name__ == '__main__':
    gr_unittest.run(qa_tx)
