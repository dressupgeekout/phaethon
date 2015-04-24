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

/** @file
 *  DDS (DirectDraw Surface) loading.
 */

#ifndef IMAGES_DDS_H
#define IMAGES_DDS_H

#include <vector>

#include "images/decoder.h"

namespace Common {
	class SeekableReadStream;
}

namespace Images {

/** DirectDraw Surface.
 *
 *  Both standard DDS files and BioWare's own version are supported.
 */
class DDS : public Decoder {
public:
	DDS(Common::SeekableReadStream &dds);
	~DDS();

private:
	/** The specific pixel format of the included image data. */
	struct DDSPixelFormat {
		uint32 size;     ///< The size of the image data in bytes.
		uint32 flags;    ///< Features of the image data.
		uint32 fourCC;   ///< The FourCC to detect the format by.
		uint32 bitCount; ///< Number of bits per pixel.
		uint32 rBitMask; ///< Bit mask for the red color component.
		uint32 gBitMask; ///< Bit mask for the green color component.
		uint32 bBitMask; ///< Bit mask for the blue color component.
		uint32 aBitMask; ///< Bit mask for the alpha component.
	};

	// Loading helpers
	void load(Common::SeekableReadStream &dds);
	void readHeader(Common::SeekableReadStream &dds);
	void readStandardHeader(Common::SeekableReadStream &dds);
	void readBioWareHeader(Common::SeekableReadStream &dds);
	void readData(Common::SeekableReadStream &dds);

	void detectFormat(const DDSPixelFormat &format);

	void setSize(MipMap &mipMap);
};

} // End of namespace Images

#endif // IMAGES_DDS_H
