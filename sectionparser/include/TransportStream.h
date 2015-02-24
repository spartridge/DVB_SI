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
