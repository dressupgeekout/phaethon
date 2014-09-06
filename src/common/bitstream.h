/* Phaethon - A FLOSS resource explorer for BioWare's Aurora engine games
 *
 * Phaethon is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * Phaethon is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * Phaethon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Phaethon. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file common/bitstream.h
 *  A bit stream.
 */

#ifndef COMMON_BITSTREAM_H
#define COMMON_BITSTREAM_H

#include "common/types.h"
#include "common/error.h"
#include "common/stream.h"

namespace Common {

/** A bit stream. */
class BitStream {
public:
	virtual ~BitStream() {
	}

	/** Return the stream position in bits. */
	virtual uint32 pos() const = 0;

	/** Return the stream size in bits. */
	virtual uint32 size() const = 0;

	/** Has the end of the stream been reached? */
	virtual bool eos() const = 0;

	/** Rewind the bit stream back to the start. */
	virtual void rewind() = 0;

	/** Skip the specified amount of bits. */
	virtual void skip(uint32 n) = 0;

	/** Read a bit from the bit stream. */
	virtual uint32 getBit() = 0;

	/** Read a multi-bit value from the bit stream. */
	virtual uint32 getBits(uint8 n) = 0;

	/** Add a bit to the value x, making it an n-bit value. */
	virtual void addBit(uint32 &x, uint32 n) = 0;

protected:
	BitStream() {
	}
};

/**
 * A template implementing a bit stream for different data memory layouts.
 *
 * Such a bit stream reads a valueBits-wide values from the data stream and
 * gives * access to their bits, one at a time.
 *
 * For example, a bit stream with the layout parameters 32, true, false
 * for valueBits, isLE and isMSB2LSB, reads 32bit little-endian values
 * from the data stream and hands out the bits in the order of LSB to MSB.
 */
template<int valueBits, bool isLE, bool isMSB2LSB>
class BitStreamImpl : public BitStream {
private:
	SeekableReadStream *_stream; ///< The input stream.
	bool _disposeAfterUse;       ///< Should we delete the stream on destruction?

	uint64 _value;   ///< Current value.
	uint8  _inValue; ///< Position within the current value.

	/** Read a data value. */
	inline uint64 readData() {
		if (isLE) {
			if (valueBits ==  8)
				return _stream->readByte();
			if (valueBits == 16)
				return _stream->readUint16LE();
			if (valueBits == 32)
				return _stream->readUint32LE();
			if (valueBits == 64)
				return _stream->readUint64LE();
		} else {
			if (valueBits ==  8)
				return _stream->readByte();
			if (valueBits == 16)
				return _stream->readUint16BE();
			if (valueBits == 32)
				return _stream->readUint32BE();
			if (valueBits == 64)
				return _stream->readUint64BE();
		}

		assert(false);
		return 0;
	}

	/** Read the next data value. */
	inline void readValue() {
		if ((size() - pos()) < valueBits)
			throw Exception("BitStream::readValue(): End of bit stream reached");

		_value = readData();
		if (_stream->err() || _stream->eos())
			throw Exception(kReadError);

		// If we're reading the bits MSB first, we need to shift the value to that position
		if (isMSB2LSB)
			_value <<= 64 - valueBits;
		}

public:
	/** Create a bit stream using this input data stream and optionally delete it on destruction. */
	BitStreamImpl(SeekableReadStream *stream, bool disposeAfterUse = false) :
		_stream(stream), _disposeAfterUse(disposeAfterUse), _value(0), _inValue(0) {

		if ((valueBits != 8) && (valueBits != 16) && (valueBits != 32) && (valueBits != 64))
			throw Exception("BitStream: Invalid memory layout %d, %d, %d", valueBits, isLE, isMSB2LSB);
	}

	/** Create a bit stream using this input data stream. */
	BitStreamImpl(SeekableReadStream &stream) :
		_stream(&stream), _disposeAfterUse(false), _value(0), _inValue(0) {

		if ((valueBits != 8) && (valueBits != 16) && (valueBits != 32) && (valueBits != 64))
			throw Exception("BitStream: Invalid memory layout %d, %d, %d", valueBits, isLE, isMSB2LSB);
	}

	~BitStreamImpl() {
		if (_disposeAfterUse)
			delete _stream;
	}

	/** Read a bit from the bit stream. */
	uint32 getBit() {
		// Check if we need the next value
		if (_inValue == 0)
			readValue();

		// Get the current bit
		int b = 0;
		if (isMSB2LSB)
			b = ((_value & 0x8000000000000000ULL) == 0) ? 0 : 1;
		else
			b = ((_value & 1) == 0) ? 0 : 1;

		// Shift to the next bit
		if (isMSB2LSB)
			_value <<= 1;
		else
			_value >>= 1;

		// Increase the position within the current value
		_inValue = (_inValue + 1) % valueBits;

		return b;
	}

	/** Read a multi-bit value from the bit stream. */
	uint32 getBits(uint8 n) {
		if (n > 32)
			throw Exception("Too many bits requested to be read");

		// Read the number of bits
		uint32 v = 0;

		if (isMSB2LSB) {
			while (n-- > 0)
				v = (v << 1) | getBit();
		} else {
			for (uint32 i = 0; i < n; i++)
				v = (v >> 1) | (((uint32) getBit()) << 31);

			v >>= (32 - n);
		}

		return v;
	}

	/** Add a bit to the value x, making it an n-bit value. */
	void addBit(uint32 &x, uint32 n) {
		if (isMSB2LSB)
			x = (x << 1) | getBit();
		else
			x = (x & ~(1 << n)) | (getBit() << n);
	}

	/** Rewind the bit stream back to the start. */
	void rewind() {
		_stream->seek(0);

		_value   = 0;
		_inValue = 0;
	}

	/** Skip the specified amount of bits. */
	void skip(uint32 n) {
		while (n-- > 0)
			getBit();
	}

	/** Return the stream position in bits. */
	uint32 pos() const {
		if (_stream->pos() == 0)
			return 0;

		uint32 p = (_inValue == 0) ? _stream->pos() : ((_stream->pos() - 1) & ~((uint32) ((valueBits >> 3) - 1)));
		return p * 8 + _inValue;
	}

	/** Return the stream size in bits. */
	uint32 size() const {
		return (_stream->size() & ~((uint32) ((valueBits >> 3) - 1))) * 8;
	}

	bool eos() const {
		return _stream->eos() || (pos() >= size());
	}
};

// typedefs for various memory layouts.

/** 8-bit data, MSB to LSB. */
typedef BitStreamImpl<8, false, true > BitStream8MSB;
/** 8-bit data, LSB to MSB. */
typedef BitStreamImpl<8, false, false> BitStream8LSB;

/** 16-bit little-endian data, MSB to LSB. */
typedef BitStreamImpl<16, true , true > BitStream16LEMSB;
/** 16-bit little-endian data, LSB to MSB. */
typedef BitStreamImpl<16, true , false> BitStream16LELSB;
/** 16-bit big-endian data, MSB to LSB. */
typedef BitStreamImpl<16, false, true > BitStream16BEMSB;
/** 16-bit big-endian data, LSB to MSB. */
typedef BitStreamImpl<16, false, false> BitStream16BELSB;

/** 32-bit little-endian data, MSB to LSB. */
typedef BitStreamImpl<32, true , true > BitStream32LEMSB;
/** 32-bit little-endian data, LSB to MSB. */
typedef BitStreamImpl<32, true , false> BitStream32LELSB;
/** 32-bit big-endian data, MSB to LSB. */
typedef BitStreamImpl<32, false, true > BitStream32BEMSB;
/** 32-bit big-endian data, LSB to MSB. */
typedef BitStreamImpl<32, false, false> BitStream32BELSB;

/** 64-bit little-endian data, MSB to LSB. */
typedef BitStreamImpl<64, true , true > BitStream64LEMSB;
/** 64-bit little-endian data, LSB to MSB. */
typedef BitStreamImpl<64, true , false> BitStream64LELSB;
/** 64-bit big-endian data, MSB to LSB. */
typedef BitStreamImpl<64, false, true > BitStream64BEMSB;
/** 64-bit big-endian data, LSB to MSB. */
typedef BitStreamImpl<64, false, false> BitStream64BELSB;

} // End of namespace Common

#endif // COMMON_BITSTREAM_H