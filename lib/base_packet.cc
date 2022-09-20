#include "base_packet.h"
/*******************************************************************************************************************************
 ***
 *packet_baseSourceCode

 **********************************************************************************************************************************/
void packet_base::set_chip_period(const float &chip_period) {

	d_chip_period = chip_period;

	if (d_chip_rate != (1 / chip_period)) {
		d_chip_rate = 1 / chip_period;
	}

}

void packet_base::set_symbol_period(const float &symbol_period) {
	d_symbol_period = symbol_period;

	if (d_symbol_rate != (1 / symbol_period)) {
		d_symbol_rate = 1 / d_symbol_period;
	}

}

void packet_base::set_sample_period(const float &sample_period) {
	d_sample_period = sample_period;

	if (d_sample_rate != (1 / sample_period)) {
		d_sample_rate = 1 / sample_period;
	}

}

void packet_base::set_chip_rate(const float &chip_rate) {
	d_chip_rate = chip_rate;

	if (d_chip_period != (1 / chip_rate)) {
		d_chip_period = 1 / chip_rate;
	}

}

void packet_base::set_symbol_rate(const float &symbol_rate) {
	d_symbol_rate = symbol_rate;

	if (d_symbol_period != (1 / symbol_rate)) {
		d_symbol_period = 1 / symbol_rate;
	}

}

void packet_base::set_sample_rate(const float &sample_rate) {
	d_sample_rate = sample_rate;

	if (d_sample_period != (1 / sample_rate)) {
		d_sample_period = 1 / sample_rate;
	}

}

void packet_base::set_byte_to_sym_mult(const float &byte2sym) {
	d_byte2sym = byte2sym;

	if (d_sym2byte != (1 / byte2sym)) {
		d_sym2byte = 1 / byte2sym;
	}

}

void packet_base::set_sym_to_byte_mult(const float &sym2byte) {
	d_sym2byte = sym2byte;

	if (d_byte2sym != (1 / sym2byte)) {
		d_byte2sym = 1 / sym2byte;
	}

}

void packet_base::set_sym_to_chip_mult(const float &sym2chip) {
	d_sym2chip = sym2chip;

	if (d_chip2sym != (1 / sym2chip)) {
		d_chip2sym = 1 / sym2chip;
	}

}

void packet_base::set_chip_to_sym_mult(const float &chip2sym) {
	d_chip2sym = chip2sym;

	if (d_sym2chip != (1 / chip2sym)) {
		d_sym2chip = 1 / chip2sym;
	}

}

/*******************************************************************************************************************************
 ***
 *ieee802_15_4_packetSourceCode

 **********************************************************************************************************************************/
ieee802_15_4_packet::ieee802_15_4_packet() {

	set_preamble_len_bytes(d_shr_preamble_len + d_shr_sfd_len);

	set_length_len_bytes(d_phr_len);
	set_data_len_bytes(d_payload_len + d_crc_len);
	set_sample_rate(4e6f);
	set_chip_rate(2e6f);
	set_symbol_rate(1e6f);
	set_byte_to_sym_mult(2.0f);
	set_sym_to_chip_mult(32.0f);

}

ieee802_15_4_packet::ieee802_15_4_packet(const float &samp_rate) {
	set_preamble_len_bytes(d_shr_preamble_len + d_shr_sfd_len);

	set_length_len_bytes(d_phr_len);
	set_data_len_bytes(d_payload_len + d_crc_len);
	set_sample_rate(samp_rate);
	set_chip_rate(2e6f);
	set_symbol_rate(1e6f);
	set_byte_to_sym_mult(2.0f);
	set_sym_to_chip_mult(32.0f);

}

ieee802_15_4_packet::ieee802_15_4_packet(const int &preamble_len,
		const int &length_len, const int &data_len,

		const float &samp_rate) {

	set_preamble_len_bytes(preamble_len);
	set_length_len_bytes(length_len);
	set_data_len_bytes(data_len);
	set_sample_rate(samp_rate);
	set_chip_rate(2e6f);
	set_symbol_rate(1e6f);
	set_byte_to_sym_mult(2.0f);
	set_sym_to_chip_mult(32.0f);

}

ieee802_15_4_packet::ieee802_15_4_packet(const ieee802_15_4_packet &copy) {

	set_preamble_len_bytes(copy.get_preamble_len_bytes());
	set_length_len_bytes(copy.get_length_len_bytes());
	set_data_len_bytes(copy.get_data_len_bytes());
	set_sample_period(copy.get_sample_period());
	set_sample_rate(copy.get_sample_rate());
	set_chip_rate(copy.get_chip_rate());
	set_chip_period(copy.get_chip_period());
	set_symbol_period(copy.get_symbol_period());
	set_symbol_rate(copy.get_symbol_rate());
	set_byte_to_sym_mult(copy.get_byte_to_sym_mult());
	set_sym_to_byte_mult(copy.get_sym_to_byte_mult());
	set_sym_to_chip_mult(copy.get_sym_to_chip_mult());

	set_chip_to_sym_mult(copy.get_chip_to_sym_mult());
}

ieee802_15_4_packet& ieee802_15_4_packet::operator=(
		const ieee802_15_4_packet &copy) {
//selfassignmentcheck

	if (&copy != this) {
		set_preamble_len_bytes(copy.get_preamble_len_bytes());
		set_length_len_bytes(copy.get_length_len_bytes());
		set_data_len_bytes(copy.get_data_len_bytes());
		set_sample_period(copy.get_sample_period());
		set_sample_rate(copy.get_sample_rate());
		set_chip_rate(copy.get_chip_rate());
		set_chip_period(copy.get_chip_period());
		set_symbol_period(copy.get_symbol_period());
		set_symbol_rate(copy.get_symbol_rate());
		set_byte_to_sym_mult(copy.get_byte_to_sym_mult());
		set_sym_to_byte_mult(copy.get_sym_to_byte_mult());
		set_sym_to_chip_mult(copy.get_sym_to_chip_mult());
		set_chip_to_sym_mult(copy.get_chip_to_sym_mult());
	}

	return *this;

}

int ieee802_15_4_packet::get_shr_preamble_len_byte() const {

	return d_shr_preamble_len;
}

int ieee802_15_4_packet::get_shr_sfd_len_bytes() const {

	return d_shr_sfd_len;
}

int ieee802_15_4_packet::get_phr_len_bytes() const {

	return d_phr_len;
}

int ieee802_15_4_packet::get_payload_len_bytes() const {

	return d_payload_len;
}

int ieee802_15_4_packet::get_crc_len_bytes() const {

	return d_crc_len;
}

int ieee802_15_4_packet::get_shr_preamble_len_chips() const {
	return round(d_shr_preamble_len *

	get_byte_to_sym_mult() * get_sym_to_chip_mult());

}

int ieee802_15_4_packet::get_shr_sfd_len_chips() const {
	return round(d_shr_sfd_len *

	get_byte_to_sym_mult() * get_sym_to_chip_mult());

}

int ieee802_15_4_packet::get_phr_len_chips() const {
	return round(d_phr_len *

	get_byte_to_sym_mult() * get_sym_to_chip_mult());

}

int ieee802_15_4_packet::get_payload_len_chips() const {
	return round(d_payload_len *

	get_byte_to_sym_mult() * get_sym_to_chip_mult());

}

int ieee802_15_4_packet::get_crc_len_chips() const {

	return round(d_crc_len * get_byte_to_sym_mult() * get_sym_to_chip_mult());
}

int ieee802_15_4_packet::get_shr_preamble_len_samples() const {
	return round(get_shr_preamble_len_chips() *

	get_chip_period() * get_sample_rate());

}

int ieee802_15_4_packet::get_shr_sfd_len_samples() const {
	return round(get_shr_sfd_len_chips() *

	get_chip_period() * get_sample_rate());

}

int ieee802_15_4_packet::get_phr_len_samples() const {
	return round(get_phr_len_chips() *

	get_chip_period() * get_sample_rate());

}

int ieee802_15_4_packet::get_payload_len_samples() const {
	return round(get_payload_len_chips() *

	get_chip_period() * get_sample_rate());

}

int ieee802_15_4_packet::get_crc_len_samples() const {
	return round(get_crc_len_chips() *

	get_chip_period() * get_sample_rate());

}
