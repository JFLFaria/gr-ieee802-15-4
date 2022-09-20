/*******************************************************************************************************************************
 ***
 *packet_baseclass Definition

 **********************************************************************************************************************************/
/*!
 *Class :packet_base
 *Author:Cruz,Frankie
 *Description:Baseclass int erfaceforminimalist
 *PHYPacketsizecalculationbased
 *ononpreamblesection,1length,
 *section,and1payloadsection.
 *Eachstandardbreaksthesethree
 **Generated:Tues
 *Modified:Dec0113:30:012018
 downfurtherint osmallersubsets.
 Nov2022:00:182018
 *BuildVersion:1.3
 *Changes:-fixedget_preamble_len_samples
 */

#ifndef INCLUDED_IEEE802_15_4_PACKET_BASE_H
#define INCLUDED_IEEE802_15_4_PACKET_BASE_H

#include <ieee802_15_4/api.h>
#include <gnuradio/block.h>
#include <math.h>

class packet_base {
public:
	packet_base() = default;
	~packet_base() = default;

	/*!
	 *Settersint erfaceforpreamble,length,
	 *anddatapacketfieldsizes
	 */
	virtual void set_preamble_len_bytes(const int &preamble_len) {
		d_preamble_len = preamble_len;
	}
	;
	virtual void set_length_len_bytes(const int &length_len) {
		d_length_len = length_len;
	}
	;
	virtual void set_data_len_bytes(const int &data_len) {
		d_data_len = data_len;
	}
	;

	/*!
	 *Gettersint erfaceforpreamble,length,
	 *anddatapacketfieldsizes
	 */
	virtual int get_preamble_len_bytes() const {
		return d_preamble_len;
	}
	;
	virtual int get_length_len_bytes() const {
		return d_length_len;
	}
	;
	virtual int get_data_len_bytes() const {
		return d_data_len;
	}
	;
	virtual int get_packet_len_bytes() const {
		return (d_preamble_len + d_length_len + d_data_len);
	}
	;
	virtual int get_preamble_len_chips() const {
		return round(d_byte2sym * d_sym2chip * d_preamble_len);
	}
	;
	virtual int get_length_len_chips() const {
		return round(d_byte2sym * d_sym2chip * d_length_len);
	}
	;
	virtual int get_data_len_chips() const {
		return round(d_byte2sym * d_sym2chip * d_data_len);
	}
	;
	virtual int get_packet_len_chips() const {
		return round(d_byte2sym * d_sym2chip * get_packet_len_bytes());
	}
	;
	virtual int get_preamble_len_samples() const {
		return round(d_chip_period * d_sample_rate * get_preamble_len_chips());
	}
	;
	virtual int get_length_len_samples() const {
		return round(d_chip_period * d_sample_rate * get_length_len_chips());
	}
	;
	virtual int get_data_len_samples() const {
		return round(d_chip_period * d_sample_rate * get_data_len_chips());
	}
	;
	virtual int get_packet_len_samples() const {
		return round(d_chip_period * d_sample_rate * get_packet_len_chips());
	}
	;

	/*!
	 *Settersint erfaceforratesandperiods
	 */
	virtual void set_chip_period(const float &chip_period);
	virtual void set_symbol_period(const float &symbol_period);
	virtual void set_sample_period(const float &sample_period);
	virtual void set_chip_rate(const float &chip_rate);
	virtual void set_symbol_rate(const float &symbol_rate);
	virtual void set_sample_rate(const float &sample_rate);

	/*!*Settersint erfaceforconversionmultipliers
	 */
	virtual void set_byte_to_sym_mult(const float &byte2sym);
	virtual void set_sym_to_byte_mult(const float &sym2byte);
	virtual void set_sym_to_chip_mult(const float &sym2chip);
	virtual void set_chip_to_sym_mult(const float &chip2sym);

	/*!
	 *Gettersint erfaceforratesandperiods
	 */
	virtual float get_chip_period() const {
		return d_chip_period;
	}
	;
	virtual float get_symbol_period() const {
		return d_symbol_period;
	}
	;
	virtual float get_sample_period() const {
		return d_sample_period;
	}
	;
	virtual float get_chip_rate() const {
		return 1 / d_chip_period;
	}
	;
	virtual float get_symbol_rate() const {
		return 1 / d_symbol_period;
	}
	;
	virtual float get_sample_rate() const {
		return 1 / d_sample_period;
	}
	;

	/*!
	 *Gettersint erfaceforconversionmultipliers
	 */
	virtual float get_byte_to_sym_mult() const {
		return d_byte2sym;
	}
	;
	virtual float get_sym_to_byte_mult() const {
		return d_sym2byte;
	}
	;
	virtual float get_sym_to_chip_mult() const {
		return d_sym2chip;
	}
	;
	virtual float get_chip_to_sym_mult() const {
		return d_chip2sym;
	}
	;
	virtual float get_bit_to_byte_mult() const {
		return d_bit2byte;
	}
	virtual float get_byte_to_bit_mult() const {
		return 1 / d_bit2byte;
	}

private:
	int d_preamble_len = 0; //preambleframelengthinbytes
	int d_length_len = 0;
	int d_data_len = 0; //datapacketframelengthinbytes
	float d_bit2byte = 0.125f; //1-bitis1/8byte(byte/bit)
	float d_chip_period = 0.0f; //(seconds/chip)
	float d_symbol_period = 0.0f; //(seconds/symbol)
	float d_chip_rate = 0.0f; //(chip/sec)
	float d_symbol_rate = 0.0f; //(sym/sec)
	float d_byte2sym = 0.0f; //1-byte=2-symbols(sym/byte)
//datalengthfieldparameterinbytes
	float d_sym2byte = 0.0f; //1-symbol=1/2byte(byte/sym)
	float d_sym2chip = 0.0f; //1-symbol=32-chips(chips/sym)
	float d_chip2sym = 0.0f; //1-chip=1/32-symbols(sym/chip)
	float d_sample_period = 0.0f; //(sec/sample)

	float d_sample_rate = 0.0f; //(sample/sec)

};

/*******************************************************************************************************************************
 ***
 *ieee802_15_4_packetclass Definition

 **********************************************************************************************************************************/
/*!

 *class :ieee802_15_4_packet
 *Author:Cruz,Frankie
 *Description:IEEE802-15-4class for
 *
 PHYPacketsizecalculation.112*Generated:TuesNov212:00:182018
 *Modified:WedNov2810:30:112018
 *BuildVersion:1.2
 *Changes:-Removedsomeinitializingsetter
 *-Needtoimproveandfixfunctionality
 */

class ieee802_15_4_packet: public packet_base {

public:

	/*!
	 *const ructor'sandDestructor
	 */
	ieee802_15_4_packet();
	ieee802_15_4_packet(const float &samp_rate);
	ieee802_15_4_packet(const int &preamble_len, const int &length_len,
			const int &data_len,

			const float &samp_rate);
	~ieee802_15_4_packet() = default;

	/*!
	 *Copyconst ructor&Assignment
	 */
	ieee802_15_4_packet(const ieee802_15_4_packet &copy);
	ieee802_15_4_packet& operator=(const ieee802_15_4_packet &copy);

	/*!
	 *Gettersforbytes,chips,andsamplesfor
	 *ieee802-15-4packetsections
	 */
	int get_shr_preamble_len_byte() const;
	int get_shr_sfd_len_bytes() const;
	int get_phr_len_bytes() const;
	int get_payload_len_bytes() const;
	int get_crc_len_bytes() const;
	int get_shr_preamble_len_chips() const;
	int get_shr_sfd_len_chips() const;
	int get_phr_len_chips() const;
	int get_payload_len_chips() const;
	int get_crc_len_chips() const;
	int get_shr_preamble_len_samples() const;
	int get_shr_sfd_len_samples() const;
	int get_phr_len_samples() const;
	int get_payload_len_samples() const;
	int get_crc_len_samples() const;

private:
	const int d_shr_preamble_len = 4; //syncpreamble(4-0x00or32-bits)
	const int d_shr_sfd_len = 1; //startofframedelimiter(1-0x7Aor8-bits)
	const int d_phr_len = 1; //phrheaderdenotingpacketlength
	const int d_payload_len = 125; //payloadlength(max125-bytesor1000-bits)
	const int d_crc_len = 2; //crctocheckpacketvalidity

};

#endif /* INCLUDED_IEEE802_15_4_PACKET_BASE_H */

