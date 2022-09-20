#include "file_read_impl.h"
#include "preamble_sink_impl.h"

/*******************************************************************************************************************************
 ***

 *file_read_implSourceCode
 **********************************************************************************************************************************/
file_read_impl::file_read_impl(const size_t &itemsize, const char *filename,
		const bool &repeat)

:
		d_itemsize(itemsize), d_fp(0), d_new_fp(0), d_repeat(repeat), d_updated(
				false), d_file_begin(true),

		d_repeat_cnt(0), d_add_begin_tag(pmt::PMT_NIL) {
	open(filename, repeat);
	do_update();
	d_filename.append(filename);

	std::stringstream str;
//str<<name()<<unique_id();
	str << "AFITFileRead!!!";
	_id = pmt::string_to_symbol(str.str());

}

file_read_impl::~file_read_impl() {
	if (d_fp)
		fclose((FILE*) d_fp);
	if (d_new_fp)
		fclose((FILE*) d_new_fp);

}

bool file_read_impl::seek(long seek_point, int whence) {
	return fseek((FILE*) d_fp, seek_point * d_itemsize, whence) == 0;

}

void file_read_impl::open(const char *filename, bool repeat) {
//obtainexclusiveaccessfordurationofthisfunction
	gr::thread::scoped_lock lock(fp_mutex);

	int fd;

//weuse"open"tousetotheO_LARGEFILEflag
	if ((fd = ::open(filename, O_RDONLY | OUR_O_LARGEFILE | OUR_O_BINARY))
			< 0) {
		perror(filename);
		throw std::runtime_error("can'topenfile");
	}

	if (d_new_fp) {
		fclose(d_new_fp);
		d_new_fp = 0;
	}

	if ((d_new_fp = fdopen(fd, "rb")) == NULL) {
		perror(filename);
//don'tleakfiledescriptoriffdopenfails
		::close(fd);
		throw std::runtime_error("can'topenfile");
	}
//Checktoensurethefilewillbeconsumedaccordingtoitemsize
	fseek(d_new_fp, 0, SEEK_END);
	int file_size = ftell(d_new_fp);
	rewind(d_new_fp);

//Warntheuserifpartofthefilewillnotbeconsumed.

	if (file_size % d_itemsize) {
		fprintf(stderr, "WARNING:File will not be fully consumed with the current output type");
		// lflb - patch - d_logger field is a protected field of gr::block class; cannot be used here
		//GR_LOG_WARN(d_logger,
		//		"WARNING:File will not be fully consumed with the current output type");
	}

	d_updated = true;

	d_repeat = repeat;

}

void file_read_impl::close() {
//obtainexclusiveaccessfordurationofthisfunction
	gr::thread::scoped_lock lock(fp_mutex);

	if (d_new_fp != NULL) {
		fclose(d_new_fp);
		d_new_fp = NULL;
	}
	d_updated = true;
}

void file_read_impl::do_update() {
	if (d_updated) {
		gr::thread::scoped_lock lock(fp_mutex); //holdwhileinscope

		if (d_fp)

			fclose(d_fp);

		d_fp = d_new_fp;
		d_new_fp = 0;
		d_updated = false;
		d_file_begin = true;
	}

//installnewfilepoint er
}

void file_read_impl::set_begin_tag(const pmt::pmt_t &val) {
	d_add_begin_tag = val;

}

int file_read_impl::read_data(const int &array_size, void *read_data)

{
	char *o = (char*) read_data;
	int i;
	int size = array_size;

	do_update();
	if (d_fp == NULL)

//updated_fpisreqd
		throw std::runtime_error("workwithfilenotopen");

	gr::thread::scoped_lock lock(fp_mutex); //holdfortherestofthisfunction

	while (size) {
		i = fread(o, d_itemsize, size, (FILE*) d_fp);

		size -= i;
		o += i * d_itemsize;

		if (size == 0)
			break;
//done

		if (i > 0)

//shortread,tryagain
			continue;

//Wegotazerofromfread.
//anyevent,ifwe'reinrepeatmode,seekbacktothebeginning
//InThisiseitherEOForerror.
//ofthefileandtryagain,elsebreak
		if (!d_repeat)
			break;

		if (fseek((FILE*) d_fp, 0, SEEK_SET) == -1) {

			fprintf(stderr, "[%s]fseekfailed\n", __FILE__);
			exit(-1);
		}

		if (d_add_begin_tag != pmt::PMT_NIL) {
			d_file_begin = true;
			d_repeat_cnt++;
		}
	}

	if (size > 0) {
		if (size == array_size)
//EOForerror
//wedidn'treadanything;saywe'redone
			return -1;

		return array_size - size;
	}
//elsereturn partialresult

	return array_size;
}

std::string file_read_impl::get_filename() const {

	return d_filename;
}

size_t file_read_impl::get_size() const {

	return d_itemsize;
}
