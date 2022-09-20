/* -*- c++ -*- */
/*
 * Copyright 2022 lflb.
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
#include "preamble_sink_impl.h"

namespace gr {
namespace ieee802_15_4 {

preamble_sink::sptr preamble_sink::make(float samp_rate, unsigned int threshold,
		unsigned int buffer, float burst_trigger, DebugType debug, CRC crc,
		SAVE save_stats_switch, std::string save_stats_file) {
	return gnuradio::get_initial_sptr(
			new preamble_sink_impl(samp_rate, threshold, buffer, burst_trigger,
					debug, crc, save_stats_switch, save_stats_file));
}

/*
 * The private constructor
 */
preamble_sink_impl::preamble_sink_impl(float samp_rate, unsigned int threshold,
		unsigned int buffer_size, float burst_trigger, DebugType debug, CRC crc,
		SAVE save_stats_switch, std::string save_stats_file) :
		gr::block("preamble_sink",
				gr::io_signature::make2(2, 2, sizeof(float),
						sizeof(gr_complex)),
				gr::io_signature::make2(2, 2, sizeof(gr_complex),
						sizeof(gr_complex))) {
	d_sync_vector = 0xA7;
	d_samp_rate = samp_rate;
	d_threshold = threshold;
	d_save_stats_file = save_stats_file;
	d_burst_trigger = burst_trigger;
	set_debug(debug);
	set_crc(crc);
	set_save_stats_switch(save_stats_switch);
	set_buffer_size(buffer_size);

	// Set sample rate for our packet size calculations
	d_phy_802_15_4.set_sample_rate(d_samp_rate);

	std::cout << "\nPreamble_sink sample rate set to " << samp_rate
			<< std::endl;

	//file name and save settings set here
	if (d_save_stats_switch == SAVE::YES) {
		//from stackoverflow.com, just a quick way to append time/date to a filename
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		d_msg_stream << "_" << std::put_time(&tm, "%d%b%y_%R") << ".txt";
		d_save_stats_file.append(d_msg_stream.str());
		clear_stream();

		std::cout << "\nFile stats set to save at " << d_save_stats_file
				<< ".\n" << std::endl;
		d_stats_writer = std::make_shared < file_store_impl
				> (sizeof(char), d_save_stats_file.c_str(), false, false);

		d_msg_stream
				<< "\n======================================================================================="
				<< "\n ";
		d_msg_stream << "Collection started on " << std::put_time(&tm, "%F_%T");
		d_msg_stream << "\nBuffer size set to " << d_buffer_size;
		d_msg_stream << "\nSample Rate at " << samp_rate << " Samp/sec";
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
	}

	//Link Quality Information

	d_lqi = 0;
	d_lqi_sample_count = 0;

	//need to convert to a int for this to work

	set_output_multiple((int) round(d_phy_802_15_4.get_sym_to_chip_mult()));
	set_min_output_buffer(1, 4088l); //hard coded for now to ensure a proper output for passthrough usrp data... need to fix later

	if (d_chip_debug) {

		fprintf(stderr, "syncvec: %x, threshold: %d\n", d_sync_vector,
				d_threshold), fflush(stderr);
	}
	enter_search();

	message_port_register_out(pmt::mp("out"));
}

/*
 * Our virtual destructor.
 */
preamble_sink_impl::~preamble_sink_impl() {
}

void preamble_sink_impl::forecast(int noutput_items,
		gr_vector_int &ninput_items_required) {
	/* <+forecast+> e.g. ninput_items_required[0] = noutput_items */

	/* * * * * * * * * * * * * * * * * * * * * * * * * Special Note* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	 * * *Need to see if this is adjustable before and after compilation for different Samp Rates * * * * *
	 **********************************************************************************************/

	//need to fix ieee802_15_4_packet class to better handle inputs
	//original implementation was blowing up block
	// now this needs to be hard coded for the moment
	ninput_items_required[0] = 1022;
	ninput_items_required[1] = 2044;

}

int preamble_sink_impl::general_work(int noutput_items,
		gr_vector_int &ninput_items, gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items) {
	const float *inbuf = (const float*) input_items[0]; // input stream M & M stream of floats for detection
	const gr_complex *in_rx = (const gr_complex*) input_items[1]; // input stream of RX data

	int ninput = ninput_items[0]; // input number of detected float data
	int ninput_rx = ninput_items[1]; // input number of RX data

	gr_complex *out_rx = (gr_complex*) output_items[0]; // output RX stream for detected preambles
	gr_complex *out_usrp = (gr_complex*) output_items[1]; // input to output raw stream

	// Do <+signal processing+>

	int count = 0;
	static std::vector<int> preamble_marker; // used to store preamble markers per each general_work itteration
	static std::vector<int> sync_marker; // used to store the sync markers per general_work itteration
	static std::vector<int> header_marker; // used to store the header markers per general_work itteration

	int i = 0;

	if (d_chip_debug)

		fprintf(stderr, ">>>Entering state machine\n"), fflush(stderr);

	// The burst buffer is going to determine how many previous bursts will be "searched"
	// through once a proper packet has been detected. If the buffer is bigger than the
	// user set limit, the FIFO structure will pop off the front burst samples
	if (d_prev_usrp_collect_buffer.get_burst_length() > d_buffer_size) {

		d_prev_usrp_collect_buffer.pop_front_burst_samples();
	}
	while (count < ninput) {
		switch (d_state) {

		case STATE_SYNC_SEARCH:
			// Look for sync vector
			if (d_chip_debug)
				fprintf(stderr, "SYNC Search, ninput=%d syncvec=%x \n", ninput,
						d_sync_vector), fflush(stderr);

			while (count < ninput) {

				if (slice(inbuf[count++]))
					d_shift_reg = (d_shift_reg << 1) | 1;

				else
					d_shift_reg = d_shift_reg << 1;

				if (d_preamble_cnt > 0) {
					d_chip_cnt = d_chip_cnt + 1;

				} //The first if block syncronizes to chip sequences.

				if (d_preamble_cnt == 0) {
					unsigned int threshold;
					threshold = gr::blocks::count_bits32(
							(d_shift_reg & 0x7FFFFFFE)
									^ (CHIP_MAPPING[0] & 0x7FFFFFFE));
					if (threshold < d_threshold) {
						//
						// if((d_shift_reg & 0xFFFFFE ) == (CHIP_MAPPING[0] & 0xFFFFFE ) ) {
						if (d_pack_debug)
							fprintf(stderr,
									"Threshold %d d_preamble_cnt : %d\n ",
									threshold, d_preamble_cnt);
						//fprintf(stderr, "Found 0 in chip sequence\n"), fflush( //FIXME: Desligado
						//		stderr);

						// we found a 0 in the chip sequence
						d_preamble_cnt += 1;
						// fprintf(stderr, "Threshold %d d_preamble_cnt: %d\n", threshold, d_preamble_cnt);

					}
				} else {
					// we found the first 0, thus we only have to do the calculation every 32 chips
					if (d_chip_cnt == 32) {

						d_chip_cnt = 0;

						if (d_packet_byte == 0) {
							if (gr::blocks::count_bits32(
									(d_shift_reg & 0x7FFFFFFE)
											^ (CHIP_MAPPING[0] & 0xFFFFFFFE))
									<= d_threshold) {

								if (d_pack_debug)
									fprintf(stderr,
											"Found %d 0 in chip sequence\n",
											d_preamble_cnt), fflush(stderr);

								// we found an other 0 in the chip sequence

								d_packet_byte = 0;
								d_preamble_cnt++;

							} else if (gr::blocks::count_bits32(
									(d_shift_reg & 0x7FFFFFFE)
											^ (CHIP_MAPPING[7] & 0xFFFFFFFE))
									<= d_threshold) {

								if (d_pack_debug)

									fprintf(stderr, "Found first SFD\n"), fflush(
											stderr);
								d_packet_byte = 7 << 4;

							} else {
								// we are not in the synchronization header
								if (d_pack_debug)

									fprintf(stderr,
											"Wrong first byte of SFD.%u\n",
											d_shift_reg), fflush(stderr);

								enter_search();

								break;
							}

						} else {
							if (gr::blocks::count_bits32(
									(d_shift_reg & 0x7FFFFFFE)
											^ (CHIP_MAPPING[10] & 0xFFFFFFFE))
									<= d_threshold) {
								d_packet_byte |= 0xA;
								if (d_pack_debug)
									fprintf(stderr, "Found sync, 0x%x\n",
											d_packet_byte), fflush(stderr);

								// found SFD
								// setup for header decode

								// each count number will be used to find
								// a relation between the usrp preambles
								if (d_debug) {

									preamble_marker.push_back(count);
								} // if there's any data in from the previous run it will be appended as
								  // chances are that it will contain the preamble with the current data,
								  // only the previous input will be passed through as a safety precaution
								  // if the previously collected buffer doesn't exist then only the current
								  // data will be passed with the possibility of not receiving the preamble
								if (d_prev_usrp_collect_buffer.is_activated()) {
									d_prev_buffer_activated = true;
									d_current_preamble_buffer.append_buffer_data(
											d_prev_usrp_collect_buffer);
									for (int c = 0; c < ninput_rx; c++) {
										d_current_preamble_buffer.append_buffer_data(
												in_rx[c]);

									}

								} else {

									d_prev_buffer_activated = false;
									d_current_preamble_buffer.store_preamble_data(
											ninput_rx, in_rx);
								}

								enter_have_sync();
								break;
							} else {

								if (d_chip_debug)

									fprintf(stderr,
											"Wrong second byte of SFD.%u\n",
											d_shift_reg), fflush(stderr);

								enter_search();

								break;
							}

						}

					}

				}

			}
			break;

		case STATE_HAVE_SYNC:
			if (d_pack_debug)
				fprintf(stderr, "Header Search bitcnt=%d, header=0x%08x\n",
						d_headerbitlen_cnt, d_header), fflush(stderr);

			while (count < ninput) {
				// Decode the bytes one after another.
				if (slice(inbuf[count++]))
					d_shift_reg = (d_shift_reg << 1) | 1;
				else
					d_shift_reg = d_shift_reg << 1;

				d_chip_cnt = d_chip_cnt + 1;

				if (d_chip_cnt == 32) {
					d_chip_cnt = 0;
					unsigned char c = decode_chips(d_shift_reg);
					if (c == 0xFF) {
						// something is wrong. restart the search for a sync
						if (d_pack_debug)

							fprintf(stderr,
									"Found a not valid chip sequence!%u\n",
									d_shift_reg), fflush(stderr);

						enter_search();
						break;
					}

					if (d_packet_byte_index == 0) {

						d_packet_byte = c;

					} else {

						// c is always < 15

						d_packet_byte |= c << 4;

					}
					d_packet_byte_index = d_packet_byte_index + 1;
					if (d_packet_byte_index % 2 == 0) {
						// we have a complete byte which represents the frame length.
						int frame_len = d_packet_byte;
						if (frame_len <= MAX_PKT_LEN) {

							// keep track of the found sync markers
							// to see correlate to good usrp markers
							if (d_debug) {

								sync_marker.push_back(count);
							}

							enter_have_header(frame_len);
						} else {

							enter_search();
						}

						break;
					}

				}

			}
			break;

		case STATE_HAVE_HEADER:
			if (d_pack_debug)
				fprintf(stderr,
						"Packet Build count=%d, ninput=%d, packet_len=%d\n",
						count, ninput, d_packetlen), fflush(stderr);

			while (count < ninput) {
				//shift bits into bytes of packet one at a time
				if (slice(inbuf[count++]))
					d_shift_reg = (d_shift_reg << 1) | 1;
				else
					d_shift_reg = d_shift_reg << 1;

				d_chip_cnt = (d_chip_cnt + 1) % 32;

				if (d_chip_cnt == 0) {
					unsigned char c = decode_chips(d_shift_reg);
					if (c == 0xff) {
						// something is wrong. restart the search for a sync
						if (d_pack_debug)

							fprintf(stderr,
									"Found a not valid chip sequence! %u\n",
									d_shift_reg), fflush(stderr);

						enter_search();
						break;

					}
					//the first symbol represents the first part of the byte.
					if (d_packet_byte_index == 0) {
						d_packet_byte = c;

					} else {
						// c is always < 15

						d_packet_byte |= c << 4;

					}
					//fprintf(stderr, "%d:0x%x\n", d_packet_byte_index , c ) ;
					d_packet_byte_index = d_packet_byte_index + 1;
					if (d_packet_byte_index % 2 == 0) {
						// we have a complete byte
						if (d_pack_debug)

							fprintf(stderr,
									"packetcnt:%d, payloadcnt:%d, payload 0x%x, d_packet_byte_index: %d\n",
									d_packetlen_cnt, d_payload_cnt,
									d_packet_byte, d_packet_byte_index), fflush(
									stderr);

						d_packet[d_packetlen_cnt++] = d_packet_byte;
						d_payload_cnt++;
						d_packet_byte_index = 0;

						if (d_payload_cnt >= d_packetlen) {
							// packet is filled, including CRC. might do check later in here
							unsigned int scaled_lqi = (d_lqi / MAX_LQI_SAMPLES)
									<< 3;
							unsigned char lqi = (
									scaled_lqi >= 256 ? 255 : scaled_lqi);

							pmt::pmt_t meta = pmt::make_dict();
							meta = pmt::dict_add(meta, pmt::mp("lqi"),
									pmt::from_long(lqi));

							std::memcpy(buf, d_packet, d_packetlen_cnt);
							pmt::pmt_t payload = pmt::make_blob(buf,
									d_packetlen_cnt);

							// * * * * * * * * * * * * * * Pass and Analyze Preamble Data Here ! ! ! ! * * * * * * * * * * * * * * * * * * * * *
							// the header count will be used as a way to
							// correlate the header count with the USRP
							// data
							if (d_debug) {

								header_marker.push_back(count);
							}

							std::tuple<dna_buffer, bool> buffer_data;

							if (d_prev_buffer_activated) {
								buffer_data = std::move(
										normalize_preamble(
												d_current_preamble_buffer,
												ninput_rx, d_burst_trigger));
							}

							d_preamble_processed = std::get < 1 > (buffer_data);

							// based on the last run, it will determine if a
							// preamble met the criteria to be sent or not
							if (d_preamble_processed == true) {
								d_sent_preambles++;
							} else {
								d_removed_preambles++;
							}

							// raw data crc will be checked for validity and filtering
							// later on
							uint16_t crc_check = crc16(buf, d_packetlen_cnt);

							// if-else statements are a way to keep track of processing errors
							// of the general_work function based on mac_in
							// function from mac.cc from gr-ieee802.15.4
							d_recent_packet_length = d_packetlen;
							if (d_packetlen_cnt < 9) {
								d_short_payload++;
							} else if (crc_check) {
								d_crc_bad_payload++;
							} else {
								d_crc_good_payload++;
							}

							// the if / else conditions will check to see if the user will
							// accept only properly sized crc verified packet outputs or
							// any and everything that comes in packet-wise
							if (d_crc_set) {
								if (!crc_check && (d_packetlen_cnt < 9)) {
									// might make a function later on to handle empty values
									// but a loop will suffice for now
									for (int c = 0; c < ninput_rx; c++) {
										d_crc_preamble_buffer[c] = gr_complex(
												0.0f, 0.0f);

									}

								} else {
									d_crc_preamble_buffer =
											d_prev_buffer_activated ?
													std::move(
															std::get < 0
																	> (buffer_data)) :
													std::move(
															d_current_preamble_buffer);

								}
							} else {
								d_crc_preamble_buffer =
										d_prev_buffer_activated ?
												std::move(
														std::get < 0
																> (buffer_data)) :
												std::move(
														d_current_preamble_buffer);
							}

							// debug check copied directly from mac.cc
							// from gr-ieee802.15.4
							if (d_usrp_debug) {
								if (d_packetlen_cnt < 9) {
									fprintf(stderr,
											"\nDetected packet MAC frame too short"), fflush(
											stderr);
								}

								if (crc_check) {
									fprintf(stderr,
											"\nDetected packet MAC failed CRC check"), fflush(
											stderr);
								} else {
									fprintf(stderr,
											"\nDetected packet MAC passed CRC check\n"), fflush(
											stderr);
								}

							}

							if (d_save_stats_switch == SAVE::YES) {
								clear_stream();
								d_msg_stream
										<< "\n======================================================================================="
										<< "\n ";
								d_msg_stream
										<< " Current Burst Sample Length : "
										<< ninput_rx << "\n ";

								if (d_prev_buffer_activated) {
									d_msg_stream
											<< " Buffer + Detected Burst Sample length:";
								} else {
									d_msg_stream << " Detected Burst Sample: ";
								}
								d_msg_stream
										<< d_current_preamble_buffer.get_sample_length()
										<< "\n ";
								d_msg_stream << "Total Packets Detected: "
										<< d_short_payload + d_crc_bad_payload
												+ d_crc_good_payload << "\n";
								d_msg_stream << "Short Packets: "
										<< d_short_payload << "\n ";
								d_msg_stream << "Bad CRC Packets: "
										<< d_crc_bad_payload << "\n ";
								d_msg_stream << "Good CRC Packets: "
										<< d_crc_good_payload << "\n ";

								d_msg_stream
										<< " Total Preambles Processed "
										<< "\n ";
								d_msg_stream << "of "
										<< d_short_payload + d_crc_bad_payload
												+ d_crc_good_payload
										<< " Total Packets Detected : ";
								d_msg_stream
										<< d_removed_preambles
												+ d_sent_preambles << "\n ";
								d_msg_stream << " Preambles Accepted: "
										<< d_sent_preambles << "\n ";
								d_msg_stream << " Preambles Rejected: "
										<< d_removed_preambles << "\n ";
								d_msg_stream
										<< "======================================================================================="
										<< "\n ";
								d_stats_writer->store_data(
										d_msg_stream.str().size(),
										(void*) d_msg_stream.str().c_str());

							}

							message_port_pub(pmt::mp("out"),
									pmt::cons(meta, payload));

							if (d_pack_debug)
								fprintf(stderr,
										"Adding message of size %d to queue\n",
										d_packetlen_cnt);

							enter_search();

							break;
						}

					}

				}

			}
			break;

		default:
			assert(0);
			break;

		}

	}

	if (d_pack_debug)
		fprintf(stderr, "Samples Processed: %d\n ", ninput_items[0]), fflush(
				stderr);

	consume(0, ninput_items[0]);

	// Debug & Itteration Statistics for the blocks current run

	if (d_usrp_debug && (d_crc_preamble_buffer.is_activated())) {
		fprintf(stderr, "++++++++++++++++++++++++++++++++++++++++++++++++++"), fflush(
				stderr);
		fprintf(stderr, "\nM&M Timer Input Count:%d\tUSRP Input Count : %d",
				ninput, ninput_rx), fflush(stderr);
		fprintf(stderr, "\nInheritted Output Item Count: %d", noutput_items), fflush(
				stderr);

		fprintf(stderr, "\nPacket Length: %d", d_recent_packet_length), fflush(
				stderr);
		fprintf(stderr, "\nTotal Processed Preambles: %d",
				(d_removed_preambles + d_sent_preambles)), fflush(stderr);
		fprintf(stderr, "\nDropped Preambles: %d", d_removed_preambles), fflush(
				stderr);
		fprintf(stderr, "\nSent Preambles : %d", d_sent_preambles), fflush(
				stderr);

		vector_print("\nPreamble", preamble_marker);
		vector_print("Sync", sync_marker);
		vector_print("Header", header_marker);

		fprintf(stderr,
				"\nTotal Run Numbers => Short Packets : %d\tBad CRC Packets : %d\tGood CRC Packets : %d\n",
				d_short_payload, d_crc_bad_payload, d_crc_good_payload), fflush(
				stderr);

		preamble_marker.clear();
		sync_marker.clear();
		header_marker.clear();

		fprintf(stderr, "--------------------------------------------------"), fflush(
				stderr);
	}

	// The current input will be appended here to the FIFO buffer sturcture for the
	// next itteration.
	d_prev_usrp_collect_buffer.append_burst_sample_size(ninput_rx);

	for (int c = 0; c < ninput_rx; c++) {
		out_rx[c] =
				d_crc_preamble_buffer.is_activated() ?
						d_crc_preamble_buffer[c] : gr_complex(0.0F, 0.0F);

		d_prev_usrp_collect_buffer.append_buffer_data(in_rx[c]);
		out_usrp[c] = in_rx[c];
	}

	d_crc_preamble_buffer.unset_buffer();

	consume(0, 0);
	consume(1, ninput_rx);

	return ninput_rx;

	// // Tell runtime system how many input items we consumed on
	// // each input stream.
	// consume_each (noutput_items);

	// // Tell runtime system how many output items we produced.
	// return noutput_items;
}

void preamble_sink_impl::enter_search() {
	if (d_chip_debug) {
		fprintf(stderr, "@enter_search\n");
	}

	d_state = STATE_SYNC_SEARCH;

	d_shift_reg = 0;
	d_preamble_cnt = 0;
	d_chip_cnt = 0;
	d_packet_byte = 0;

	d_current_preamble_buffer.unset_buffer();
}

void preamble_sink_impl::enter_have_sync() {

	if (d_chip_debug) {
		fprintf(stderr, "@enter_have_sync\n");
	}

	d_state = STATE_HAVE_SYNC;
	d_packetlen_cnt = 0;
	d_packet_byte = 0;
	d_packet_byte_index = 0;

	// Link Quality Information

	d_lqi = 0;
	d_lqi_sample_count = 0;

}

void preamble_sink_impl::enter_have_header(const int &payload_len) {

	if (d_chip_debug)
		fprintf(stderr, "@enter_have_header ( payload_len = %d )\n",
				payload_len);

	d_state = STATE_HAVE_HEADER;
	d_packetlen = payload_len;
	d_payload_cnt = 0;
	d_packet_byte = 0;
	d_packet_byte_index = 0;

}

unsigned char preamble_sink_impl::decode_chips(const unsigned int &chips) {
	int i;
	int best_match = 0xFF;
	int min_threshold = 33; // Matching to 32 chips , could never have a e r r o r o f 33 chips965
	for (i = 0; i < 16; i++) {
		// FIXME : we can store the last chip

		// ignore the first and last chip since it depends on the last chip .
		unsigned int threshold = gr::blocks::count_bits32(
				(chips & 0x7FFFFFFE) ^ (CHIP_MAPPING[i] & 0x7FFFFFFE));

		if (threshold < min_threshold) {
			best_match = i;

			min_threshold = threshold;

		}

	}

	if (min_threshold < d_threshold) {

		if (d_chip_debug) {
			fprintf(stderr, "Found sequence with %d errors at 0x%x\n",
					min_threshold,
					(chips & 0x7FFFFFFE)
							^ (CHIP_MAPPING[best_match] & 0x7FFFFFFE)), fflush(
					stderr);
		}

		// LQI : Average number of chips correct * MAX_LQI_SAMPLES
		//
		if (d_lqi_sample_count < MAX_LQI_SAMPLES) {
			d_lqi += 32 - min_threshold;
			d_lqi_sample_count++;
		}

		return std::move((char) best_match & 0xF);
	}

	return std::move(0xFF);
}

int preamble_sink_impl::slice(const float &x) {
	return std::move(x > 0 ? 1 : 0);

}

void preamble_sink_impl::set_threshold(const unsigned int &threshold) {
	d_threshold = threshold;

}

unsigned int preamble_sink_impl::get_threshold() const {

	return d_threshold;
}

void preamble_sink_impl::set_samp_rate(const float &samp_rate) {
	d_samp_rate = samp_rate;
	d_phy_802_15_4.set_sample_rate(samp_rate);

}

void preamble_sink_impl::set_buffer_size(const unsigned int &buffer_size) {
	if (buffer_size <= 0) {
		std::cout << "\nBuffer size too small... It's now set to 1"
				<< std::endl;

		d_buffer_size = 1;

	} else {
		d_buffer_size = buffer_size;

	}

}

unsigned int preamble_sink_impl::get_buffer_size() const {

	return d_buffer_size;
}

float preamble_sink_impl::get_samp_rate() const {

	return d_samp_rate;
}

void preamble_sink_impl::set_burst_trigger(const float &burst_trigger) {
	d_burst_trigger = burst_trigger;

}

float preamble_sink_impl::get_burst_trigger() const {

	return d_burst_trigger;
}

void preamble_sink_impl::set_debug(const DebugType &debug) {
	d_debug = debug;

	// quick bitwise comparison will be used to determine

	// what level of debuggin will be established
	d_chip_debug = (debug & CHIP_ON) > 0 ? 1 : 0;
	d_pack_debug = (debug & PACKET_ON) > 0 ? 1 : 0;
	d_usrp_debug = (debug & USRP_ON) > 0 ? 1 : 0;

}
DebugType preamble_sink_impl::get_debug() const {

	return d_debug;
}

void preamble_sink_impl::set_crc(const CRC &crc) {
	d_crc = crc;

	d_crc_set = (crc & CRC_ONLY) > 0 ? 1 : 0;

}

void preamble_sink_impl::set_save_stats_switch(const SAVE &save_stats_switch) {
	d_save_stats_switch = save_stats_switch;

	d_save_switch_set = (save_stats_switch > 0) ? 1 : 0;

}

void preamble_sink_impl::set_save_stats_file(
		const std::string &save_stats_file) {
	d_save_stats_file = save_stats_file;
}

SAVE preamble_sink_impl::get_save_stats_switch() const {

	return d_save_stats_switch;
}

std::string preamble_sink_impl::get_save_stats_file() const {

	return d_save_stats_file;
}

CRC preamble_sink_impl::get_crc() const {

	return d_crc;
}

uint16_t preamble_sink_impl::crc16(const char *buf, const int &len) {

	uint16_t crc = 0;

	for (int i = 0; i < len; i++) {
		for (int k = 0; k < 8; k++) {

			int input_bit = (!!(buf[i] & (1 << k)) ^ (crc & 1));
			crc = crc >> 1;
			if (input_bit) {
				crc ^= (1 << 15);
				crc ^= (1 << 10);
				crc ^= (1 << 3);
			}

		}

	}

	return std::move(crc);
}

void preamble_sink_impl::clear_stream() {

	d_msg_stream.str(std::string());
}

void preamble_sink_impl::vector_print(const std::string &label,
		const std::vector<int> &input) {
	if (input.empty()) {
		fprintf(stderr, "No %s Detected\n", label.c_str()), fflush(stderr);

	} else {
		fprintf(stderr, "%s Detected at : ", label.c_str());
		for (auto i : input) {

			fprintf(stderr, "%d ", i);
		}

		fprintf(stderr, "\n"), fflush(stderr);
	}

}

std::tuple<dna_buffer, bool> preamble_sink_impl::normalize_preamble(
		const dna_buffer &buffer, const int &max_return_size,
		const float &burst_trigger) {
	dna_buffer position_adjusted_vector;
	float absolute_magnitude = 0.0f;
	int absolute_magnitude_location = 0;
	int burst_trigger_location = 0;
	float temp_magnitude = 0;
	bool preamble_processed = false;

	/************************************************************
	 Add Proper filter_burst Burst Detector Down the Road ! ! ! ! !
	 ***********************************************************/

	// first step is to go through and find the min/max of the burst
	for (int c = 0; c < buffer.get_sample_length(); c++) {
		temp_magnitude = std::fabs(buffer[c]);

		if (temp_magnitude > absolute_magnitude) {
			absolute_magnitude = temp_magnitude;

			absolute_magnitude_location = c;

		}

	}

	// then we divide by the largest magnitude to normalize the burst
	// if the signal is pure noise, then it's going to return a burst of
	// pure noise
	bool trigger_set = false;
	for (int c = 0; c < buffer.get_sample_length(); c++) {
		temp_magnitude = std::fabs(buffer[c]);
		position_adjusted_vector.append_buffer_data(buffer[c]);

		// need a flag to ensure loop doesn't keep modifing burst point

		if ((temp_magnitude / absolute_magnitude >= burst_trigger)
				&& (trigger_set == false)) {
			trigger_set = true;
			burst_trigger_location = c;
		}

	}

	int buffer_stop_element = d_phy_802_15_4.get_preamble_len_samples() + // 640 samples for SHR @4MS/ s
			burst_trigger_location;

	int zero_buffer_length = 0;
	if ((buffer_stop_element + 1)
			> position_adjusted_vector.get_sample_length()) {
		// case where the preamble is not available because it's index is beyond the
		// current buffers max
		position_adjusted_vector.unset_buffer();
		position_adjusted_vector.insert_rear_spacing(max_return_size);

	} else {
		// case where we have more than enough samples to capture the preambe

		// first we're going to remove the elements beyond the preamble location , then
		// we ' re going to remove the front where it will most likely be noise
		position_adjusted_vector.erase_buffer_range(buffer_stop_element,
				position_adjusted_vector.get_sample_length() - 1);
		position_adjusted_vector.erase_buffer_range(0,
				burst_trigger_location - 1);

		// Afterwards we will set the remainder of the input vector to zero and returned
		// the normalized and non-normalized shifted preamble data
		zero_buffer_length = max_return_size
				- position_adjusted_vector.get_sample_length();
		position_adjusted_vector.insert_rear_spacing(zero_buffer_length);
		preamble_processed = true;

	}

	return std::move(
			std::make_tuple(position_adjusted_vector, preamble_processed));
}

std::tuple<dna_buffer, int> preamble_sink_impl::filter_burst(
		const dna_buffer &buffer, const int &filter_taps) {
	// First , we need to generate the averaging filter taps by creating a vector
	// that's the length of the input of filter taps with each tap weighted 1/ filter_taps
	std::vector<float> filter;
	for (int c = 0; c < filter_taps; c++) {
		filter.push_back(1.0f / filter_taps);

	}

	// these two values will be used to keep track
	// of the maximum magnitude while the signal is being filtered
	float absolute_max_value = 0.0f;
	float temp_max_value = 0.0f;
	int absolute_max_value_position = 0;

	// the length of a discrete signal after convolution is the sum of the two signals less
	// one element
	std::vector<float> filtered_burst;
	float burst_sum = 0.0f;

	for (int i = 0; i < buffer.get_sample_length(); i++) {
		burst_sum = 0.0f;

		for (int j = 0; j < filter_taps; j++) {
			if (((i - j) > 0) && (i >= (filter_taps - 1))) {
				burst_sum += std::fabs(buffer[i - j]) * filter[j];

			}

		}

		filtered_burst.push_back(burst_sum);
	}

}

} /* namespace ieee802_15_4 */
} /* namespace gr */

