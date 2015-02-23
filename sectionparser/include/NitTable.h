// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef NITTABLE_H_
#define NITTABLE_H_

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "SiTable.h"
#include "MpegDescriptor.h"
#include "TransportStream.h"

/**
 * Network Information Table (NIT) class
 */
class NitTable: public SiTable
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
    NitTable(uint8_t id, uint16_t extId, uint8_t ver, bool cur)
        : SiTable(id, extId, ver, cur)
    {
    }

    /**
     * Get the network identifier
     *
     * @return network id
     */
    uint16_t getNetworkId() const
    {
        return getExtensionId();
    }

    /**
     * Add a network descriptor
     *
     * @param descriptor the network descriptor
     */
    void addNetworkDescriptor(MpegDescriptor& descriptor)
    {
        m_networkDescriptors.push_back(descriptor);
    }

    /**
     * Add network descriptors
     *
     * @param descriptors network descriptors
     */
    void addNetworkDescriptors(const std::vector<MpegDescriptor>& descriptors)
    {
        m_networkDescriptors.insert(m_networkDescriptors.end(), descriptors.begin(), descriptors.end());
    }

    /**
     * Get network descriptors
     *
     * @return vector of network descriptors
     */
    const std::vector<MpegDescriptor>& getNetworkDescriptors() const
    {
        return m_networkDescriptors;
    }

    /**
     * Remove network descriptors
     */
    void removeNetworkDescriptors()
    {
        m_networkDescriptors.clear();
    }

    /**
     * Add a transport stream
     *
     * @param ts the transport stream
     */
    void addTransportStream(TransportStream& ts)
    {
        m_tsList.push_back(ts);
    }

    /**
     * Get transport streams
     *
     * @return vector of transport streams
     */
    const std::vector<TransportStream>& getTransportStreams() const
    {
        return m_tsList;
    }

    /**
     * Remove transport streams
     */
    void removeTransportStreams()
    {
        m_tsList.clear();
    }

private:
    /**
     * Network descriptors
     */
    std::vector<MpegDescriptor> m_networkDescriptors;

    /**
     * List of transport streams
     */
    std::vector<TransportStream> m_tsList;
};

#endif /* NITTABLE_H_ */
