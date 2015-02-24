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


#ifndef TOTTABLE_H_
#define TOTTABLE_H_

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "SiTable.h"
#include "MpegDescriptor.h"

#include "DvbUtils.h"

/**
 * Time and Date Table (TDT)/Time Offset Table (TOT) class
 */
class TotTable: public SiTable
{
public:
    /**
     * Constructor
     *
     * @param id table identifier
     * @param extId table identifier extension
     * @param ver version number
     * @param cur current/next indicator
     */
    TotTable(uint8_t id, uint16_t extId, uint8_t ver, bool cur)
        : SiTable(id, extId, ver, cur),
          m_utcTime(0)
    {
    }

    /**
     * Add a descriptor
     *
     * @param descriptor the descriptor
     */
    void addDescriptor(const MpegDescriptor& descriptor)
    {
        m_descriptors.push_back(descriptor);
    }

    /**
     * Add descriptors
     *
     * @param descriptors the descriptors
     */
    void addDescriptors(const std::vector<MpegDescriptor>& descriptors)
    {
        m_descriptors.insert(m_descriptors.end(), descriptors.begin(), descriptors.end());
    }

    /**
     * Get descriptors
     *
     * @return vector of descriptors
     */
    std::vector<MpegDescriptor>& getDescriptors()
    {
        return m_descriptors;
    }

    /**
     * Remove descriptors
     */
    void removeDescriptors()
    {
        m_descriptors.clear();
    }

    /**
     * Get the UTC time as a binary coded decimal
     *
     * @return UTC time (BCD)
     */
    uint64_t getUtcTimeBcd() const
    {
        return m_utcTime;
    }

    /**
     * Get the time (UTC)
     *
     * @return UTC time
     */
    time_t getUtcTime() const
    {
        return MjdToDate(m_utcTime);
    }

    /**
     * Set the time (UTC)
     *
     * @param utcTime
     */
    void setUtcTime(uint64_t utcTime)
    {
        m_utcTime = utcTime;
    }

private:
    /**
     * UTC time
     */
    uint64_t m_utcTime;

    /**
     * Descriptors
     */
    std::vector<MpegDescriptor> m_descriptors;
};

#endif /* TOTTABLE_H_ */
