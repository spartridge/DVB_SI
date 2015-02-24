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
