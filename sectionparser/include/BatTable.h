//
// DVB_SI for Reference Design Kit (RDK)
//
// Copyright (C) 2015  Dmitry Barablin & Stan Partridge  (stan.partridge@arris.com)
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

#ifndef BATTABLE_H_
#define BATTABLE_H_

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "SiTable.h"
#include "MpegDescriptor.h"
#include "TransportStream.h"

/**
 * Bouquet Association Table (BAT) class
 */
class BatTable: public SiTable
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
    BatTable(uint8_t id, uint16_t extId, uint8_t ver, bool cur)
        : SiTable(id, extId, ver, cur)
    {
    }

    /**
     * Get bouquet identifier
     *
     * @return buquet identifier
     */
    uint16_t getBouquetId() const
    {
        return getExtensionId();
    }

    /**
     * Add a bouquet descriptor
     *
     * @param descriptor bouquet descriptor
     */
    void addBouquetDescriptor(const MpegDescriptor& descriptor)
    {
        m_bouquetDescriptors.push_back(descriptor);
    }

    /**
     * Add bouquet descriptors
     *
     * @param descriptors bouquet descriptors
     */
    void addBouquetDescriptors(const std::vector<MpegDescriptor>& descriptors)
    {
        m_bouquetDescriptors.insert(m_bouquetDescriptors.end(), descriptors.begin(), descriptors.end());
    }

    /**
     * Get bouquet descriptors
     *
     * @return vector of bouquet descriptors
     */
    const std::vector<MpegDescriptor>& getBouquetDescriptors() const
    {
        return m_bouquetDescriptors;
    }

    /**
     * Remove bouquet descriptors
     */
    void removeBouquetDescriptors()
    {
        m_bouquetDescriptors.clear();
    }

    /**
     * Add a transport stream
     *
     * @param ts transport stream
     */
    void addTransportStream(const TransportStream& ts)
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
     * Bouquet descriptors
     */
    std::vector<MpegDescriptor> m_bouquetDescriptors;

    /**
     * List of transport streams
     */
    std::vector<TransportStream> m_tsList;
};

#endif /* BATTABLE_H_ */
