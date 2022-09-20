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

#ifndef INCLUDED_IEEE802_15_4_PREAMBLE_SINK_IMPL_H
#define INCLUDED_IEEE802_15_4_PREAMBLE_SINK_IMPL_H

#include <ieee802_15_4/preamble_sink.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <gnuradio/blocks/count_bits.h>
#include <iostream>
#include <tuple>
#include <cmath>
#include <sstream>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <typeinfo>
#include <iomanip>
#include <ctime>

// win32 (mingw/msvc) specific
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef O_BINARY
#define OUR_O_BINARY O_BINARY
#else
#define OUR_O_BINARY 0
#endif

//should be handled via configure
#ifdef O_LARGEFILE
#define OUR_O_LARGEFILE O_LARGEFILE
#else
#define OUR_O_LARGEFILE 0
#endif

#include "base_packet.h"
#include "dna_buffer.h"
//#include "file_read_impl.h"
#include "file_store_impl.h"

/* !
 * Class: preamble_sink_impl
 * Author:Schmid,Thomas;Bloessl,Bastian;Cruz,Frankie
 * Description: the class is the implementation file of
 * preamble_sink.h. See preamble_sink.h
 * for more information about this class
 * Generated: Thurs Nov 8 15:30:18 2018 Wed Nov 28 10:30:11 2018
 * Modified: Dec 01 13:30:01 2018
 * Build Version: 1.6
 * Changes : See preamble_sink.h
 */

// this is the mapping between chips and symbols if we do
// a fm demodulation of the O−QPSK signal. Note that this
// is different than the O−QPSK chip sequence from the
// 802.15.4 standard since there there is a translation
// happening.
// See "CMOS RFIC Architectures for IEEE802.15.4 Networks",
// John Notor, Anthony Caviglia, Gary Levy, for more details.
static const unsigned int CHIP_MAPPING[] = { 1618456172, 1309113062, 1826650030,
		1724778362, 778887287, 2061946375, 2007919840, 125494990, 529027475,
		838370585, 320833617, 422705285, 1368596360, 85537272, 139563807,
		2021988657 };

static const int MAX_PKT_LEN = 128 - 1; // remove header and CRC
static const int MAX_LQI_SAMPLES = 8; // Number o f chip c o r r el a ti o n samples to take


namespace gr {
	namespace ieee802_15_4 {

		class preamble_sink_impl : public preamble_sink
		{
		private:
			enum {
				STATE_SYNC_SEARCH, STATE_HAVE_SYNC, STATE_HAVE_HEADER
			} d_state;

			unsigned int d_sync_vector; // 802.15.4 standard is 4x0 bytes and 1x0xA7
			unsigned int d_threshold = 10; // how many bits may be wrong in sync vector

			unsigned int d_shift_reg; // used to look for sync_vector
			int d_preamble_cnt; // count on where we are in preamble
			int d_chip_cnt; // counts the chips collected

			unsigned int d_header; // header bits
			int d_headerbitlen_cnt; // how many so far

			unsigned char d_packet[MAX_PKT_LEN]; // assembled payload
			unsigned char d_packet_byte; // byte being assembled
			int d_packet_byte_index; // which bit of d_packet_byte we're working on
			int d_packetlen; // length of packet
			int d_packetlen_cnt; // how many so far
			int d_payload_cnt; // how many bytes in payload

			unsigned int d_lqi; // Link Quality Information
			unsigned int d_lqi_sample_count;

			float d_burst_trigger = 0.3f; // default preamble burst trigger
			float d_samp_rate = 4e6f; // block system's sample rate
			int d_chip_debug = 0; // very verbose output for almost each sample
			int d_pack_debug = 0; // less verbose output for higher level debugging
			int d_usrp_debug = 0; // USRP verbose output for debugging
			int d_crc_set = 0; // CRC data input setting
			int d_save_switch_set = 0; // Save switch input setting
			int d_crc_good_payload = 0; // good data packet counter
			int d_crc_bad_payload = 0; // bad data packet counter
			int d_short_payload = 0; // short payload counter
			int d_recent_packet_length = 0; // sanity check for packet length
			int d_removed_preambles = 0; // detected preambles that were too short to process
			int d_sent_preambles = 0; // detected preambles that were sent for RFDNA processing
			bool d_preamble_processed = false; // Activates preamble processing
			unsigned int d_buffer_size; // previous storage buffer leng h
			bool d_prev_buffer_activated = false;

			DebugType d_debug = ALL_OFF; //Default debug state
			CRC d_crc = ALL_DATA; // All on by default
			ieee802_15_4_packet d_phy_802_15_4; // ieee802.15.4 packet with size info

			SAVE d_save_stats_switch = NO; // off by default
			std::string d_save_stats_file; // file location where to save stats
			std::stringstream d_msg_stream; // handles string messages
			std::shared_ptr<file_store_impl> d_stats_writer; // for statistics output file

			// FIX ME
			char buf[256]; // PDDU Packet Buffer
			dna_buffer d_prev_usrp_collect_buffer; // previous run usrp buffered data
			dna_buffer d_current_preamble_buffer; // current run's detected buffer data for crc processing
			dna_buffer d_crc_preamble_buffer; // crc evaluated buffered data

			// enter_search , enter_have_sync ,
			// and enter_have_header are state
			// functions used to set the private
			// variables based on if the RX
			// signal is searching , found , and
			// parsed ieee802.15.4 packets
			void enter_search();
			void enter_have_sync();
			void enter_have_header(const int &payload_len);

		public:
			preamble_sink_impl(float samp_rate, unsigned int threshold, unsigned int buffer, float burst_trigger, DebugType debug, CRC crc, SAVE save_stats_switch, std::string save_stats_file);
			~preamble_sink_impl();

			// Where all the action really happens
			void forecast (int noutput_items, gr_vector_int &ninput_items_required);

			int general_work(int noutput_items,
					gr_vector_int &ninput_items,
					gr_vector_const_void_star &input_items,
					gr_vector_void_star &output_items);

			// decode_chips takes in the value of the RX
			// chips as an integer and performs a XOR comparison
			// between the CHIP_MAPPING vector to return a 1−symbol
			// via a byte equivalent to 1 of the 16 mapped const ellation
			// values.
			unsigned char decode_chips(const unsigned int &chips);

			// slice will be used to determine if an input bit is
			// 1 or 0 for the shift register
			int slice(const float &x);

			// setter & getter for error threshold
			void set_threshold(const unsigned int &threshold);
			unsigned int get_threshold() const;

			// gettets and setters to get/set sample rate
			void set_samp_rate(const float &samp_rate);
			float get_samp_rate() const;

			// getters and setters for buffer size
			void set_buffer_size(const unsigned int &buffer);
			unsigned int get_buffer_size() const;

			// getters and setters for burst trigger
			void set_burst_trigger(const float &burst_trigger);
			float get_burst_trigger() const;

			// getters and setters to get/set debug settings
			void set_debug(const DebugType &debug);
			DebugType get_debug() const;

			// getters and setters to  get/set crc
			// verified packets only or everything
			void set_crc(const CRC &crc);
			CRC get_crc() const;

			// getters and setters for stat_file functions
			void set_save_stats_switch(const SAVE &save_stats_switch);
			void set_save_stats_file(const std::string &save_stats_file);
			SAVE get_save_stats_switch() const;
			std::string get_save_stats_file() const;

			// function used in mac.cc from gr−ieee805−15−4
			// to verify data validity of raw data
			uint16_t crc16(const char *buf, const int &len);

			// shortcut to clear string stream
			void clear_stream();

			// generic print print function that for vector
			// inputs
			void vector_print(const std::string &label, const std::vector<int> &input);

			// returns a trigger−set /sample adjusted preamble from the passed in buffer
			std::tuple<dna_buffer, bool> normalize_preamble(const dna_buffer &buffer,
					const int &max_return_size, const float &burst_trigger);

			// NOTE: Didn't finish, but will if I have time...
			// Filter_burst is going to filter the absolute magnitude of the burst with an n−taps.
			// The magnitude pulse scaled and normalized to 1 and the index point where the filtered
			// burst exceeds the user−given threshold
			std::tuple<dna_buffer, int> filter_burst(const dna_buffer &buffer,
					const int &filter_taps);

		};

	} // namespace ieee802_15_4
} // namespace gr

#endif /* INCLUDED_IEEE802_15_4_PREAMBLE_SINK_IMPL_H */

