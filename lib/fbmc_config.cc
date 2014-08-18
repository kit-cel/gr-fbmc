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

#include "fbmc_config.h"
#include <cmath>
#include <stdexcept>
#include <boost/format.hpp>
#include <numeric>
#include <iostream>
#include <fftw3.h>

namespace gr{
	namespace fbmc{
		fbmc_config::fbmc_config(int num_used_subcarriers, 
								int num_payload_sym, 
								int num_overlap_sym, 
								std::string modulation, 
								std::string preamble,
								int samp_rate):
							d_num_used_subcarriers(num_used_subcarriers),
							d_num_payload_sym(num_payload_sym),
							d_num_overlap_sym(num_overlap_sym),
							d_modulation(modulation),
							d_preamble(preamble),
							d_samp_rate(samp_rate)						
		{
			// user parameter validity check
			check_user_args();

			// calculate all the variables defined by the c'tor parameters
			d_num_total_subcarriers = int(std::pow(2, std::ceil(log2(d_num_used_subcarriers))));

			if(d_num_used_subcarriers >= d_num_total_subcarriers)
			{
				std::cerr << "WARNING: Invalid number of used subcarriers" << std::endl;
				d_num_used_subcarriers = d_num_total_subcarriers-1;
			}

			int num_used_adjusted = d_num_used_subcarriers - d_num_used_subcarriers%4; // make sure it's modulo 4 so that the betas fit nicely
			if(num_used_adjusted != d_num_used_subcarriers)
			{
				std::cerr << "WARNING: Number of used subcarriers has been changed to " << num_used_adjusted << std::endl;
				d_num_used_subcarriers = num_used_adjusted;
			}

			d_num_sync_sym = d_num_overlap_sym + 2; // num_overlap_sym is needed to settle the filters
			d_num_preamble_sym = d_num_sync_sym + d_num_overlap_sym; // another num_overlap_sym is needed to clear the filter registers	

	    	// generate the prototype filter taps
	    	gen_prototype_filter();
	    	d_group_delay = (d_prototype_taps.size()-1)/2;

			d_num_sym_frame = d_num_preamble_sym + d_num_payload_sym + d_num_overlap_sym; // total symbols per frame	

			d_const = gr::digital::constellation_qpsk::make();

			// generate (DC free) channel map centered around zero, if the number of used channels is odd, add the one to the USB
			d_channel_map.assign(d_num_total_subcarriers, 0); // preset the vector to all zeros
			int num_used_usb = d_num_used_subcarriers/2 + (d_num_used_subcarriers%2); // add one if the M is odd
			int num_used_lsb = d_num_used_subcarriers/2;
			for(int i = 0; i < num_used_usb; i++)
				d_channel_map[1+i] = 1;
			for(int i = 0; i < num_used_lsb; i++)
				d_channel_map[d_num_total_subcarriers-1-i] = 1;

			// generate frequency domain preamble for insertion into the tx frame
			gen_preamble_sym();

			// generate time domain preamble for correlation in the receiver
			gen_ref_preamble_sym();

			// check calulated parameters for validity
			check_calc_params();	

			// print a short summary of the parameters to stdout
			print_info();
		}

	    void
	    fbmc_config::gen_prototype_filter()
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

	    	int sps = d_num_total_subcarriers/2;
	    	int delay = d_num_overlap_sym;

			// Distance between zeros in time (tau0) and frequency(v0)
			double tau0 = 1.0/std::sqrt(2.0);
			double v0   = 1.0/std::sqrt(2.0);
			if(std::abs(tau0*v0-0.5) > 1e-6)
				throw std::runtime_error("assertion std::abs(tau0*v0-0.5) <= 1e-6 failed");

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
	    fbmc_config::gauss(std::valarray<float> x, float alpha)
	    {
	    	return std::sqrt(std::sqrt(2.0*alpha)) * std::exp(-M_PI*alpha*std::pow(x,std::valarray<float>(2,x.size())));
	    }

	    float
	    fbmc_config::d(int k, float alpha, float v0, int K)
	    {
	        // d Coefficients for closed form IOTA calculation
	        if(std::abs(v0 - 1.0/std::sqrt(2.0)) > 1e-6)
	        	throw std::runtime_error("assertion std::abs(v0 - 1.0/std::sqrt(2.0)) < 1e-6 failed");

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

		bool
		fbmc_config::check_user_args()
		{
			if(d_num_used_subcarriers < 1) 
				throw std::runtime_error(std::string("Number of used subcarriers must be > 1"));
			else if(d_num_payload_sym < 1)
				throw std::runtime_error(std::string("Need at least 1 payload symbol"));
			else if(d_num_overlap_sym != 4)
				throw std::runtime_error(std::string("Overlap must be 4"));
			else if(d_preamble != "IAM")
				throw std::runtime_error(std::string("Only IAM is implemented as preamble"));
			else if(d_modulation != "QPSK")
				throw std::runtime_error(std::string("Only QPSK is implemented as modulation"));
			else if(d_samp_rate <= 0)
				throw std::runtime_error(std::string("Invalid sample rate"));

			return true;
		}

		bool
		fbmc_config::check_calc_params()
		{
			if(d_num_total_subcarriers < d_num_used_subcarriers || d_num_total_subcarriers % 4 != 0)
				throw std::runtime_error(std::string("Invalid number of total subcarriers, has to be positive and an integer multiple of 4"));
			else if(d_num_sym_frame % 4 != 0)
				throw std::runtime_error(str(boost::format("Number of symbols per frame has to be a multiple of 4, but is %d") % d_num_sym_frame));
			else if(std::accumulate(d_channel_map.begin(), d_channel_map.end(), 0) != d_num_used_subcarriers || d_channel_map.size() != d_num_total_subcarriers)
			{
				std::cout << std::accumulate(d_channel_map.begin(), d_channel_map.end(), 0) << " " << d_channel_map.size() << std::endl;
				throw std::runtime_error(std::string("Channel map does not match the number of total/used subcarriers!"));
			}
			return true;
		}
		void
		fbmc_config::print_info()
		{
			std::cout << "**********************************************************" << std::endl;
			std::cout << "******************* FBMC parameters *********************" << std::endl;
			std::cout << "**********************************************************" << std::endl;
			std::cout << "FFT length:\t\t\t\t\t\t" << d_num_total_subcarriers << std::endl;
			float subc_spac = float(d_samp_rate)/d_num_total_subcarriers/1000;
			std::cout << "Subcarrier spacing (kHz):\t\t" << subc_spac << std::endl;
			std::cout << "Number of used subcarriers:\t" << d_num_used_subcarriers << std::endl;
			std::cout << "Occupied bandwidth (kHz):\t\t" << float(d_samp_rate)/d_num_total_subcarriers*d_num_used_subcarriers/1000 << std::endl;
			std::cout << "Number of symbols per frame:\t" << d_num_sym_frame << std::endl;
			std::cout << "\t-> payload symbols:\t\t\t" << d_num_payload_sym << std::endl;
			std::cout << "\t-> preamble symbols:\t\t" << d_num_preamble_sym << std::endl;
			std::cout << "\t-> overlap guard symbols:\t" << d_num_overlap_sym << std::endl;
			float tsym = float(d_num_total_subcarriers)/d_samp_rate;
			std::cout << "Symbol duration (ms):\t\t\t" << tsym*1000 << std::endl;
			std::cout << "Frame duration (ms):\t\t\t\t" << tsym*1000*d_num_sym_frame << std::endl;
			std::cout << "Modulation:\t\t\t\t\t\t" << modulation() << std::endl;
			std::cout << "Filterbank group delay:\t\t" << (d_prototype_taps.size()-1)/2 << std::endl;
			int payl_bits_frame = d_num_payload_sym*log2(constellation_points().size())*d_num_used_subcarriers;
			std::cout << "Bits per frame:\t\t\t\t\t" << payl_bits_frame << std::endl;
			float payl_bit_rate = float(payl_bits_frame) / (tsym*d_num_sym_frame);
			std::cout << "Payload bit rate (kbps):\t\t\t" << payl_bit_rate/1000 << std::endl;
			std::cout << "Spectral efficiency (b/Hz):\t\t" << payl_bit_rate/(d_num_used_subcarriers*subc_spac*1000) << std::endl;
			std::cout << "**********************************************************" << std::endl << std::endl;
		}

		void 
		fbmc_config::gen_preamble_sym()
		{
			if(d_num_used_subcarriers > pow(2,16)-1)
				throw std::runtime_error("Max length 2**16-1 of PN sequence exceeded");
		    uint16_t start_state = 0xACE1u;  /* Any nonzero start start will work. */
		    uint16_t lfsr = start_state;
		    unsigned bit;
		    unsigned period = 0;
		    float output;
		 
		    for(int i = 0; i < 2*d_num_total_subcarriers; i++)
		    {
		        /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
		        bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
		        lfsr =  (lfsr >> 1) | (bit << 15);
		        output = ((lfsr % 2) - 0.5)*2; // map bits to -1/1
		        d_prbs.push_back(output);
		    }

		    if(d_channel_map.size() != d_num_total_subcarriers)
		    	throw std::runtime_error("Channel map too small");

		    d_preamble_sym = std::vector<gr_complex>(4*d_num_total_subcarriers, gr_complex(0,0));
		    int n=0;
		    for(int i = 0; i < d_num_total_subcarriers; i++)
		    {		    	
		    	if(d_channel_map[i]==0)
		    		d_preamble_sym[i] = gr_complex(0,0);
		    	else
		    	{
		    		d_preamble_sym[i] = gr_complex(1.0/sqrt(2)*d_prbs[n],0);
		    		n++;
		    	}		 
		    	d_preamble_sym[i+2*d_num_total_subcarriers] = d_preamble_sym[i];		
		    }
		}

		void
		fbmc_config::gen_ref_preamble_sym()
		{

			// to convert the frequency domain preamble into time domain, an IFFT and a filter operation is needed
			int nsym = d_preamble_sym.size()/d_num_total_subcarriers;
		    fftwf_complex* buffer = (fftwf_complex*) fftwf_malloc(sizeof(fftw_complex)*d_num_total_subcarriers);
		    std::vector<gr_complex> preamble_sym_fft(d_preamble_sym.size(), gr_complex(0,0));
		
			// Check datatype
			if(sizeof(gr_complex)!=sizeof(fftw_complex)) std::runtime_error("sizeof(gr_complex)!=sizeof(fftw_complex)");
			
			// Setup and execute FFT
			fftwf_plan fft_plan = fftwf_plan_dft_1d(d_num_total_subcarriers, buffer, buffer, FFTW_BACKWARD, FFTW_ESTIMATE);
			for(int i = 0; i < nsym; i++)
			{
				memcpy(buffer, &d_preamble_sym[0]+i*d_num_total_subcarriers, d_num_total_subcarriers*sizeof(gr_complex));
				fftwf_execute(fft_plan);
				memcpy(&preamble_sym_fft[0]+i*d_num_total_subcarriers, buffer, d_num_total_subcarriers*sizeof(gr_complex));				
			}

			fftwf_destroy_plan(fft_plan);
			fftwf_free(buffer);

			// filter with prototype filters of the filter bank
			std::vector<gr_complex> taps(d_prototype_taps.size(), gr_complex(0,0));
			for(int i = -d_num_total_subcarriers/2; i < d_num_total_subcarriers/2-1; i++)
			{
				for(int n = 0; n < d_prototype_taps.size(); n++)
				{
					taps[n] += d_prototype_taps[n]*exp(gr_complex(0,2*M_PI*i*n/d_num_total_subcarriers))	;
				}
			}

			FILE* dbg_fp2 = fopen("taps_vals.bin", "wb");
			fwrite(&taps[0], sizeof(gr_complex), taps.size(), dbg_fp2);
			fclose(dbg_fp2);

			int diff = preamble_sym_fft.size() - taps.size();
			if(diff > 0) // preamble is longer than the filter impulse response, pad the preamble
			{
				for(int i = 0; i < diff; i++)
					preamble_sym_fft.push_back(0);
			}
			else if(diff < 0) // more taps than the preamble length, pad impulse response
			{
				for(int i = 0; i < -diff; i++)
					taps.push_back(0);
			}

			std::vector<gr_complex> filtered_preamble_sym(2*taps.size()-1, gr_complex(0,0));

			// 1st half
			for(int i = 0; i<=taps.size()-1; i++)
			{
				for(int j = 0; j <= i; j++)
					filtered_preamble_sym[i] += taps[j]*preamble_sym_fft[i-j];
			} 
			// 2nd half
			for(int i = 2; i <= taps.size(); i++)
			{
				for(int j = 0; j <= taps.size() - i; j++)
					filtered_preamble_sym[taps.size() + i - 2] += taps[j+1+i-2]*preamble_sym_fft[taps.size()-j-1];
			}


			// add the first and second half of one symbol, thus shortening the sequence by a factor of 2 (--> output commutator)
			d_ref_preamble_sym.clear();
			d_ref_preamble_sym.resize(filtered_preamble_sym.size()/2, gr_complex(0,0));
			for(int i = 0; i < filtered_preamble_sym.size()/d_num_total_subcarriers; i++)
			{
				for(int n = 0; n < d_num_total_subcarriers/2; n++)
					d_ref_preamble_sym[n+i*d_num_total_subcarriers/2] = filtered_preamble_sym[n+i*d_num_total_subcarriers] + filtered_preamble_sym[n+i*d_num_total_subcarriers + d_num_total_subcarriers/2];
			}

			FILE* dbg_fp = fopen("ref_vals.bin", "wb");
			fwrite(&d_ref_preamble_sym[0], sizeof(gr_complex), d_ref_preamble_sym.size(), dbg_fp);
			fclose(dbg_fp);
		}
	}
}