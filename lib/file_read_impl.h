/*******************************************************************************************************************************
 ***
 *file_read_implclass Definition

 **********************************************************************************************************************************/
/*!
 *

 class :file_read_impl6*Author:GNURadio;Cruz,Frankie
 *Description:Basedoffoffile_source_impl
 ***Generated:ThursDec0611:00:002018
 Modified:ThursDec0710:00:002018
 hardcodedduetotimeconst raint s.
 Essentiallyloadbinarydata
 **BuildVersion:1.1
 *Changes:-addedastringstore&getterfunction
 *-addedgetterforitemsizeforthefilename
 ***/

#ifndef INCLUDED_IEEE802_15_4_FILE_READ_IMPL_H
#define INCLUDED_IEEE802_15_4_FILE_READ_IMPL_H

#include <ieee802_15_4/api.h>
#include <gnuradio/block.h>
#include <stdio.h>


class file_read_impl {

private:
	size_t d_itemsize;
	FILE *d_fp;
	FILE *d_new_fp;
	bool d_repeat;
	bool d_updated;
	bool d_file_begin;
	long d_repeat_cnt;
	pmt::pmt_t d_add_begin_tag;
	std::string d_filename; //storeageforfilename

	boost::mutex fp_mutex;
	pmt::pmt_t _id;

	void do_update();

public:
	file_read_impl(const size_t &itemsize, const char *filename,
			const bool &repeat = false);
	~file_read_impl();

	bool seek(long seek_point, int whence);
	void open(const char *filename, bool repeat);
	void close();

	int read_data(const int &array_size, void *read_data);

	void set_begin_tag(const pmt::pmt_t &val);
	std::string get_filename() const;
	size_t get_size() const;

};

#endif /*INCLUDED_IEEE802_15_4_FILE_READ_IMPL_H */
