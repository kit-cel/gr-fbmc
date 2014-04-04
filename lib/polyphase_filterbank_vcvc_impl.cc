/* -*- c++ -*- */
/* 
 * Copyright 2014 Communications Engineering Lab (CEL), Karlsruhe Institute of Technology (KIT).
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "polyphase_filterbank_vcvc_impl.h"

namespace gr {
  namespace fbmc {

    polyphase_filterbank_vcvc::sptr
    polyphase_filterbank_vcvc::make(int L)
    {
      return gnuradio::get_initial_sptr
        (new polyphase_filterbank_vcvc_impl(L));
    }

    /*
     * The private constructor
     */
    polyphase_filterbank_vcvc_impl::polyphase_filterbank_vcvc_impl(int L)
      : gr::sync_block("polyphase_filterbank_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*L)),
              d_prototype_taps(std::vector<gr_complex>(0)),
              d_L(L),
              d_num_branch_taps(0),
              d_branch_taps(NULL),
              d_branch_states(NULL),
              d_group_delay(0),
              d_overlap(4) // hard-coded for now)
    {
    	// set up the table needed for the generation of the filter taps
        // [0, Table V]
        float tab[15][8] = {
            {1.0, 3.0/4, 105.0/64, 675.0/256, 76233.0/16384, 457107.0/65536, 12097169.0/1048576, 70545315.0/4194304},
            {-1.0, -15.0/8, -219.0/64, -6055.0/1024, -161925.0/16384, -2067909.0/131072, -26060847.0/1048576, 0},
            {3.0/4, 16.0/19, 1545.0/512, 9765.0/2048, 596227.0/65536, 3679941.0/262144, 3941559701.0/1677216, 0},
            {-5.0/8, -123.0/128, -2289.0/1024, -34871.0/8192, -969375.0/131072, -51182445.0/4194304, 0, 0},
            {35.0/64, 213.0/256, 7797.0/4096, 56163.0/16384, 13861065.0/2097152, 87185895.0/8388608, 0, 0},
            {-63.0/128, -763.0/1024, -13875.0/8192, -790815.0/262144, -23600537.0/4194304, 0, 0, 0},
            {231.0/512, 1395.0/2048, 202281.0/131072, 1434705.0/524288, 85037895.0/16777216, 0, 0, 0},
            {-429.0/1024, -20691.0/32768, -374325.0/262144, -5297445.0/2097152, 0, 0, 0, 0},
            {6435.0/16384, 38753.0/65536, 1400487.0/1048576, 9895893.0/4194304, 0, 0, 0, 0},
            {-12155.0/32768, -146289.0/262144, -2641197.0/2097152, 0, 0, 0, 0, 0},
            {46189.0/131072, 277797.0/524288, 20050485.0/16777216, 0, 0, 0, 0, 0},
            {-88179.0/262144, -2120495.0/4194304, 0, 0, 0, 0, 0, 0},
            {676039.0/2097152, 4063017.0/8388608, 0, 0, 0, 0, 0, 0},
            {-1300075.0/4194304, 0, 0, 0, 0, 0, 0, 0},
            {5014575.0/16777216, 0, 0, 0, 0, 0, 0, 0}
        };

        // initialize table as std::valarray for convenient handling
        for(int i = 0; i < 15; i++)
        {
        	d_b[i] = std::valarray<float>(8);
        	for(int n = 0; n < 8; n++)
        		d_b[i][n] = tab[i][n];
        }

    	// generate the prototype filter taps
    	gen_prototype_filter();
    	//std::cout << "number of prototype taps: " << d_prototype_taps.size() << std::endl;

    	d_group_delay = (d_prototype_taps.size() - 1)/2;

    	// pad the prototype to an integer multiple length of L
    	while(d_prototype_taps.size() % d_L != 0)
    		d_prototype_taps.push_back(gr_complex(0,0));

    	// prepare the filter branches
    	d_num_branch_taps = d_prototype_taps.size()/d_L*2; // calculate number of taps per branch filter
    	d_branch_taps = new gr_complex*[d_L];
    	d_branch_states = new boost::circular_buffer<gr_complex>[d_L];
    	for(int l = 0; l < d_L; l++)
    	{
    		// write the upsampled prototype coefficients into the branch filter
    		d_branch_taps[l] = new gr_complex[d_num_branch_taps];
    		memset(d_branch_taps[l], 0, d_num_branch_taps*sizeof(gr_complex));
    		int offset = 0;
    		if(l >= L/2)
    			offset = 1;
    		for(int n = 0; n < d_num_branch_taps; n++)
    			if( (n+offset) % 2) // tap is zero due to oversampling
    				d_branch_taps[l][n] = 0;
    			else
    				d_branch_taps[l][n] = d_prototype_taps[l+(n/2)*d_L];

    		// set size of state registers and initialize them with zeros
    		d_branch_states[l].set_capacity(d_num_branch_taps); // this includes the new sample in each iteration
    		for(int i = 0; i < d_num_branch_taps; i++)
    			d_branch_states[l].push_front(gr_complex(0,0));
    	}
    }

    /*
     * Our virtual destructor.
     */
    polyphase_filterbank_vcvc_impl::~polyphase_filterbank_vcvc_impl()
    {
    	delete[] d_branch_taps;
    	delete[] d_branch_states;
    }
    void
    polyphase_filterbank_vcvc_impl::gen_prototype_filter()
    {

/*
 * 		Calculates IOTA pulse function by orthogonalizing a gaussian pulse in the
    	time and frequency domain as shown in [0] and [1].
 *
 *      [0] Cosine-modulated filterbanks based on extended Gaussian functions
        by Siohan, Pierre and Roche, Christian
        in IEEE Transactions on Signal Processing (Nov 2000)

    	[1] Derivation of Extended Gaussian Functions Based on the Zak Transform
        by Siohan, Pierre and Roche, Christian
        in IEEE Signal Processing Letters (Mar 2004)
 */
    	int sps = d_L/2;
    	int delay = d_overlap;

		// Distance between zeros in time (tau0) and frequency(v0)
		double tau0 = 1.0/std::sqrt(2.0);
		double v0   = 1.0/std::sqrt(2.0);
		if(std::abs(tau0*v0-0.5) > 1e-6)
		{
			std::cout << std::abs(tau0*v0-0.5) << " > " << 1e-6 << std::endl;
			throw std::runtime_error("assertion std::abs(tau0*v0-0.5) <= 1e-6 failed");
		}

		// Gaussian function parameter
		float alpha = 1.0;
		// Check alpha [1, above Eq. (15)]
		float alpha_m_v0   = 0.528 * std::pow(v0,2);
		float alpha_m_tau0 = 1.892/std::pow(tau0,2);
		if(alpha_m_v0 > alpha)
			throw std::runtime_error("assertion alpha_m_v0 <= alpha failed");
		if(alpha > alpha_m_tau0)
			throw std::runtime_error("assertion alpha <= alpha_m_tau0 failed");

		// Build impulse response

		// Select sample times [0, Eq. (13)]
		std::valarray<float> t(2*sps*delay+1);
		for(int i = -sps*delay; i < sps*delay+1; i++)
			t[i+sps*delay] = tau0 / sps * i;

		// Number of iterations for sums in Eq. (7) [0, below Eq. (27)]
		int K=14;

		// Calculate IOTA impulse response [0, Eq. (7)] or [1, Eq. (15)] with c = 2
		std::valarray<float> s1(0.0, t.size());
		std::valarray<float> s2(0.0, t.size());

		for(int k = 0; k < K+1; k++)
		{
			// Elements of first sum
			s1 += d(k,    alpha,  v0, K) * ( gauss(t+float(k)/float(v0),alpha) + gauss(t-float(k)/float(v0),alpha) );
			// Elements of second sum
			s2 += d(k,1.0/alpha,tau0, K) * std::cos(float(2.0*M_PI*k)*t/float(tau0));
		}


		// construct impulse response
		std::valarray<float> imp_res = float(0.5) * s1 * s2;
		float norm_fac = std::sqrt((imp_res * imp_res).sum());
		imp_res /= norm_fac;
		d_prototype_taps = std::vector<gr_complex>(imp_res.size());
		for(int i = 0; i < imp_res.size(); i++)
			d_prototype_taps[i] = imp_res[i];
    }

    std::valarray<float>
    polyphase_filterbank_vcvc_impl::gauss(std::valarray<float> x, float alpha)
    {
    	return std::sqrt(std::sqrt(2.0*alpha)) * std::exp(-M_PI*alpha*std::pow(x,std::valarray<float>(2,x.size())));
    }

    float
    polyphase_filterbank_vcvc_impl::d(int k, float alpha, float v0, int K)
    {
        // d Coefficients for closed form IOTA calculation
        if(std::abs(v0 - 1.0/std::sqrt(2.0)) > 1e-6)
        {
        	std::cerr << "std::abs(v0 - 1.0/std::sqrt(2.0)) = " << std::abs(v0 - 1.0/std::sqrt(2.0)) << std::endl;
        	throw std::runtime_error("assertion std::abs(v0 - 1.0/std::sqrt(2.0)) < 1e-6 failed");
        }

        if(K != 14)
        	throw std::runtime_error("assertion K == 14 failed");

		// Number of iterations [0, below Eq. (27)]
		int jk = std::floor(float(K-k)/2);

		std::valarray<float> tmp(jk+1);
		for(int i = 0; i < jk+1; i++)
			tmp[i] = 2*i + k;
		std::valarray<float> fac1 = d_b[k][std::slice(0,jk+1,1)];
		std::valarray<float> fac2 = std::exp(float(-M_PI*alpha/(2.0*std::pow(v0,float(2.0)))) * tmp);
    	std::valarray<float> res = fac1 * fac2;

    	return res.sum();
    }

    void
    polyphase_filterbank_vcvc_impl::filter(gr_complex* in, gr_complex* out)
    {
    	for(int l = 0; l < d_L; l++)
    		filter_branch(&in[l], &out[l], l);
    }

    void
    polyphase_filterbank_vcvc_impl::filter_branch(gr_complex* in, gr_complex* out, int l)
    {
    	// the actual convolution
    	d_branch_states[l].push_front(*in);
    	*out = 0;
    	for(int n = 0; n < d_num_branch_taps; n++)
    		*out += d_branch_states[l][n] * d_branch_taps[l][n];
    }

    int
    polyphase_filterbank_vcvc_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        gr_complex *in  = (gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        //FIXME: make sure center coeff has phase 0
        if(d_group_delay % d_L != 0) // just a reminder to avoid unexpected behavior
        	throw std::runtime_error("assertion d_group_delay % d_L == 0 failed");

        // Filter one vector of L samples and return L samples
        filter(in, out);

        // Tell runtime system how many output items we produced.
        return 1;
    }

  } /* namespace fbmc */
} /* namespace gr */

