#include <vector>
#include <valarray>
#include "gnuradio/types.h"
#include <gnuradio/digital/constellation.h>


namespace gr{
	namespace fbmc{
		class fbmc_config
		{
			public:
				fbmc_config(int num_used_subcarriers = 100, 
							int num_payload_sym = 102, 
							int num_overlap_sym = 4, 
							std::string modulation = "QPSK", 
							std::string preamble = "IAM");

				// public get methods
				gr_complex blub;
				int num_used_subcarriers(){return d_num_used_subcarriers;}
				int num_total_subcarriers(){return d_num_total_subcarriers;}
				int num_payload_sym(){return d_num_payload_sym;}
				int num_overlap(){return d_num_overlap_sym;}
				int num_sync_sym(){return d_num_sync_sym;}
				int num_preamble_sym(){return d_num_preamble_sym;}
				int num_sym_frame(){return d_num_sym_frame;}
				std::vector<gr_complex> const_points(){return d_const->points();}
				std::vector<gr_complex> prototype_taps(){return d_prototype_taps;}

			private:
				bool check_user_args(); // checks constructor parameters for validity
				bool check_calc_params(); // check the calculated params for validity

				int d_num_used_subcarriers; // between 0.5 ... 1 * d_num_total_subcarriers
				int d_num_total_subcarriers; // power of 2
				int d_num_payload_sym; // number of payload symbols (one symbol consists of L subcarriers)
				int d_num_overlap_sym; // number of overlapping symbols due to the pulse shaping
				std::string d_modulation; // name of modulation type
				gr::digital::constellation_sptr d_const; // constellation object
				std::string d_preamble; // name of preamble and therefore equalization and synchronization method
				int d_num_sync_sym; // number of synchronization symbols (excluding overlap)
				int d_num_preamble_sym; // number of preamble symbols (includes overlap)
				int d_num_sym_frame; // number of symbols per frame including preamble and overlap
				std::vector<gr_complex> d_prototype_taps; // prototype taps for pulse shaping
				std::valarray<float> d_b[15]; // keeps a table of coefficients needed for the IOTA pulse generation
				int d_group_delay; // group delay introduced by the filter bank

				void gen_prototype_filter(); // calculates the taps for the prototype filter (IOTA)
				std::valarray<float> gauss(std::valarray<float> x, float alpha); // calculates a gauss pulse
				float d(int k, float alpha, float v0, int K); // coefficients for IOTA calculation

				// frame structure:
				// ... || d_num_sync_sym | d_num_overlap | d_num_payload_sym | d_num_overlap || ...
				//      |<-     d_num_preamble_sym     ->|
				//      |<-                        d_num_frame_sym                         ->|
		};		
	}
}
