#include "fbmc_config.h"
#include <cmath>
#include <stdexcept>

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
			// calculate all the variables defined by the c'tor parameters
			d_num_total_subcarriers = int(std::pow(2, std::ceil(log2(d_num_used_subcarriers))));
			if(d_preamble == "IAM") // use modified IAM method for channel equalization and synchronization
			{
				int num_ident_sym = 2; // number of identical symbols at the reciever that can be used for timing synchronization
				d_num_sync_sym= num_ident_sym + d_num_overlap_sym; // num_overlap_sym is needed to settle the filters
				d_num_preamble_sym = d_num_sync_sym + d_num_overlap_sym; // another num_overlap_sym is needed to clear the filter registers
			}
			else
				throw std::runtime_error(std::string("Only IAM is implemented for the preamble"));

			// final validity check
			if(!check_args())
				throw std::runtime_error(std::string("Invalid argument(s)!"));
		}

		bool
		fbmc_config::check_args()
		{
			return false;
		}
	}
}