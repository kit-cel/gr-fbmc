#include "fbmc_config.h"
#include <cmath>
#include <stdexcept>
#include <boost/format.hpp>

namespace gr{
	namespace fbmc{
		fbmc_config::fbmc_config(int num_used_subcarriers, 
								int num_payload_sym, 
								int num_overlap_sym, 
								std::string modulation, 
								std::string preamble):
							d_num_used_subcarriers(num_used_subcarriers),
							d_num_payload_sym(num_payload_sym),
							d_num_overlap_sym(num_overlap_sym),
							d_modulation(modulation),
							d_preamble(preamble)						
		{
			// user parameter validity check
			check_user_args();

			// calculate all the variables defined by the c'tor parameters
			d_num_total_subcarriers = int(std::pow(2, std::ceil(log2(d_num_used_subcarriers))));

			int num_ident_sym = 2; // number of identical symbols at the reciever that can be used for timing synchronization
			d_num_sync_sym= num_ident_sym + d_num_overlap_sym; // num_overlap_sym is needed to settle the filters
			d_num_preamble_sym = d_num_sync_sym + d_num_overlap_sym; // another num_overlap_sym is needed to clear the filter registers	

	    	// generate the prototype filter taps
	    	gen_prototype_filter();
	    	d_group_delay = (d_prototype_taps.size()-1)/2;

			d_num_sym_frame = d_num_preamble_sym + d_num_payload_sym + d_num_overlap_sym; // total symbols per frame	

			d_const = gr::digital::constellation_qpsk::make();

			// check calulated parameters for validity
			check_calc_params();	
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
				throw std::runtime_error(std::string("Need at least 1 used subcarrier"));
			else if(d_num_payload_sym < 1)
				throw std::runtime_error(std::string("Need at least 1 payload symbol"));
			else if(d_num_overlap_sym != 4)
				throw std::runtime_error(std::string("Overlap must be 4"));
			else if(d_preamble != "IAM")
				throw std::runtime_error(std::string("Only IAM is implemented as preamble"));
			else if(d_modulation != "QPSK")
				throw std::runtime_error(std::string("Only QPSK is implemented as modulation"));

			return true;
		}

		bool
		fbmc_config::check_calc_params()
		{
			if(d_num_total_subcarriers < d_num_used_subcarriers || d_num_total_subcarriers % 2 != 0)
				throw std::runtime_error(std::string("Invalid number of total subcarriers"));
			else if(d_num_sym_frame % 4 != 0)
				throw std::runtime_error(str(boost::format("Number of symbols per frame has to be a multiple of 4, but is %d") % d_num_sym_frame));		

			return true;
		}
	}
}