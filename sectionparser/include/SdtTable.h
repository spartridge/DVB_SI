// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

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
