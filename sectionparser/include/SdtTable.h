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


#ifndef SDTTABLE_H_
#define SDTTABLE_H_

// C system includes
#include <stdint.h>

// Other libraries' includes

// Project's includes
#include "DvbService.h"
#include "SiTable.h"

/**
 * Service Description Table (SDT) class
 */
class SdtTable: public SiTable
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
    SdtTable(uint8_t id, uint16_t extId, uint8_t ver, bool cur)
        : SiTable(id, extId, ver, cur), m_originalNetworkId(0)
    {
    }

    /**
     * Add a service
     *
     * @param service the service
     */
    void addService(const DvbService& service)
    {
        m_serviceList.push_back(service);
    }

    /**
     * Get services
     *
     * @return vector of services
     */
    const std::vector<DvbService>& getServices() const
    {
        return m_serviceList;
    }

    /**
     * Remove services
     */
    void removeServices()
    {
        m_serviceList.clear();
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
     * @param originalNetworkId the original network identifier
     */
    void setOriginalNetworkId(uint16_t originalNetworkId)
    {
        m_originalNetworkId = originalNetworkId;
    }

private:
    /**
     * List of servies
     */
    std::vector<DvbService> m_serviceList;

    /**
     * Original network identifier
     */
    uint16_t m_originalNetworkId;
};

#endif /* SDTTABLE_H_ */
