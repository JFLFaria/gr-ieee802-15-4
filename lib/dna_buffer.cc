#include "dna_buffer.h"
/*******************************************************************************************************************************
 ***
 *dna_bufferSourceCode

 **********************************************************************************************************************************/
dna_buffer::dna_buffer() {

	d_dna_buffer_on = false;
	d_space = 0;

}

dna_buffer::dna_buffer(const int &space) {
	d_dna_buffer_on = false;

	set_spacing(space);
}

dna_buffer::~dna_buffer() {

}

dna_buffer::dna_buffer(const dna_buffer &copy) {
	d_dna_buffer_on = copy.d_dna_buffer_on;
	d_space = copy.d_space;
	d_buffer = copy.d_buffer;
	d_sample_size_array = copy.d_sample_size_array;

}

dna_buffer& dna_buffer::operator=(const dna_buffer &copy) {
	if (&copy != this) {
		d_dna_buffer_on = copy.d_dna_buffer_on;
		d_space = copy.d_space;
		d_buffer = copy.d_buffer;
		d_sample_size_array = copy.d_sample_size_array;
	}

	return *this;

}

dna_buffer& dna_buffer::operator=(const std::vector<gr_complex> &copy) {
	if (copy != d_buffer) {
		d_dna_buffer_on = true;
		d_buffer = copy;
	}
}

dna_buffer::dna_buffer(dna_buffer &&moved) {
	d_dna_buffer_on = std::move(moved.d_dna_buffer_on);
	d_space = std::move(moved.d_space);
	d_buffer = std::move(moved.d_buffer);
	d_sample_size_array = std::move(moved.d_sample_size_array);

}

dna_buffer& dna_buffer::operator=(dna_buffer &&moved) {
	if (this != &moved) {
		d_dna_buffer_on = std::move(moved.d_dna_buffer_on);
		d_space = std::move(moved.d_space);
		d_buffer = std::move(moved.d_buffer);
		d_sample_size_array = std::move(moved.d_sample_size_array);
	}

	return *this;

}

dna_buffer& dna_buffer::operator=(std::vector<gr_complex> &&moved) {
	set_buffer();
	d_buffer = std::move(moved);

}

void dna_buffer::set_buffer() {
	d_dna_buffer_on = true;

}

void dna_buffer::set_spacing(const int &space) {
	d_space = space < 0 ? 0 : space;

}

void dna_buffer::unset_buffer() {
	d_dna_buffer_on = false;

	d_buffer.clear();
	d_sample_size_array.clear();

}

void dna_buffer::erase_buffer_element(const int &element) {
	d_buffer.erase(d_buffer.begin() + element);

}

void dna_buffer::erase_buffer_range(const int &front, const int &back) {
	d_buffer.erase(d_buffer.begin() + front,

	d_buffer.begin() + back);

}

void dna_buffer::insert_rear_spacing(const int &space) {
	if (space > 0) {
		for (int c = 0; c < space; c++) {

			d_buffer.push_back(gr_complex(0.0f, 0.0f));
		}

	}

}

void dna_buffer::insert_front_spacing(const int &space) {
	if (space > 0) {
		for (int c = 0; c < space; c++) {

			d_buffer.insert(d_buffer.begin(), gr_complex(0.0f, 0.0f));
		}

	}

}

void dna_buffer::append_burst_sample_size(const int &size) {

	d_sample_size_array.push_back(size);
}

void dna_buffer::pop_front_burst_samples() {
//havetohaveachecktoseeifthere'sanything
//inthebufferorwecouldpossiblycrasheverything
//tryingtoremoveanon-existentarray
	if (!d_sample_size_array.empty()) {
//popofffront#samplesfromoldestburstinthebuffer
		d_buffer.erase(d_buffer.begin(),
				d_buffer.begin() + d_sample_size_array[0]);

//popofffrontofcontainerholding#ofsamplesofpreviousburst
		d_sample_size_array.erase(d_sample_size_array.begin());
	}
}

bool dna_buffer::is_activated() const {

	return d_dna_buffer_on;
}

int dna_buffer::get_space() const {

	return d_space;
}

int dna_buffer::get_sample_length() const {

	return d_buffer.size();
}

int dna_buffer::get_burst_length() const {

	return d_sample_size_array.size();
}

gr_complex dna_buffer::operator[](const int &index) const {
	return d_buffer[index];

}

std::vector<gr_complex> dna_buffer::get_stored_buffer() const {
//ifthevectorisempty

	return (d_buffer.empty()) ? std::vector<gr_complex>() : d_buffer;

}

void dna_buffer::append_buffer_data(const dna_buffer &append) {
//functionwrapperforparentclass

	append_buffer_data(append.d_buffer);
}

void dna_buffer::append_buffer_data(const std::vector<gr_complex> &append) {
//aftercheckingtoseeifthearraypassed
//isempty,thearraydatywillbeappended
//withtheuserdefinedlengthofemptydata
	if (!append.empty()) {
		for (int i = 0; i < d_space + append.size(); i++) {
			if (i < d_space) {
				d_buffer.push_back(gr_complex(0.0f, 0.0f));
			} else {
//d_buffer.push_back(append[i-d_space]);
				append_buffer_data(append[i - d_space]);
			}

		}

	}

}

void dna_buffer::append_buffer_data(const gr_complex &element) {
	d_dna_buffer_on = true; //bydefault,ifanydataisappended,thenthebuffermustbeset

	d_buffer.push_back(element);
}

void dna_buffer::store_preamble_data(const int &input_size,
		const gr_complex *in_rx)

		{

//initialchecktoensurethatthesize

//ofthereceiveddataisworthcopying
//beforeestimatingtheindexes
	if (input_size > 0) {
		std::vector < gr_complex > temp; //temporaryplaceholder
		set_buffer();

		for (int i = 0; i < input_size; i++) {
			temp.push_back(in_rx[i]);

		}

		d_buffer = std::move(temp);
	}

}
