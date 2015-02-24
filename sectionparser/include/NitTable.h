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
