// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef EITTABLE_H_
#define EITTABLE_H_

// C++ system includes
#include <vector>

// Other libraries' includes

// Project's includes
#include "SiTable.h"
#include "DvbEvent.h"

/**
 * Event Information Table (EIT) class
 */
class EitTable: public SiTable
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
    EitTable(uint8_t id, uint16_t extId, uint8_t ver, bool cur)
        : SiTable(id, extId, ver, cur),
          m_tsId(0),
          m_networkId(0),
          m_lastTableId(0)
    {
    }

    /**
     * Add an event
     *
     * @param event the event
     */
    void addEvent(DvbEvent& event)
    {
        m_eventList.push_back(event);
    }

    /**
     * Get events
     *
     * @return vector of events
     */
    const std::vector<DvbEvent>& getEvents() const
    {
        return m_eventList;
    }

    /**
     * Remove events
     */
    void removeEvents()
    {
        m_eventList.clear();
    }

    /**
     * Get the last table identifier
     *
     * @return table id
     */
    uint8_t getLastTableId() const
    {
        return m_lastTableId;
    }

    /**
     * Set the last table identifier
     *
     * @param lastTableId the last table identifier
     */
    void setLastTableId(uint8_t lastTableId)
    {
        m_lastTableId = lastTableId;
    }

    /**
     * Get the network identifier
     *
     * @return network id
     */
    uint16_t getNetworkId() const
    {
        return m_networkId;
    }

    /**
     * Set the network identifier
     *
     * @param networkId the network identifier
     */
    void setNetworkId(uint16_t networkId)
    {
        m_networkId = networkId;
    }

    /**
     * Get the transport identifier
     *
     * @return ts id
     */
    uint16_t getTsId() const
    {
        return m_tsId;
    }

    /**
     * Set the transport identifier
     *
     * @param tsId
     */
    void setTsId(uint16_t tsId)
    {
        m_tsId = tsId;
    }

private:
    /**
     * Transport identifier
     */
    uint16_t m_tsId;

    /**
     * Network identifier
     */
    uint16_t m_networkId;

    /**
     * Last table identifier
     */
    uint8_t m_lastTableId;

    /**
     * List of events
     */
    std::vector<DvbEvent> m_eventList;
};

#endif /* EITTABLE_H_ */
