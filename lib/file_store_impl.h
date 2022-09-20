/*******************************************************************************************************************************
 ***
 *file_store_baseclass Definition

 ********************************************************************************************************************************/
/*!
 *
 class :file_store_base
 *
 Author:GNURadio;Cruz,Frankie
 *
 Description:Basedoffoffile_sink_base
 *
 *
 Generated:ThursDec0611:00:002018
 Modified:ThursDec0710:00:002018
 **
 hardcodedduetotimeconst raint s
 **BuildVersion:1.1
 *Changes:-addedastring andgetterfunction
 *forthefilename
 **/

#ifndef INCLUDED_IEEE802_15_4_FILE_STORE_IMPL_H
#define INCLUDED_IEEE802_15_4_FILE_STORE_IMPL_H

#include <ieee802_15_4/api.h>
#include <gnuradio/block.h>
#include <stdio.h>


class file_store_base {

protected:
	FILE *d_fp; //currentFILEpoint er
	FILE *d_new_fp; //newFILEpoint er
	bool d_updated;
	bool d_is_binary;
//isthereanewFILEpoint er?
	boost::mutex d_mutex;
	bool d_unbuffered;
	bool d_append;
	std::string d_filename; //c_string offilename

protected:
	file_store_base(const char *filename, const bool &is_binary,
			const bool &append);

public:
	file_store_base() {
	}
	~file_store_base();

	/*!
	 *\briefOpenfilenameandbeginoutputtoit.
	 */
	bool open(const char *filename);
	/*!
	 *\briefClosecurrentoutputfile.
	 *
	 *Closescurrentoutputfileandignoresanyoutputuntil
	 *openiscalledtoconnecttoanotherfile.
	 */
	void close();

	/*!
	 *\briefifwe'vehadanupdate,doitnow.
	 */
	void do_update();

	/*!
	 *\briefturnonunbufferedwritesforsloweroutputs
	 */
	void set_unbuffered(const bool &unbuffered);

	/*!
	 *optiontoappenddatatofile
	 */
	void set_append(const bool &append);

	std::string get_filename() const; //return sstoredfilename

};

/*******************************************************************************************************************************
 ***
 *file_store_implclass Definition

 ********************************************************************************************************************************/
/*!
 *
 class :file_store_impl
 *
 Author:GNURadio;Cruz,Frankie
 *
 Description:Basedoffoffile_sink_impl
 *
 *
 *
 Generated:ThursDec0611:00:002018
 Modified:ThursDec0611:00:002018

 hardcodedduetotimeconst raint s.
 Essentiallysave'sbinarydata
 **BuildVersion:1.0
 *Changes:-Baseline
 */

class file_store_impl: public file_store_base {
private:

	size_t d_itemsize;
public:
//file_read_implwilltaketheitem_sizeusingthesizeof()functionofanint ,float ,gr_complex,
//orchar.
	file_store_impl(const size_t &itemsize, const char *filename,

	const bool &binary_data = true, const bool &append = false);
//file_store_impl(){}91
	~file_store_impl();

	int store_data(const int &array_size, void *input_items);

	size_t get_size() const; //return sstoredfilesize

};

#endif /* INCLUDED_IEEE802_15_4_FILE_STORE_IMPL_H */
