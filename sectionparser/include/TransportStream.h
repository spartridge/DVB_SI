// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef TRANSPORTSTREAM_H_
#define TRANSPORTSTREAM_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <vector>

/**
 * Transport stream class
 */
class TransportStream
{
public:
    /**
     * Constructor
     *
     * @param tsId transport stream identifier
     * @param origNetId original network identifier
     */
    TransportStream(uint16_t tsId, uint16_t origNetId)
        : m_tsId(tsId),
          m_originalNetworkId(origNetId)
    {
    }

    /**
     * Destructor
     */
    virtual ~TransportStream()
    {
    }

    /**
     * Get the original network identifier
     *
     * @return original network id
     */
    uint16_t getOriginalNetworkId() const
    {
        return m_originalNetworkId;
    }

    /**
     * Set the original network identifier
     *
     * @param originalNetworkId original network id
     */
    void setOriginalNetworkId(uint16_t originalNetworkId)
    {
        m_originalNetworkId = originalNetworkId;
    }

    /**
     * Get the transport stream identifier
     *
     * @return transport stream id
     */
    uint16_t getTsId() const
    {
        return m_tsId;
    }

    /**
     * Set the transport stream identifier
     *
     * @param tsId transport stream identifier
     */
    void setTsId(uint16_t tsId)
    {
        m_tsId = tsId;
    }

    /**
     * Get transport stream descriptors
     *
     * @return vector of ts descriptors
     */
    const std::vector<MpegDescriptor>& getTsDescriptors() const
    {
        return m_tsDescriptors;
    }

    /**
     * Add a descriptor
     *
     * @param descriptor the descriptor
     */
    void addDescriptor(MpegDescriptor& descriptor)
    {
        m_tsDescriptors.push_back(descriptor);
    }

    /**
     * Add descriptors
     *
     * @param descriptors the descriptors
     */
    void addDescriptors(const std::vector<MpegDescriptor>& descriptors)
    {
        m_tsDescriptors.insert(m_tsDescriptors.end(), descriptors.begin(), descriptors.end());
    }

    /**
     * Remove descriptors
     */
    void removeDescriptors()
    {
        m_tsDescriptors.clear();
    }

private:
    /**
     * Transport stream identifier
     */
    uint16_t m_tsId;

    /**
     * Original network identifier
     */
    uint16_t m_originalNetworkId;

    /**
     * Transport stream descriptors
     */
    std::vector<MpegDescriptor> m_tsDescriptors;
};

#endif /* TRANSPORTSTREAM_H_ */
