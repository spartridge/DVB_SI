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

#include "MpegDescriptor.h"

using std::vector;

/**
 * Constructor
 *
 * @param tag descriptor tag
 * @param data descriptor data
 * @param length data length
 */
MpegDescriptor::MpegDescriptor(DescriptorTag tag, uint8_t* data, uint8_t length): m_tag(tag)
{
    m_data.assign(data, data+length);
}

/**
 * Parse descriptors
 *
 * @param p pointer to the raw data
 * @param length length of the raw data
 * @return vector of descriptors
 */
vector<MpegDescriptor> MpegDescriptor::parseDescriptors(uint8_t* p, uint16_t length)
{
    vector<MpegDescriptor> ret;

    // Sanity check
    if(p)
    {
        uint8_t *end = p + length;

        // A valid descriptor should have at least two bytes (tag + length)
        while(p + 2 <= end)
        {
            DescriptorTag tag = static_cast<DescriptorTag>(p[0]);
            uint8_t len = p[1];

            // Boundary check
            if((len + 2) <= (end - p))
            {
                // Let's now add the descriptor (tag + data) to the vector
                ret.emplace_back(tag, p + 2, len);
            }

            p += len + 2;
        }
    }

    return ret;
}

/**
 * Find a descriptor by tag
 *
 * @param list descriptors
 * @param tag descriptor tag to search for
 * @return descriptor descriptor if found, null otherwise
 */
const MpegDescriptor* MpegDescriptor::find(const vector<MpegDescriptor>& list, DescriptorTag tag)
{
    const MpegDescriptor *ret = 0;

    // Let's find a descriptor by given tag in the given vector
    for(auto it = list.begin(), end = list.end(); it != end; ++it)
    {
        // found?
        if(it->getTag() == tag)
        {
            ret = &(*it);
            break;
        }
    }

    return ret;
}

/**
 * Find descriptors by tag
 * @param list descriptors
 * @param tag descriptor tag to search for
 * @return vector of descriptors if found, null otherwise
 */
vector<MpegDescriptor> MpegDescriptor::findAll(const vector<MpegDescriptor>& list, DescriptorTag tag)
{
    vector<MpegDescriptor> ret;

    // Let's find all descriptors with a given tag in the given vector
    for(auto it = list.begin(), end = list.end(); it != end; ++it)
    {
        if(it->getTag() == tag)
        {
            ret.push_back(*it);
        }
    }

    return ret;
}
