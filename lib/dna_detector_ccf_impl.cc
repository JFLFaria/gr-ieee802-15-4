/* -*- c++ -*- */
/*
 * Copyright 2022 Frankie Cruz.
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
#include "dna_detector_ccf_impl.h"

namespace gr {
namespace ieee802_15_4 {

dna_detector_ccf::sptr dna_detector_ccf::make(int devices, int dna_row_size,
		int dna_column_size, int dna_region_num, DebugType debug,
		float samp_rate, std::string fingerprint_filename,
		SWITCH fingerprint_save_switch, std::string class_matrix_filename,
		std::string means_matrix_filename, std::string norm_vector_filename,
		std::string xoffset_vector_filename, SWITCH class_matrix_load_switch,
		SWITCH save_stats_switch, SWITCH save_dna_results_switch) {
	return gnuradio::get_initial_sptr(
			new dna_detector_ccf_impl(devices, dna_row_size, dna_column_size,
					dna_region_num, debug, samp_rate, fingerprint_filename,
					fingerprint_save_switch, class_matrix_filename,
					means_matrix_filename, norm_vector_filename,
					xoffset_vector_filename, class_matrix_load_switch,
					save_stats_switch, save_dna_results_switch));
}

/*
 * The private constructor
 */
dna_detector_ccf_impl::dna_detector_ccf_impl(int devices, int dna_row_size,
		int dna_column_size, int dna_region_num, DebugType debug,
		float samp_rate, std::string fingerprint_filename,
		SWITCH fingerprint_save_switch, std::string class_matrix_filename,
		std::string means_matrix_filename, std::string norm_vector_filename,
		std::string xoffset_vector_filename, SWITCH class_matrix_load_switch,
		SWITCH save_stats_switch, SWITCH save_dna_results_switch) :
		gr::block("dna_detector_ccf",
				gr::io_signature::make(1, 1, sizeof(gr_complex)),
				gr::io_signature::make(0, 0, 0)) {
	d_devices = devices;
	d_dna_row_size = dna_row_size;
	d_dna_column_size = dna_column_size;
	d_dna_region_num = dna_region_num;
	set_debug(debug);
	d_samp_rate = samp_rate;
	d_phy_802_15_4.set_sample_rate(samp_rate);
	d_fingerprint_filename = fingerprint_filename;
	d_fingerprint_save_switch = fingerprint_save_switch;
	d_class_matrix_filename = class_matrix_filename;
	d_class_matrix_load_switch = class_matrix_load_switch;
	d_save_stats_switch = save_stats_switch;
	d_save_dna_results_switch = save_dna_results_switch;
	d_means_matrix_filename = means_matrix_filename;
	d_norm_vector_filename = norm_vector_filename;
	d_xoffset_vector_filename = xoffset_vector_filename;

	//vector will be used to keep track of what device
	//"bin's" were detected
	for (int c = 0; c < devices; c++) {

		d_emitter_bin_counter.push_back(0);
	}
	std::cout << "\nSample Rate on DNA Detector set to" << samp_rate
			<< std::endl;

	//Iftheuserdoesn'tloadanydnadata,
	//the
	//asanIdentitymatrix
	//datawillautomaticallybeset
	if (d_class_matrix_load_switch == SWITCH::ON) {
		//emptyvectorswillbeusedtostoretheloadeddata
		//theeigenmapfunctionwillhandlemappingthedata
		//asavector/matrixasappropriate
		std::vector<float> data_vector(dna_row_size * dna_column_size, 0.0f);
		std::vector<float> means_vector(devices * dna_column_size, 0.0f);
		std::vector<float> norm_vector(dna_row_size, 0.0f);
		std::vector<float> offset_vector(dna_row_size, 0.0f);

		d_dna_loader = std::make_shared < file_read_impl
				> (sizeof(float), class_matrix_filename.c_str(), false);
		d_dna_loader->read_data(dna_row_size * dna_column_size,
				(void*) data_vector.data());
		std::cout << "\n" << d_class_matrix_filename
				<< " classification matrix loaded";
		std::cout << data_vector.size() << "-items.\n" << std::endl;
		d_dna_matrix = EigMat<float>::Map(data_vector.data(), dna_row_size,
				dna_column_size);

		std::cout << "\n" << dna_row_size << " by " << dna_column_size
				<< " loaded.\n" << std::endl;

		d_class_means_loader = std::make_shared < file_read_impl
				> (sizeof(float), means_matrix_filename.c_str(), false);
		d_class_means_loader->read_data(devices * dna_column_size,
				(void*) means_vector.data());
		std::cout << "\n" << d_means_matrix_filename << " means matrix loaded ";
		std::cout << data_vector.size() << "-items.\n" << std::endl;
		d_class_means_matrix = EigMat<float>::Map(means_vector.data(), devices,
				dna_column_size);

		std::cout << "\n" << devices << " by " << dna_column_size
				<< " loaded.\n" << std::endl;

		d_norm_vector_loader = std::make_shared < file_read_impl
				> (sizeof(float), norm_vector_filename.c_str(), false);
		d_norm_vector_loader->read_data(dna_row_size,
				(void*) norm_vector.data());
		std::cout << "\n" << d_norm_vector_filename
				<< " normalization vector loaded";
		std::cout << norm_vector.size() << "-items.\n" << std::endl;
		d_norm_vector = EigRowVec<float>::Map(norm_vector.data(), 1,
				dna_row_size);
		std::cout << "\n 1 by " << dna_row_size << " loaded.\n" << std::endl;

		d_xoffset_vector_loader = std::make_shared < file_read_impl
				> (sizeof(float), xoffset_vector_filename.c_str(), false);
		d_xoffset_vector_loader->read_data(dna_row_size,
				(void*) offset_vector.data());
		std::cout << "\n" << d_xoffset_vector_filename
				<< " xoffset vector loaded ";
		std::cout << offset_vector.size() << "-items.\n" << std::endl;
		d_xoffset_vector = EigRowVec<float>::Map(offset_vector.data(), 1,
				dna_row_size);
		std::cout << "\n 1 by " << dna_row_size << " loaded.\n" << std::endl;

	} else {
		std::cout << "\nClassification data matrix wasn't loaded. A "
				<< dna_row_size << " by " << dna_column_size
				<< " identity matrix loaded instead." << "\n";
		std::cout << "A pure " << dna_row_size
				<< " by 1 finger print passthrough will occur\n" << std::endl;
		d_dna_matrix = Eigen::MatrixXf::Identity(dna_row_size, dna_row_size);

		std::cout << "\nMeans data matrix wasn't loaded. A " << devices
				<< " by " << dna_column_size
				<< " identity matrix loaded instead." << "\n";
		std::cout << "A " << devices << " by " << dna_column_size
				<< " identity matrix will be loaded instead.\n" << std::endl;
		d_class_means_matrix = Eigen::MatrixXf::Identity(devices,
				dna_column_size);

		std::cout << "\nNormalization data vector wasn't loaded. A 1 by "
				<< dna_row_size << " ones vector loaded instead." << "\n";
		std::cout << "A 1 by " << dna_row_size
				<< " ones vector will be loaded instead.\n" << std::endl;
		d_norm_vector = Eigen::MatrixXf::Ones(1, dna_row_size);

		std::cout << "\nXOffset data vector wasn't loaded. A 1 by "
				<< dna_row_size << " ones vector loaded instead." << "\n";
		std::cout << "A 1 by " << dna_row_size
				<< " ones vector will be loaded instead.\n" << std::endl;
		d_xoffset_vector = Eigen::MatrixXf::Ones(1, dna_row_size);

	}

	//Thecollectedfingerprintdatawillcontinuouslybe
	//stackedontheendofthefileuntiltheprogramisover
	if (d_fingerprint_save_switch == SWITCH::ON) {
		std::cout << "\nGenerated fingerprints will be saved as binary data to "
				<< d_fingerprint_filename << "\n" << std::endl;
		d_dna_fingerprint_write = std::make_shared < file_store_impl
				> (sizeof(float), d_fingerprint_filename.c_str(),
				//true,true);
				true, false);
	}

	//Theoutputdnaanalysiswillbesavedjustlikethe
	//fingerprintsbutwith"_dna_results"appendedto
	//thedefaultfilename
	if (d_save_dna_results_switch == SWITCH::ON) {
		std::string temp = fingerprint_filename + "_dna_results";
		std::cout << "\nGenerated dna_results will be saved as binary data to "
				<< temp << "\n" << std::endl;
		d_dna_results_writer = std::make_shared < file_store_impl
				> (sizeof(float), temp.c_str(), true, false); //true,true);

	}

	//Optiontocollectthestatisticsgenerated
	//forthegeneratedfingerprintsandresults
	if (d_save_stats_switch == SWITCH::ON) {
		//fromstackoverflow.com,justaquickwaytoappendtime/datetoafilename
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);

		//stat'sresultfileload/creation
		d_msg_stream << d_fingerprint_filename << "_stats_"
				<< std::put_time(&tm, "%d%b%y_%R") << ".txt";
		d_fingerprint_stats_filename.append(d_msg_stream.str());
		clear_stream();
		std::cout << "\nGenerated stats will be saved to "
				<< d_fingerprint_stats_filename << ".\n" << std::endl;
		d_stats_writer =
				std::make_shared < file_store_impl
						> (sizeof(char), d_fingerprint_stats_filename.c_str(), false, false);
		d_msg_stream
				<< "\n======================================================================================="
				<< "\n";
		d_msg_stream << "Collection started on " << std::put_time(&tm, "%F_%T");
		d_msg_stream << "\nSample Rate at " << samp_rate;
		d_msg_stream << "\n" << d_dna_region_num
				<< " regions of interest for fingerprints="
				<< dna_region_num * 9 << " samples per fingerprint";
		d_msg_stream << "\nPreamble sample length at " << samp_rate
				<< " Samp/sec is " << d_phy_802_15_4.get_preamble_len_samples();
		d_msg_stream << "\nPacket sample length at " << samp_rate
				<< " Samp/sec is " << d_phy_802_15_4.get_packet_len_samples();
		d_msg_stream
				<< "\n======================================================================================="
				<< "\n";

		d_stats_writer->store_data(d_msg_stream.str().size(),
				(void*) d_msg_stream.str().c_str());
		clear_stream();

		//matrixresultfileload/creation
		d_msg_stream << d_fingerprint_filename << "_matrix_mult_"
				<< std::put_time(&tm, "%d%b%y_%R") << ".txt";
		d_matrix_mult_results_filename.append(d_msg_stream.str());
		clear_stream();
		d_matrix_results_writer =
				std::make_shared < file_store_impl
						> (sizeof(char), d_matrix_mult_results_filename.c_str(), false, false);
		std::cout << "\nGenerated multiplication results will be saved to "
				<< d_matrix_mult_results_filename << ".\n" << std::endl;
		d_msg_stream
				<< "\n======================================================================================="
				<< "\n";
		d_msg_stream << "Collection started on " << std::put_time(&tm, "%F_%T");
		d_msg_stream << "\nSample Rate at " << samp_rate << "\n";
		d_msg_stream << "\nPreamble sample length at " << samp_rate
				<< " Samp/sec is " << d_phy_802_15_4.get_preamble_len_samples();
		d_msg_stream << "\nPacket sample length at " << samp_rate
				<< " Samp/sec is " << d_phy_802_15_4.get_packet_len_samples();
		d_msg_stream << "\n" << d_dna_region_num
				<< " regions of interest for fingerprints= "
				<< dna_region_num * 9 << " samples per fingerprint";
		d_msg_stream << "\nLoaded Classification Matrix Dimensions: "
				<< d_dna_matrix.rows() << " by " << d_dna_matrix.cols() << "\n";
		d_msg_stream << d_dna_matrix << "\n";
		d_msg_stream
				<< "\n======================================================================================="
				<< "\n";
		d_msg_stream << "Loaded Means Matrix Dimensions: "
				<< d_class_means_matrix.rows() << " by "
				<< d_class_means_matrix.cols() << "\n";
		d_msg_stream << d_class_means_matrix << "\n";
		d_msg_stream
				<< "\n======================================================================================="
				<< "\n";
		d_msg_stream << "Loaded Normalization Vector Dimensions: 1 by "
				<< d_norm_vector.cols() << "\n";
		d_msg_stream << d_norm_vector << "\n";
		d_msg_stream
				<< "\n======================================================================================="
				<< "\n";
		d_msg_stream << "Loaded X-Offset Vector Dimensions: 1 by "
				<< d_xoffset_vector.cols() << "\n";
		d_msg_stream << d_xoffset_vector << "\n";
		d_msg_stream
				<< "\n======================================================================================="
				<< "\n";

		d_matrix_results_writer->store_data(d_msg_stream.str().size(),
				(void*) d_msg_stream.str().c_str());

	} //OptiontosaveDNAresults

	burst_search();

	message_port_register_out(pmt::mp("dna_out"));
}

/*
 * Our virtual destructor.
 */
dna_detector_ccf_impl::~dna_detector_ccf_impl() {
}

void dna_detector_ccf_impl::forecast(int noutput_items,
		gr_vector_int &ninput_items_required) {
	ninput_items_required[0] = 2044;
}

int dna_detector_ccf_impl::general_work(int noutput_items,
		gr_vector_int &ninput_items, gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items) {
	const gr_complex *input_bursts = (const gr_complex*) input_items[0]; //detected ieee802-15-4 bursts
	int ninput = ninput_items[0]; //size of input burst data stream
	//<+OTYPE+> *out = (<+OTYPE+> *) output_items[0];  // Not used

	float magnitude = 0.0f;

	int burst_index = 0;
	int sample_limit = d_phy_802_15_4.get_preamble_len_samples() + 10; // "oh crap" 10 sample buffer added

	while (burst_index < ninput) {
		switch (d_state) {

		case SEARCHING:
			//since the bursts are zero buffered and dealing with floating numbers,
			//an extremely small floating number will be used
			if (std::abs(input_bursts[burst_index]) > 1.0e-38f) {
				//The previous detector ensures that every burst has exactly a complex
				//zero as their value so all complex zero's are ignored before the
				//preamble bursts

				burst_detect();
			} else {
				burst_index++;
			}
			break;

		case DETECTED:
			//if(d_current_detected_preamble.get_sample_length()<sample_limit)
			if (d_current_detected_preamble.size() < sample_limit) {
				d_current_detected_preamble.push_back(
						input_bursts[burst_index++]);

			} else {
				//filled burst will be sent to be fingerprinted

				burst_fingerprint();
			}
			break;

		case FINGERPRINT:
			//put case statement in brackets because of
			//compilation errors
		{
			//At this point,I need to perform RF DNA Analysis and get the fingerprint
			//from the latest pre-amble
			d_dna_fingerprints = calculate_fingerprints(
					d_current_detected_preamble);

			//Here, the fingerprint will be compared against the trained
			//data and return the euclidean distances calculated from
			//the loaded Matlab matricies
			d_dna_results = fingerprint_emitter(d_dna_fingerprints);

			//We need to check if a NaN value is detected since it would invalidate our fingerprint
			bool nan_found = false;
			for (std::vector<gr_complex>::size_type i = 0;
					i != d_dna_fingerprints.size(); i++) {
				if (std::isnan(d_dna_fingerprints[i])) {
					nan_found = true;
					d_nan_counter++;

					break;
				}

			}

			//Increment counters for stats
			if (!nan_found) {
				d_fingerprints_counter++;

			}

			//Here, the generated fingerprint will be passed on if
			//the user has enabled saving fingerprints and they do
			//not contain NaN's
			if ((d_fingerprint_save_switch == SWITCH::ON) && !nan_found) {
				d_dna_fingerprint_write->store_data(d_dna_fingerprints.size(),
						(void*) d_dna_fingerprints.data());

			}

			//Here, the generated fingerprint will be passed on if
			//the user has enabled saving fingerprints and they do
			//not contain NaN's
			if ((d_save_dna_results_switch == SWITCH::ON) && !nan_found) {
				d_dna_results_writer->store_data(d_dna_results.size(),
						(void*) d_dna_results.data());

			}
			//setting the initial min to the numeric limit
			//of a float ensures that
			float min = std::numeric_limits<float>::max();
			int most_likely = 0;

			if (!nan_found) {
				for (int c = 0; c < d_devices; c++) {
					if (d_dna_results[c] < min) {
						min = d_dna_results[c];

						most_likely = c + 1;

					}

				}

			}

			//Increment Emitter array
			d_emitter_bin_counter[most_likely - 1] += 1;

			//now I use c to pass the vector values to the histogram
			pmt::pmt_t dna_results = pmt::make_f32vector(d_devices, 0.0);
			for (int i = 0; i < d_devices; i++) {

				pmt::f32vector_set(dna_results, i, most_likely);
			}

			//A message indicating which device will only be sent out
			//if the fingerprint generated was valid
			if (!nan_found) {

				message_port_pub(pmt::mp("dna_out"), dna_results);
			}

			//This if block will save the data overview of the fingerprints
			//in a simple to read output
			if (d_save_stats_switch == SWITCH::ON) {
				clear_stream();
				d_msg_stream
						<< "\n---------------------------------------------------------------------------------------"
						<< "\n";
				d_msg_stream << "Burst: " << d_burst_count + 1 << "\n";
				d_msg_stream << "Device Most Likely: "
						<< (nan_found ?
								"N/A, NaN detected" :
								std::to_string(most_likely)) << "\n";
				d_msg_stream << "Device Result Value: "
						<< (nan_found ?
								"NaN" :
								std::to_string(d_dna_results[most_likely - 1]))
						<< "\n";
				d_msg_stream << "Total Valid Fingerprints: "
						<< d_fingerprints_counter << "\n";
				d_msg_stream << "Total Invalid Fingerprints: " << d_nan_counter
						<< "\n";

				for (std::vector<unsigned long int>::size_type i = 0;
						i != d_emitter_bin_counter.size(); i++) {
					d_msg_stream << "Emitter_" << i + 1 << "Count: "
							<< d_emitter_bin_counter[i] << "\t";

				}
				d_msg_stream
						<< "\n---------------------------------------------------------------------------------------"
						<< "\n";
				d_stats_writer->store_data(d_msg_stream.str().size(),
						(void*) d_msg_stream.str().c_str());

			}

			//this if statement will keep track of all the math involved and results for every
			//fingerprint that is analyzed
			if (d_save_stats_switch == SWITCH::ON) {
				clear_stream();
				d_msg_stream
						<< "\n---------------------------------------------------------------------------------------"
						<< "\n";
				d_msg_stream << "Burst: " << d_burst_count + 1
						<< (nan_found ? " contains a NaN\n" : "\n");
				d_msg_stream << "Fingerprints: ";

				for (std::vector<float>::size_type i = 0;
						i != d_dna_fingerprints.size(); i++) {
					if (i < (d_dna_fingerprints.size() - 1)) {
						d_msg_stream << d_dna_fingerprints[i] << ",";

					} else {

						d_msg_stream << d_dna_fingerprints[i] << "\n";

					}

				}
				d_msg_stream << "DNAResults: ";
				for (std::vector<float>::size_type i = 0;
						i != d_dna_results.size(); i++) {
					if (i < (d_dna_results.size() - 1)) {
						d_msg_stream << d_dna_results[i] << ",";

					} else {

						d_msg_stream << d_dna_results[i] << "\n";

					}

				}
				d_msg_stream
						<< "---------------------------------------------------------------------------------------"
						<< "\n";
				d_matrix_results_writer->store_data(d_msg_stream.str().size(),
						(void*) d_msg_stream.str().c_str());

			}

			//Option to actively see the fingerprints &results comming out
			if ((d_debug == DebugType::USRP_ON)
					|| (d_debug == DebugType::ALL_ON)) {
				std::cout
						<< "\n---------------------------------------------------------------------------------------"
						<< "\n";
				std::cout << "Fingerprint #" << d_burst_count + 1 << ":";

				for (std::vector<float>::iterator c =
						d_dna_fingerprints.begin();
						c != d_dna_fingerprints.end(); ++c) {
					std::cout << (*c) << ",";

				}
				std::cout << "\nResults #" << d_burst_count + 1 << ": ";
				for (std::vector<float>::iterator c = d_dna_results.begin();
						c != d_dna_results.end(); ++c) {
					std::cout << (*c) << ",";

				}
				std::cout
						<< "\n---------------------------------------------------------------------------------------"
						<< std::endl;
			}

			d_burst_count++; //Keeping track for statistics

			burst_search();
			break;
		}
		default:
			assert(0);
			break;

		}

	}

	if ((d_debug == DebugType::PACKET_USRP_ON)
			|| (d_debug == DebugType::ALL_ON)) {
		std::cout << "--------------------------------------------------\n";
		std::cout << "Current Preamble Sample Len: "
				<< d_phy_802_15_4.get_preamble_len_samples() << "\n";
		std::cout << "Current Input Size: " << ninput << "\n";
		std::cout << "Current Burst Count: " << d_burst_count << "\n";
		std::cout << "Current Valid Fingerprints: " << d_fingerprints_counter
				<< "\n";
		std::cout << "Current Invalid Fingerprints: " << d_nan_counter << "\n";
		std::cout << "--------------------------------------------------"
				<< std::endl;

	}

	consume(0, noutput_items);

	//Tell run time system how many output items we produced.

	// // Tell runtime system how many input items we consumed on
	// // each input stream.
	// consume_each (noutput_items);

	// Tell runtime system how many output items we produced.
	// return noutput_items;
	return 0;
}

void dna_detector_ccf_impl::burst_search() {
	d_state = SEARCHING;
	d_current_detected_preamble.clear();

}

void dna_detector_ccf_impl::burst_detect() {
	d_state = DETECTED;

}

void dna_detector_ccf_impl::burst_fingerprint() {
	//At this point,I need to perform RF DNA Analysis
	//this is to get rid of the extra zeroes beyond the preamble sample size
	if (d_current_detected_preamble.size()
			> d_phy_802_15_4.get_preamble_len_samples()) {

		d_current_detected_preamble.resize(
				d_phy_802_15_4.get_preamble_len_samples());
	}

	d_state = FINGERPRINT;

}

void dna_detector_ccf_impl::set_devices(const int &devices) {
	d_devices = devices;

}

void dna_detector_ccf_impl::set_dna_row_size(const int &dna_row_size) {
	d_dna_row_size = dna_row_size;

}

void dna_detector_ccf_impl::set_dna_column_size(const int &dna_column_size) {
	d_dna_column_size = dna_column_size;

}

void dna_detector_ccf_impl::set_dna_region_num(const int &dna_region_num) {
	d_dna_region_num = dna_region_num;

}

void dna_detector_ccf_impl::set_debug(const DebugType &debug) {
	d_debug = debug;

	d_usrp_debug = (debug & USRP_ON) > 0 ? 1 : 0;

}

void dna_detector_ccf_impl::set_dna_fingerprint_filename(
		const std::string &fingerprint_filename) {
	d_fingerprint_filename = fingerprint_filename;

}

void dna_detector_ccf_impl::set_dna_fingerprint_save_switch(
		const SWITCH &fingerprint_save_switch) {
	d_fingerprint_save_switch = fingerprint_save_switch;
}

void dna_detector_ccf_impl::set_dna_classification_matrix_filename(
		const std::string &class_matrix_filename) {
	d_class_matrix_filename = class_matrix_filename;

}

void dna_detector_ccf_impl::set_dna_means_matrix_filename(
		const std::string &means_matrix_filename) {
	d_means_matrix_filename = means_matrix_filename;
}

void dna_detector_ccf_impl::set_dna_classification_load_switch(
		const SWITCH &class_matrix_load_switch) {
	d_class_matrix_load_switch = class_matrix_load_switch;
}

void dna_detector_ccf_impl::set_save_stats_switch(
		const SWITCH &save_stats_switch) {
	d_save_stats_switch = save_stats_switch;

}

void dna_detector_ccf_impl::set_save_dna_results_switch(
		const SWITCH &save_dna_results_switch) {
	d_save_dna_results_switch = save_dna_results_switch;

}

void dna_detector_ccf_impl::set_dna_norm_vector_filename(
		const std::string &norm_vector_filename) {
	d_norm_vector_filename = norm_vector_filename;

}

void dna_detector_ccf_impl::set_dna_xoffset_vector_filename(
		const std::string &xoffset_vector_filename) {
	d_xoffset_vector_filename = xoffset_vector_filename;

}

void dna_detector_ccf_impl::set_samp_rate(const float &samp_rate) {
	d_samp_rate = samp_rate;

}

int dna_detector_ccf_impl::get_devices() const {

	return d_devices;
}

int dna_detector_ccf_impl::get_dna_row_size() const {

	return d_dna_row_size;
}

int dna_detector_ccf_impl::get_dna_column_size() const {

	return d_dna_column_size;
}

int dna_detector_ccf_impl::get_dna_region_num() const {

	return d_dna_region_num;
}

DebugType dna_detector_ccf_impl::get_debug() const {

	return d_debug;
}

std::string dna_detector_ccf_impl::get_dna_fingerprint_filename() const {

	return d_fingerprint_filename;
}

SWITCH dna_detector_ccf_impl::get_dna_fingerprint_save_switch() const {

	return d_fingerprint_save_switch;
}

std::string dna_detector_ccf_impl::get_dna_classification_matrix_filename() const {

	return d_class_matrix_filename;
}

std::string dna_detector_ccf_impl::get_dna_means_matrix_filename() const {

	return d_means_matrix_filename;
}

SWITCH dna_detector_ccf_impl::get_dna_classification_load_switch() const {

	return d_class_matrix_load_switch;
}

SWITCH dna_detector_ccf_impl::get_save_stats_switch() const {

	return d_save_stats_switch;
}

SWITCH dna_detector_ccf_impl::get_save_dna_results_switch() const {

	return d_save_dna_results_switch;
}

float dna_detector_ccf_impl::get_samp_rate() const {

	return d_samp_rate;
}

std::string dna_detector_ccf_impl::get_dna_norm_vector_filename() const {

	return d_norm_vector_filename;
}

std::string dna_detector_ccf_impl::get_dna_xoffset_vector_filename() const {

	return d_xoffset_vector_filename;
}

float dna_detector_ccf_impl::constrain_angle(const float &angle) {
	float x = fmod(angle + M_PI, 2 * M_PI);

	if (x < 0) {
		x += 2 * M_PI;

	}

	return x - M_PI;

}

float dna_detector_ccf_impl::angle_conv(const float &angle) {

	return fmod(constrain_angle(angle), 2 * M_PI);
}

float dna_detector_ccf_impl::angle_diff(const float &angle_a,
		const float &angle_b) {
	float dif = fmod(angle_b - angle_a + M_PI, 2 * M_PI);

	if (dif < 0) {
		dif += (2 * M_PI);

	}

	return std::move(dif - M_PI);

}

float dna_detector_ccf_impl::unwrap_phase(const float &prev_angle,
		const float &current_angle) {
	return std::move(
			prev_angle - angle_diff(current_angle, angle_conv(prev_angle)));

}

std::vector<float> dna_detector_ccf_impl::calculate_fingerprints(
		const std::vector<gr_complex> &burst) {
	//Assuming that the sampling rate is @4MS/s, then there should be 640 samples.
	//That preamble will be broken down into 64-samples analyzing the variance, phase,
	//and frequencies variance, skew, and kurtosis such that 9-samples will be produced for
	//every 64-samples.
	//
	//
	//[Variance,Skewness,Kurtosis][Variance,Skewness,Kurtosis][Variance,Skewness,Kurtosis]
	//
	//For example, if each returning block of 9-samples represents 1-subregion and 10-subregion represents a fingerprint
	//i.e. 90-samples returned @4MS/sec
	//Amplitude
	//Phase(rads)
	//Frequency(rads/sec)

	std::vector<float> fingerprint;
	int sub_samples = d_phy_802_15_4.get_preamble_len_samples()
			/ d_dna_region_num;

	std::vector<gr_complex>::const_iterator i;
	std::vector<float> subregion_temp;
	for (int c = 0; c < d_dna_region_num; c++) {
		i = burst.begin(); //setting the itterator to the beginning of the burst

		//Next, the itterator will be offset by a multiple of the subregion size
		//based on the sample rate and passed into an rvalue vector to make
		//the subregion
		subregion_temp = std::move(
				make_subregion(
						std::vector < gr_complex
								> (i + sub_samples * c, i
										+ (c + 1) * sub_samples)));

		//Finally, we append the sub_region to the fingerprint vector

		fingerprint.insert(fingerprint.end(), subregion_temp.begin(),
				subregion_temp.end());
	}

	return std::move(fingerprint);
}

//Accumulators and burst floats were cast to doubles for additional precision
std::vector<float> dna_detector_ccf_impl::make_subregion(
		const std::vector<gr_complex> &burst) {
	std::vector<float> sub_region;
	std::vector<double> temp_phase;

	/*FILE* fp;

	 fp = fopen("/home/user/gr_dump/burst_dump.bin", "ab");
	 if(fp == NULL){
	 std::cout << "Error opening file /home/user/gr_dump/burst_dump.bin\n";
	 }
	 else{
	 int n = fwrite(burst.data(), sizeof(gr_complex), burst.size(), fp);
	 fclose(fp);
	 }
	 */

	/*std::cout << "Burst\n";
	 std::cout << "===========================================================\n";
	 for (std::vector<gr_complex>::size_type i = 0; i != burst.size(); i++)
	 std::cout << burst[i] << " ";
	 std::cout << "\n" <<"===========================================================\n";
	 */

	//complex data was converted to doubles for statistical calculations
	//for additional precision
	std::vector<dna_marker<double>> stats;
	for (int c = 0; c < 3; c++) {

		stats.push_back(std::move(dna_marker<double>()));
	}

	//prev_phase and unwrapped phase will be used to keep
	//track of how many times the IQ unit circle has been
	//traversed
	double prev_phase = 0.0;
	double unwrapped_phase = 0.0;
	for (std::vector<gr_complex>::size_type i = 0; i != burst.size(); i++) {
		unwrapped_phase = unwrap_phase(prev_phase, std::arg(burst[i]));
		stats[0](std::abs(burst[i]));
		stats[1](unwrapped_phase);
		stats[2](d_samp_rate * (unwrapped_phase - prev_phase));
		prev_phase = unwrapped_phase;
	}

	//After all the processing is done, 9-samples (3 per feature) will be pushed
	//onto the vector and returned.
	//all of the statistics were tabulated
	//Each output was cast back to floats after
	for (int c = 0; c < stats.size(); c++) {
		sub_region.push_back(variance(stats[c]));
		sub_region.push_back(skewness(stats[c]));
		sub_region.push_back(kurtosis(stats[c]));

	}

	return std::move(sub_region);
}

std::vector<float> dna_detector_ccf_impl::fingerprint_emitter(
		const std::vector<float> &fingerprint) {

	std::vector<float> emitter_results;

	auto fingerprint_vector = EigRowVec<float>::Map(fingerprint.data(),
			fingerprint.size());
	auto adjusted_fingerprint_vector = fingerprint_vector - d_xoffset_vector;
	auto results = (adjusted_fingerprint_vector.cwiseProduct(d_norm_vector))
			* d_dna_matrix;

	for (int c = 0; c < d_devices; c++) {
		//using Euclidiean distance, c=(a^2-b^2)^.5,
		//by pulling a row of the means matrix that corresponds to each device,
		//perform a n element-wise difference and squaring of the differences,
		//summing the squared difference results, and finally getting the root
		//of the sum.
		auto row_diff = d_class_means_matrix.row(c) - results;
		auto row_squared = row_diff.cwiseProduct(row_diff);
		auto row_sum = row_squared.rowwise().sum();
		auto row_sqrt = row_sum.cwiseSqrt();
		auto row_results = row_sqrt.value();

		emitter_results.push_back(row_results);
	}

	return std::move(emitter_results);
}

void dna_detector_ccf_impl::clear_stream() {

	d_msg_stream.str(std::string());
}

} /* namespace ieee802_15_4 */
} /* namespace gr */

