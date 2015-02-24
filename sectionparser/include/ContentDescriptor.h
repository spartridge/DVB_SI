//
// DVB_SI for Reference Design Kit (RDK)
//
// Copyright (C) 2015  Dmitry Barablin & Stan Partridge (stan.partridge@arris.com)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef CONTENTDESCRIPTOR_H_
#define CONTENTDESCRIPTOR_H_

// Other libraries' includes

// Project's includes
#include "MpegDescriptor.h"

// Syntax of the descriptor from ETSI EN 300 468
//
//content_descriptor(){
//    descriptor_tag                 8 uimsbf
//    descriptor_length              8 uimsbf
//    for (i=0;i<N;i++) {
//        content_nibble_level_1     4 uimsbf
//        content_nibble_level_2     4 uimsbf
//        user_byte                  8 uimsbf
//    }
//}

/**
 * Content descriptor class
 */
class ContentDescriptor: public MpegDescriptor
{
public:
    /**
     * Constructor
     *
     * @param desc mpeg descriptor
     */
    ContentDescriptor(const MpegDescriptor& desc)
        : MpegDescriptor(desc)
    {
    }

    /**
     * Get the number of content identifiers (count)
     *
     * @return count
     */
    uint8_t getCount()
    {
        return m_data.size()/2;
    }

    /**
     * Get the first level of content identifier
     *
     * @param n
     * @return content_nibble_level_1
     */
    uint8_t getNibbleLvl1(uint8_t n) const
    {
        return m_data[n*2] >> 4;
    }

    /**
     * Get the second level of content identifier
     *
     * @param n
     * @return content_nibble_level_2
     */
    uint8_t getNibbleLvl2(uint8_t n) const
    {
        return m_data[n*2] & 0xf;
    }

    /**
     * Get the user byte
     *
     * @param n
     * @return user_byte
     */
    uint8_t getUserByte(uint8_t n) const
    {
        return m_data[n*2 + 1];
    }

};

#endif /* CONTENTDESCRIPTOR_H_ */
