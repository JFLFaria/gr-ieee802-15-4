/*******************************************************************************************************************************
 ***
 *dna_bufferClassDefinition

 ********************************************************************************************************************************/
/*!
 *
 Class:dna_buffer
 *
 Author:Cruz,Frankie
 *
 Description:Extended vector template to buffer input data with markers
 *
 *
 *
 Generated:TuesNov2012:10:182018
 Modified:Dec1312:00:372018
 **
 togiveauserabeginning/endofabufferandkeeptrack
 ofdnapreambles
 **BuildVersion:1.5
 *Changes:-bufferwillnowkeepanarrayofsamplelengthstopopoff
 *
 *

 thesamplesonceabufferthresholdhasbeenexceeded
 -fixedinputerrorwitherase_buffer_element
 */

#ifndef INCLUDED_IEEE802_15_4_DNA_BUFFER_H
#define INCLUDED_IEEE802_15_4_DNA_BUFFER_H

#include <ieee802_15_4/api.h>
#include <gnuradio/block.h>

class dna_buffer {
public:
	/*!
	 *constructorsanddestructors
	 */
	dna_buffer();
	dna_buffer(const int &space); //constructorw/blankspaceintervals
	~dna_buffer();

	/*!
	 *copyconstructorsandassignmentoperators
	 */
	dna_buffer(const dna_buffer &copy);
	dna_buffer& operator=(const dna_buffer &copy);
	dna_buffer& operator=(const std::vector<gr_complex> &copy);

	/*!
	 *moveconstructorsandassignmentoperators
	 */
	dna_buffer(dna_buffer &&moved);
	dna_buffer& operator=(dna_buffer &&moved);
	dna_buffer& operator=(std::vector<gr_complex> &&moved);

	/*!
	 *setters
	 */
	void set_buffer();
	void set_spacing(const int &space); //setsspacingsize
	void unset_buffer(); //deactivatesthedna_bufferandclearsoutvalues
	void erase_buffer_element(const int &element); //wrapper for vector erase function
	void erase_buffer_range(const int &front, const int &back); //wrapper for vector erase function
	void insert_rear_spacing(const int &space); //inserts a given zero buffer at the end of the buffer
	void insert_front_spacing(const int &space); //insert a given zero buffer at the front of the buffer
	void append_burst_sample_size(const int &size); //insert the size of the sample stored
	void pop_front_burst_samples(); //remove#ofsamplesfromthebufferandremovetheelementthesamplesize

	/*!
	 *getters
	 */
	bool is_activated() const; //returns true if the buffer is set
	int get_space() const; //returnsthespacingdefault
	int get_sample_length() const; //retunshowlongthebufferisinsamples
	int get_burst_length() const; //returnshowmanyburstsarestoredinthebuffer
	gr_complex operator[](const int &index) const; //getaccesstoindividualdatainbuffer
	std::vector<gr_complex> get_stored_buffer() const; //returnsacopyofthestoredbuffer

	/*!
	 *append_preamble_points will be used in the case where more data has to be
	 *appended i.e. input buffer is large enough to collect more than one
	 *transmission
	 */
	void append_buffer_data(const dna_buffer &append);
	void append_buffer_data(const std::vector<gr_complex> &append);
	void append_buffer_data(const gr_complex &element);

	/*!
	 *store_preamblewilltaketheinputsizeandpointerto
	 *avectorandstoreitintothebufferspace
	 */
	void store_preamble_data(const int &ninput_size, const gr_complex *in_rx);

protected:
	std::vector<gr_complex> d_buffer;
	std::vector<int> d_sample_size_array; //keepstrackoffrontburstssamplesize
	bool d_dna_buffer_on; //keepstrackofbufferifit'son/off
	int d_space; //defaultpreamblespacing86
};

#endif /* INCLUDED_IEEE802_15_4_DNA_BUFFER_H */
