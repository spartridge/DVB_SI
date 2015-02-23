// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

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
