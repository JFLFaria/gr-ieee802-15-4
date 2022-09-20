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

#ifndef INCLUDED_IEEE802_15_4_PREAMBLE_SINK_H
#define INCLUDED_IEEE802_15_4_PREAMBLE_SINK_H

#include <ieee802_15_4/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace ieee802_15_4 {
  /* !
   * SAVE enable/disable saving file statistics
   * or everything else.
   */
  enum SAVE {
  	NO = 0, YES = 1
  };

  /* !
   * CRC will be used to determine if either crc verified data will be output
   * or everything else.
   */
  enum CRC {
  	ALL_DATA = 0, CRC_ONLY = 1
  };

  /* !
   * DebugType will be used to determine the level of debugging outputs that will be enabled/disabled
   */
  enum DebugType {
  	ALL_OFF = 0,
  	CHIP_ON = 1,
  	PACKET_ON = 2,
  	USRP_ON = 4,
  	CHIP_PACKET_ON = 3,
  	CHIP_USRP_ON = 5,
  	PACKET_USRP_ON = 6,
  	ALL_ON = 7
  };


  /* !
   * Class: preamble_sink
   * Author: Schmid, Thomas; Bloessl, Bastian; Cruz, Frankie
   * Description: the class is directly derived from Bastian Bloessl's
   * packet_sink class from his IEEE802.15.4 It is the same
   * code with a few minor modifications that allow the RX
   * 802.15.4 preamble to be sent once it has been detected
   * Generated: Thurs Nov 8 15:30:18 2018
   * Modified: Sat Feb 2 08:00:23 2018
   * Build Version: 1.16
   * Changes : −Fixed short by test atistic
   * −Cleaned code up a bit
   * −Fixed Buffer placement...
   */
    class IEEE802_15_4_API preamble_sink : virtual public gr::block
    {
     public:
    	typedef boost::shared_ptr<preamble_sink> sptr;

		/* !
		 * Make preambe_sink
		 *
		 * param samp_rate is the sample rate of the system
		 * param buffer_size is an array length consisting of previous bursts
		 * param threshold is the error threshold of the detected chips
		 * param debug is the debug output message setting
		 * param crc is the crc verified vs non−verified acceptance setting
		 * param burst_trigger will set the starting threshold of detection for normalized preamble
		 * param save wi enable/disable a file save of the packets
		 * param save_file is the loction of where the file will be saved
		 */
		  static sptr make(float samp_rate, unsigned int threshold, unsigned int buffer, float burst_trigger,
				  DebugType debug, CRC crc, SAVE save_stats_switch, std::string save_stats_file);


		virtual void set_threshold(const unsigned int &threshold) = 0;
		virtual void set_samp_rate(const float &samp_rate) = 0;
		virtual void set_buffer_size(const unsigned int &buffer_size) = 0;
		virtual void set_burst_trigger(const float &burst_trigger) = 0;
		virtual void set_debug(const DebugType &debug) = 0;
		virtual void set_crc(const CRC &crc) = 0;
		virtual void set_save_stats_switch(const SAVE &save_stats_switch) = 0;
		virtual void set_save_stats_file(const std::string &save_stats_file) = 0;

		virtual unsigned int get_threshold() const = 0;
		virtual float get_samp_rate() const = 0;
		virtual unsigned int get_buffer_size() const = 0;
		virtual float get_burst_trigger() const = 0;
		virtual DebugType get_debug() const = 0;
		virtual SAVE get_save_stats_switch() const = 0;
		virtual std::string get_save_stats_file() const = 0;
    };
  } // namespace ieee802_15_4
} // namespace gr

#endif /* INCLUDED_IEEE802_15_4_PREAMBLE_SINK_H */

