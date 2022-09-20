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

#ifndef INCLUDED_IEEE802_15_4_DNA_DETECTOR_CCF_IMPL_H
#define INCLUDED_IEEE802_15_4_DNA_DETECTOR_CCF_IMPL_H

#include <ieee802_15_4/dna_detector_ccf.h>
#include "preamble_sink_impl.h" //included to add class types stored i.e. dna_bufferpackets
#include "file_read_impl.h"
//Boost matrix libraries
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/operations.hpp>
#include <boost/numeric/ublas/io.hpp>

//Boost Stats Libraries
#include <boost/math/distributions.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/skewness.hpp>
#include <boost/accumulators/statistics/kurtosis.hpp>

//Eigen Matrix libraries
#include <eigen3/Eigen/Dense>

//std libraries
#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>
#include <cmath>
#include <iomanip>
#include <ctime>


namespace gr {
  namespace ieee802_15_4 {

  //short hand syntax simplifier for calling Eignen complex matricies and vectors
  template<class T> using EigMat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
  //EigRow is a Nx1 matrix short hand
  template<class T> using EigColVec = Eigen::Matrix<T, Eigen::Dynamic, 1>;
  //EigColVec is a Nx1 vector short hand
  template<class T> using EigRowVec = Eigen::Matrix<T, 1, Eigen::Dynamic>;
  //EigRowVec is a 1xM vector short hand

  //short hand syntax for calling boost specific matricies and vectors
  template<class V> using BoostMat = boost::numeric::ublas::matrix<V>;//specifiy NxM at generation
  template<class V> using BoostVec = boost::numeric::ublas::vector<V>;//specify vector length at generation

  //using the accumulator namespace and dna_marker
  //alias provide some flexibility on the data being analyzed
  //and shortening the ammount of code to type....
  using namespace boost::accumulators;
  //shortening boost call
  template<class X> using dna_marker = boost::accumulators::accumulator_set<X, features<tag::count, tag::mean(immediate), tag::variance(immediate), tag::skewness, tag::kurtosis>>;

  class dna_detector_ccf_impl : public dna_detector_ccf
    {
     private:
		enum {
			SEARCHING, DETECTED, FINGERPRINT
		} d_state; //keep track of what state we're in

		int d_devices = 5;
		int d_usrp_debug = 0;
	//number of devices/fingerprints input for analysis
		int d_dna_row_size;
		int d_dna_column_size;
		int d_dna_region_num = 10;
		float d_samp_rate = 4e6;
	//default samplerate
		std::stringstream d_msg_stream;
	//designed to handle messages string messages
		std::stringstream d_emitter_stats_stream; //designedtosaveresultsinfoforeachemmittersresults
		std::string d_fingerprint_filename;
		std::string d_fingerprint_stats_filename; //filenametosavestats
		std::string d_matrix_mult_results_filename; //filenametosave
	//USRP verboseoutputfordebugging
	//dnafingerprintsrowdimension
	//dnafingerprintscolumndimension
	//#offingerprintregions

	//filenametosavefingerprints
	//normalizationvectorsfilename
		std::string d_norm_vector_filename;
		std::string d_xoffset_vector_filename;
		SWITCH d_fingerprint_save_switch = SWITCH::OFF;
		std::string d_class_matrix_filename;
		std::string d_means_matrix_filename;
	//offsetvectorsfilename
	//optiontosavethefingerprintsgeneratedperrun
	//classificationmatrixfilename
	//meansmatrixfilename
		SWITCH d_class_matrix_load_switch = SWITCH::OFF; //optiontoloadaclassifierfile
		SWITCH d_save_stats_switch = SWITCH::OFF;
		SWITCH d_save_dna_results_switch = SWITCH::OFF;
	//optiontosavethegeneratedstatistics
	//optiontosavednamultiplicationresults

	//FileRead&WriteModifiers
		std::shared_ptr<file_read_impl> d_xoffset_vector_loader; //loadescalculatedfingerprintoffsets
		std::shared_ptr<file_read_impl> d_norm_vector_loader; //loadscalculatednormalizationscalingvector
		std::shared_ptr<file_read_impl> d_class_means_loader; //loadscalculatedmeansforeachdevicegeneratedbyMATLAB
		std::shared_ptr<file_read_impl> d_dna_loader; //loadsclassificationmatrixgeneratedbyMATLAB
		std::shared_ptr<file_store_impl> d_dna_fingerprint_write; //ford_dna_fingerprints
		std::shared_ptr<file_store_impl> d_stats_writer; //forstatistisoutputfile
		std::shared_ptr<file_store_impl> d_matrix_results_writer; //storesresultsofmultiplicationmatrix
		std::shared_ptr<file_store_impl> d_dna_results_writer; //storesresultsofdnamultiplication

		DebugType d_debug=ALL_OFF;
		unsigned long int d_burst_count = 0;
	//debugtype
		unsigned long int d_fingerprints_counter = 0; //fingerprintgenerationcounter
		unsigned long int d_nan_counter = 0;
	//burstcounter(4,294,967,295sizelimit)
	//keepstrackoffingerprintsthatwerenotsent

		ieee802_15_4_packet d_phy_802_15_4;
	//containsalltheclasssizingfunctions

		std::vector<unsigned long int> d_emitter_bin_counter;
		std::vector<gr_complex> d_current_detected_preamble;
		std::vector<float> d_dna_results;
		std::vector<float> d_dna_fingerprints;
		EigMat<float> d_dna_matrix;
	//arraykeepingtrackofwhatdeviceswereidentified
	//calculatedfingerprintsfromdetectedpreamble
	//classificationmatrixgeneratedfrommatlab
		EigMat<float> d_class_means_matrix; //deviceclassificationmeansgeneratedfrommatlab
		EigRowVec<float> d_norm_vector;
		EigRowVec<float> d_xoffset_vector; //offsetvectortoadjustcollectedfingerprints
	//normalizationvectortoscaleincommingfingerprintby


     public:
      dna_detector_ccf_impl(int devices, int dna_row_size, int dna_column_size, int dna_region_num, DebugType debug, float samp_rate, std::string fingerprint_filename, SWITCH fingerprint_save_switch, std::string class_matrix_filename, std::string means_matrix_filename, std::string norm_vector_filename, std::string xoffset_vector_filename, SWITCH class_matrix_load_switch, SWITCH save_stats_switch, SWITCH save_dna_results_switch);
      ~dna_detector_ccf_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

      //statefunctionstoprocessdata
      	void burst_search();
      	void burst_detect();
      	void burst_fingerprint();

      //publiclyavailablesetters
      //setthe#ofdevices
      	void set_devices(const int &devices);
      	void set_dna_row_size(const int &dna_row_size); //settherowsizeofdnamatrix,ieNofNxM
      	void set_dna_column_size(const int &dna_column_size); //setthecolumnsizeofthednamatrix,ieMofNxM
      //setthenumberofregionsusedinfingerprinting
      	void set_dna_region_num(const int &dna_region_num);
      	void set_debug(const DebugType &debug);
      	void set_dna_fingerprint_filename(const std::string &fingerprint_filename);
      	void set_dna_fingerprint_save_switch(const SWITCH &fingerprint_save_switch);
      	void set_dna_classification_matrix_filename(
      			const std::string &class_matrix_filename);
      	void set_dna_means_matrix_filename(const std::string &mean_matrix_filename);
      	void set_dna_classification_load_switch(
      			const SWITCH &class_matrix_load_switch);
      	void set_save_stats_switch(const SWITCH &save_stats_switch);
      	void set_save_dna_results_switch(const SWITCH &save_dna_results_switch);
      	void set_samp_rate(const float &samp_rate);
      	void set_dna_norm_vector_filename(const std::string &norm_vector_filename);
      	void set_dna_xoffset_vector_filename(
      			const std::string &xoffset_vector_filename);
      	void clear_stream(); //shorthandwaytocleard_msg_streamviastackoverflow.com

      //publiclyavailablegetters
      	int get_devices() const;
      	int get_dna_row_size() const;
      	int get_dna_column_size() const;
      	int get_dna_region_num() const;
      	DebugType get_debug() const;
      	std::string get_dna_fingerprint_filename() const;
      	SWITCH get_dna_fingerprint_save_switch() const;
      	std::string get_dna_classification_matrix_filename() const;
      	std::string get_dna_means_matrix_filename() const;
      	SWITCH get_dna_classification_load_switch() const;
      	SWITCH get_save_stats_switch() const;
      	SWITCH get_save_dna_results_switch() const;
      	float get_samp_rate() const;
      	std::string get_dna_norm_vector_filename() const;
      	std::string get_dna_xoffset_vector_filename() const;

      //WorkFunctions
      	float constrain_angle(const float &angle);//putsanglefrom−pitopifromstackoverflow.com
      	float angle_conv(const float &angle); //convertsanglefrom−2pito2pifromstackoverflow.com
      	float angle_diff(const float &angle_a, const float &angle_b); //ensuresawaytohandlepi==−pifromstackoverflow.com
      	float unwrap_phase(const float &prev_angle, const float &current_angle); //unwrapfunctionfromstackoverflow.com
      	std::vector<float> calculate_fingerprints(
      			const std::vector<gr_complex> &burst); //theinputburstwillreturntheestdevice

      //make_subregionwillreturnasubretionconsistingoftheamplitude,phse,andfrequencyoftheprovidedsamples
      //likewise,theamplitude,phase,andfrequencysub−sectionswillconsistof3sampleseachofthecalculated
      //varriance,skewness,andkurtosis
      	std::vector<float> make_subregion(
      			const std::vector<gr_complex> &sub_region);

      //fingerprint_emmiterwilltakeinthecalculatedfingerprintsstatisticsand
      //willmultiplythevectorwiththeloadedfingerprintdata
      	std::vector<float> fingerprint_emitter(
      			const std::vector<float> &fingerprint);
    };

  } // namespace ieee802_15_4
} // namespace gr

#endif /* INCLUDED_IEEE802_15_4_DNA_DETECTOR_CCF_IMPL_H */

