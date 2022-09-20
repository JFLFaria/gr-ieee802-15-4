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

#ifndef INCLUDED_IEEE802_15_4_DNA_DETECTOR_CCF_H
#define INCLUDED_IEEE802_15_4_DNA_DETECTOR_CCF_H

#include <ieee802_15_4/api.h>
#include <gnuradio/block.h>
#include <string>
#include "preamble_sink.h"

namespace gr {
  namespace ieee802_15_4 {
	  /*!
	   *SWITCHisasimpleon/offenumoption
	   */
	  enum SWITCH {
		OFF = 0, ON = 1
	  };

	  /*!
	   * Class:dna_detector
	   * Author:Cruz,Frankie
	   * Description:theclassiscalculateswhichemmiterisbeingreceived
	   * byperformingrf−dnaanalysisandoutputtingtheresults
	   * toahistogramviamsgpassingandreducestheammount
	   * ofoutputdataburstspassedinbyreducingthe"dead"
	   * spaceinbetween
	   * Generated:MonDec317:30:122018
	   * Modified:SatFeb29:00:282018
	   * BuildVersion:1.11
	   * Changes:-Cleanedcodeupabit
	   */

    class IEEE802_15_4_API dna_detector_ccf : virtual public gr::block
    {
     public:
		typedef boost::shared_ptr<dna_detector_ccf> sptr;

		/*!
		 *paramfingerprintsisthenumberofemmitersthatwillbeanalyzed
		 *paramdna_row_sizeistherowsizeoftheimportedrfdnafingerprints
		 *paramdna_column_sizeisthecolumnsizeoftheimportedrfdnafingerprints
		 *paramdna_region_numisthenumberofregionsusedforrfdnafingerprints
		 *paramsamp_rateisthesamplerateofthesystem
		 *paramfingerprint_filenameisthefilenametheuserpickstostorefingerprints
		 *paramfingerprint_save_switchistheon/offtostorefingerprints
		 *paramclass_matrix_filenameisthetrainedclassificationfiletoloadintoamatrix
		 *parammeans_matrix_filenameisthetraineddevicemeansfiletoloadintoamatrix
		 *paramclass_matrix_load_switchistheon/offtoloadtheDNAfile
		 *paramnorm_vector_filenameisthescalingmultiplierforfingerprintsgeneratedbytraineddata
		 *paramxoffset_vector_filenameisanoffsetthathastobeappliedtothegeneratedfingerprints
		 *paramsave_stats_switchistheon/offtostorerunningstatsinfo(note:writecanbebehind)
		 *paramsave_dna_resultsistheon/offtostoretheresultofthefingerprintandDNAmatrix
		 */
		static sptr make(int devices, int dna_row_size, int dna_column_size, int dna_region_num, DebugType debug, float samp_rate, std::string fingerprint_filename, SWITCH fingerprint_save_switch, std::string class_matrix_filename, std::string means_matrix_filename, std::string norm_vector_filename, std::string xoffset_vector_filename, SWITCH class_matrix_load_switch, SWITCH save_stats_switch, SWITCH save_dna_results_switch);


		virtual void set_devices(const int &devices)=0;
		virtual void set_dna_row_size(const int &row_size)=0;
		virtual void set_dna_column_size(const int &col_size)=0;
		virtual void set_dna_region_num(const int &region_num)=0;
		virtual void set_debug(const DebugType &debug)=0;
		virtual void set_dna_fingerprint_filename(
				const std::string &fingerprint_filename)=0;
		virtual void set_dna_fingerprint_save_switch(
				const SWITCH &fingerprint_save_switch)=0;
		virtual void set_dna_classification_matrix_filename(
				const std::string &class_matrix_filename)=0;
		virtual void set_dna_means_matrix_filename(
				const std::string &means_matrix_filename)=0;
		virtual void set_dna_classification_load_switch(
				const SWITCH &class_matrix_load_switch)=0;
		virtual void set_save_stats_switch(const SWITCH &save_stats_switch)=0;
		virtual void set_save_dna_results_switch(
				const SWITCH &save_dna_results_switch)=0;
		virtual void set_samp_rate(const float &samp_rate)=0;
		virtual void set_dna_norm_vector_filename(
				const std::string &norm_vector_filename)=0;
		virtual void set_dna_xoffset_vector_filename(
				const std::string &xoffset_vector_filename)=0;

		virtual int get_devices() const =0;
		virtual int get_dna_row_size() const =0;
		virtual int get_dna_column_size() const =0;
		virtual int get_dna_region_num() const =0;
		virtual DebugType get_debug() const =0;
		virtual std::string get_dna_fingerprint_filename() const =0;
		virtual SWITCH get_dna_fingerprint_save_switch() const =0;
		virtual std::string get_dna_classification_matrix_filename() const =0;
		virtual std::string get_dna_means_matrix_filename() const =0;
		virtual SWITCH get_dna_classification_load_switch() const =0;
		virtual SWITCH get_save_stats_switch() const =0;
		virtual SWITCH get_save_dna_results_switch() const =0;
		virtual float get_samp_rate() const =0;
		virtual std::string get_dna_norm_vector_filename() const =0;
		virtual std::string get_dna_xoffset_vector_filename() const =0;

    };

  } // namespace ieee802_15_4
} // namespace gr

#endif /* INCLUDED_IEEE802_15_4_DNA_DETECTOR_CCF_H */

