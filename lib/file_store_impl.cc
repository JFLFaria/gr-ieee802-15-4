#include "file_store_impl.h"
#include "preamble_sink_impl.h"

/*******************************************************************************************************************************
 ***
 *file_store_baseSourceCode

 **********************************************************************************************************************************/
file_store_base::file_store_base(const char *filename, const bool &is_binary,
		const bool &append)

:
		d_fp(0), d_new_fp(0), d_updated(false), d_is_binary(is_binary), d_append(
				append) {

	if

	(!open(filename))
		throw std::runtime_error("can'topenfile");
	d_filename.append(filename);
}

file_store_base::~file_store_base() {
	close();
	if (d_fp) {
		fclose(d_fp);
		d_fp = 0;
	}

}

bool file_store_base::open(const char *filename) {

	gr::thread::scoped_lock guard(d_mutex);
//holdmutexfordurationofthisfunction

//weusetheopensystemcalltogetaccesstotheO_LARGEFILEflag.
	int fd;
	int flags;
	if (d_append) {
		flags = O_WRONLY | O_CREAT | O_APPEND | OUR_O_LARGEFILE | OUR_O_BINARY;

	} else {
		flags = O_WRONLY | O_CREAT | O_TRUNC | OUR_O_LARGEFILE | OUR_O_BINARY;

	}
	if ((fd = ::open(filename, flags, 0664)) < 0) {

		perror(filename);

		return false;
	}
	if (d_new_fp) {

		d_new_fp = 0;

//ifwe'vealreadygotanewoneopen,closeit
		fclose(d_new_fp);
	}

	if ((d_new_fp = fdopen(fd, d_is_binary ? "wb" : "w")) == NULL) {
		perror(filename);
		::close(fd);
//don'tleakfiledescriptoriffdopenfails.
	}

	d_updated = true;

	return d_new_fp != 0;
}

void file_store_base::close() {

//holdmutexfordurationofthisfunction
	gr::thread::scoped_lock guard(d_mutex);

	if (d_new_fp) {

		fclose(d_new_fp);
		d_new_fp = 0;
	}
	d_updated = true;

}

void file_store_base::do_update() {
	if (d_updated) {

		gr::thread::scoped_lock guard(d_mutex);
		if (d_fp)

			fclose(d_fp);
		d_fp = d_new_fp;
		d_new_fp = 0;
//installnewfilepoint er
		d_updated = false;

	}

//holdmutexfordurationofthisblock
}

void file_store_base::set_unbuffered(const bool &unbuffered) {
	d_unbuffered = unbuffered;

}

void file_store_base::set_append(const bool &append) {
	d_append = append;

}

std::string file_store_base::get_filename() const {

	return d_filename;
}

/*******************************************************************************************************************************
 ***

 *file_store_implSourceCode195**********************************************************************************************************************************/
file_store_impl::file_store_impl(const size_t &itemsize, const char *filename,

const bool &binary_data, const bool &append) :
		file_store_base(filename, binary_data, append),

		d_itemsize(itemsize) {
}

file_store_impl::~file_store_impl() {
}

int file_store_impl::store_data(const int &array_size, void *input_items)

{
	char *inbuf = (char*) input_items;
	int nwritten = 0;

//updated_fpisreqd
	do_update();

	if (!d_fp)

		return array_size;
//dropoutputonthefloor

	while (nwritten < array_size) {

		int count = fwrite(inbuf, d_itemsize, array_size - nwritten, d_fp);
		if (count == 0) {
			if (ferror(d_fp)) {

				std::stringstream s;
				s << "file_sinkwritefailedwitherror" << fileno(d_fp)
						<< std::endl;

				throw std::runtime_error(s.str());
			} else { //isEOF

				break;
			}

		}

		nwritten += count;
		inbuf += count * d_itemsize;
	}

	if (d_unbuffered)
		fflush(d_fp);

	return nwritten;
}

size_t file_store_impl::get_size() const {

	return d_itemsize;
}
