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


#include "sectionlist.h"

// C++ system includes
#include <algorithm>
#include <sstream>

// Other libraries' includes

// Project's includes
#include "oswrap.h"

#include "NitTable.h"
#include "BatTable.h"
#include "SdtTable.h"
#include "EitTable.h"
#include "TotTable.h"

// Using declarations
using std::list;
using std::string;
using std::vector;

/**
 * Constructor
 *
 * @param data section data
 * @param len data length
 */
Section::Section(uint8_t* data, size_t len)
    : extensionId(0),
      version(0),
      current(true),
      number(0),
      lastNumber(0)
{
    // Sanity check
    if(!data || len < 3)
    {
        OS_LOG(DVB_ERROR, "<%s> Invalid section: data = %p, len = %d\n",
                __FUNCTION__, data, len);
        return;
    }

    // Table Identifier
    tableId = data[0];

    //Section syntax indicator
    syntax = data[1] & 0x80;

    // Section length
    length = ((uint16_t)(data[1] & 0xf)) << 8 | data[2];

    uint8_t offset = 3;

    // Let's check the syntax
    if(syntax)
    {
        // Table ID extension
        extensionId = (data[3] << 8) | data[4];

        // Version number
        version = (data[5] >> 1) & 0x1f;

        // Current/next indicator
        current = data[5] & 1;

        // Section number
        number = data[6];

        // Last section number
        lastNumber = data[7];
        offset = 8;
    }

    OS_LOG(DVB_TRACE3, "<%s> tableId = 0x%x, extId = 0x%x, length = %d, len = %d\n",
            __FUNCTION__, tableId, extensionId, length, len);

    // Let's read the payload
    if((offset < len) && ((length + 3) <= len))
    {
        payload.assign(data + offset, data + length + 3);
    }
    else
    {
        OS_LOG(DVB_ERROR, "<%s> tableId = 0x%x(data len = %d), section length(%d) <= payload offset(%d)\n",
                __FUNCTION__, tableId, len, length, offset);
    }
}

/**
 * Constructor
 */
SectionList::SectionList()
    : m_complete(false),
      m_firstReceivedNumber(0)
{
}

/**
 * Destructor
 */
SectionList::~SectionList()
{
}

/**
 * Initialize the section list
 *
 * @param section SI section
 */
void SectionList::init(Section& section)
{
    // Let's init the list
    m_sectionList.clear();
    m_sectionList.push_front(section);

    // Let's set the new section number
    m_firstReceivedNumber = section.number;

    // We need to handle EIT sections differently
    if(!isEit(section.tableId))
    {
        m_complete = complete(section);
    }
    else
    {
        // Don't check after receiving the first EIT section
        m_complete =  false;
    }
}

/**
 * Add a section to the section list
 *
 * @param section
 * @return true if the section was added successfully
 */
bool SectionList::add(Section& section)
{
    // empty?
    if(!m_sectionList.empty())
    {
        // Let's check if syntax indicator is on or not
        if(m_sectionList.front().syntax)
        {
            // Table ID extenstion should match
            if(m_sectionList.front().extensionId != section.extensionId)
            {
                OS_LOG(DVB_DEBUG, "<%s> 0x%x.0x%x: ext_id mismatch\n", __FUNCTION__, section.tableId, section.extensionId);
                // Ext id doesn't match
                return false;
            }

            // Let's check for version and/or last section number mismatch
            if((m_sectionList.front().version != section.version) ||
               (m_sectionList.front().lastNumber != section.lastNumber))
            {
                OS_LOG(DVB_DEBUG, "<%s> 0x%x.0x%x: version or last section number mismatch\n", __FUNCTION__, section.tableId, section.extensionId);
                init(section);
                return true;
            }

            // If we have an already complete version of the table, we can ignore the incoming section
            if(m_complete && (m_sectionList.front().version == section.version))
            {
                OS_LOG(DVB_DEBUG, "<%s> 0x%x.0x%x: ignoring (same version, table already complete)\n", __FUNCTION__, section.tableId, section.extensionId);
                // Ignore
                return false;
            }
        }
        else // syntax
        {
            OS_LOG(DVB_DEBUG, "<%s> 0x%x: syntax is false, calling Init()\n", __FUNCTION__, section.tableId);
            init(section);
            return true;
        }
    }
    else
    {
        OS_LOG(DVB_DEBUG, "<%s> 0x%x.0x%x: first section received\n", __FUNCTION__, section.tableId, section.extensionId);
        init(section);
        return true;
    }

    // Everything checks out. Time to insert the section into the list.
    insert(section);

    // Let's update the completeness flag
    m_complete = complete(section);

    return true;
}

/**
 * Insert section into the list
 *
 * @param section SI section
 */
void SectionList::insert(Section& section)
{
    auto it = m_sectionList.begin();
    auto end = m_sectionList.end();

    // Let's keep the section numbers sorted in the list
    while(it != end)
    {
        if(it->number == section.number)
        {
            // Do nothing
            return;
        }
        else if(it->number > section.number)
        {
            m_sectionList.insert(it, section);
            return;
        }

        ++it;
    }

    m_sectionList.push_back(section);
}

/**
 * Build an NIT table
 *
 * @return NitTable if successful, NULL otherwise
 */
NitTable* SectionList::buildNit()
{
    // Let's create the table object
    NitTable *nit = new NitTable(m_sectionList.front().tableId, m_sectionList.front().extensionId,
                                 m_sectionList.front().version, m_sectionList.front().current);

    // Time to parse the sections one by one
    for(auto it = m_sectionList.begin(), end = m_sectionList.end(); it != end; ++it)
    {
        // Network descriptors
        uint8_t *p = it->payload.data();
        if(!p)
        {
            continue;
        }

        uint16_t netDescLength = (((uint16_t)(p[0] & 0x0f) << 8) | p[1]);
        p += 2;

        nit->addNetworkDescriptors(MpegDescriptor::parseDescriptors(p, netDescLength));

        p += netDescLength;

        // Transport streams
        uint16_t tsLoopLength = (((uint16_t)(p[0] & 0x0f) << 8) | p[1]);

        p += 2;
        uint8_t *end = p + tsLoopLength;

        // ts_id(2 bytes) + orig_net_id(2 bytes) + ts_desc_len(2 bytes)
        while((p + 6) <= end)
        {
            // TS constructor takes ts_id(2 bytes) and orig_net_id(2 bytes) parameters
            TransportStream ts(((uint16_t)p[0] << 8) | p[1], ((uint16_t)p[2] << 8) | p[3]);

            uint16_t tsDescLength = ((uint16_t)(p[4] & 0x0f) << 8) | p[5];

            p += 6;
            ts.addDescriptors(MpegDescriptor::parseDescriptors(p, tsDescLength));
            nit->addTransportStream(ts);

            p += tsDescLength;
        }
    }

    return nit;
}

/**
 * Build a BAT table
 *
 * @return BatTable if successful, NULL otherwise
 */
BatTable* SectionList::buildBat()
{
    // Let's create the table object
    BatTable *bat = new BatTable(m_sectionList.front().tableId, m_sectionList.front().extensionId,
                                 m_sectionList.front().version, m_sectionList.front().current);

    // Time to parse the sections one by one
    for(auto it = m_sectionList.begin(), end = m_sectionList.end(); it != end; ++it)
    {
        // Bouquet descriptors
        uint8_t *p = it->payload.data();
        if(!p)
        {
            continue;
        }

        uint16_t bouquetDescLength = (((uint16_t)(p[0] & 0x0f) << 8) | p[1]);
        p += 2;

        bat->addBouquetDescriptors(MpegDescriptor::parseDescriptors(p, bouquetDescLength));

        p += bouquetDescLength;

        // Transport streams
        uint16_t tsLoopLength = (((uint16_t)(p[0] & 0x0f) << 8) | p[1]);

        p += 2;
        uint8_t *end = p + tsLoopLength;

        // ts_id(2 bytes) + orig_net_id(2 bytes) + ts_desc_len(2 bytes)
        while((p + 6) <= end)
        {
            // TS constructor takes ts_id(2 bytes) and orig_net_id(2 bytes) parameters
            TransportStream ts(((uint16_t)p[0] << 8) | p[1], ((uint16_t)p[2] << 8) | p[3]);

            uint16_t tsDescLength = ((uint16_t)(p[4] & 0x0f) << 8) | p[5];

            p += 6;
            ts.addDescriptors(MpegDescriptor::parseDescriptors(p, tsDescLength));
            bat->addTransportStream(ts);

            p += tsDescLength;
        }
    }

    return bat;
}

/**
 * Build an SDT table
 *
 * @return SdtTable if successful, NULL otherwise
 */
SdtTable* SectionList::buildSdt()
{
    // Let's create the table object
    SdtTable *sdt = new SdtTable(m_sectionList.front().tableId, m_sectionList.front().extensionId,
                                 m_sectionList.front().version, m_sectionList.front().current);

    sdt->setOriginalNetworkId(((uint16_t)(m_sectionList.front().payload[0]) << 8) | m_sectionList.front().payload[1]);

    OS_LOG(DVB_DEBUG, "<%s> SDT: orig_net_id = 0x%x, payload size = %d\n", __FUNCTION__, sdt->getOriginalNetworkId(), m_sectionList.front().payload.size());

    // Time to parse the sections one by one
    for(auto it = m_sectionList.begin(), end = m_sectionList.end(); it != end; ++it)
    {
        uint8_t *p = it->payload.data();
        if(!p)
        {
            // let's ignore "empty" sections
            continue;
        }

        uint8_t *payloadEnd = p + it->payload.size();

        // Skip original_network_id bytes and reserved byte
        p += 3;
        while((p + 4) < payloadEnd)
        {
            // Let's extract the data out of the section
            uint16_t serviceId    = ((uint16_t)(p[0]) << 8) | p[1];
            bool eitSchedule      = ((p[2] & 0x2) >> 1);
            bool eitPresent       =  ((p[2]) & 0x1);
            uint8_t runningStatus = (uint8_t)(p[3]) >> 5;
            bool isScrambled      = ((p[3] & 0x10) >> 4);
            uint16_t descLength   = ((uint16_t)(p[3] & 0xf) << 8) | p[4];

            // Let's now make a DvbService object using the extracted data
            DvbService service(serviceId, eitSchedule, eitPresent, runningStatus, isScrambled);
            OS_LOG(DVB_DEBUG, "<%s> SDT: service_id = 0x%x, desc len = %d, run_st = %d, isScrambled = %d, eitSchedule = %d\n",
                    __FUNCTION__, serviceId, descLength, runningStatus, isScrambled, eitSchedule);
            p += 5;

            // Boundary check
            if((p + descLength) > payloadEnd)
            {
                break;
            }

            service.addDescriptors(MpegDescriptor::parseDescriptors(p, descLength));
            sdt->addService(service);

            p += descLength;
        }
    }

    return sdt;
}

/**
 * Build an EIT table
 *
 * @return EitTable if successful, NULL otherwise
 */
EitTable* SectionList::buildEit()
{
    // Let's create the table object
    EitTable *eit = new EitTable(m_sectionList.front().tableId, m_sectionList.front().extensionId,
                                 m_sectionList.front().version, m_sectionList.front().current);

    eit->setTsId(((uint16_t)(m_sectionList.front().payload[0]) << 8) | m_sectionList.front().payload[1]);
    eit->setNetworkId(((uint16_t)(m_sectionList.front().payload[2]) << 8) | m_sectionList.front().payload[3]);
    eit->setLastTableId(m_sectionList.front().payload[5]);

    // Time to parse the sections one by one
    for(auto it = m_sectionList.begin(), end = m_sectionList.end(); it != end; ++it)
    {
        uint8_t *p = it->payload.data();
        if(!p)
        {
            // let's ignore "empty" sections
            continue;
        }

        uint8_t *payloadEnd = p + it->payload.size();

        // Skip ts_id, network_id etc
        p += 6;
        while((p + 12) < payloadEnd)
        {
            // Let's extract the data out of the section
            uint16_t eventId      = ((uint16_t)(p[0]) << 8) | p[1];
            uint64_t startTime    = ((uint64_t)(p[2]) << 32) | ((uint64_t)(p[3]) << 24) |
                                    ((uint64_t)(p[4]) << 16) | ((uint64_t)(p[5]) << 8) |
                                    ((uint64_t)(p[6]));
            uint32_t duration     = ((uint32_t)(p[7]) << 16) | ((uint32_t)(p[8]) << 8) | p[9];
            uint8_t runningStatus = (uint8_t)(p[10]) >> 5;
            bool isScrambled      = ((p[10] & 0x8) == 0x8) ? false : true;
            uint16_t descLength   = ((uint16_t)(p[10] & 0xf) << 8) | p[11];

            OS_LOG(DVB_DEBUG, "<%s> EIT: event_id = 0x%x, dur = 0x%x, run_st = %d, des_len = %d\n",
                                        __FUNCTION__, eventId, duration, runningStatus, descLength);

            // Let's now make a DvbEvent object using the extracted data
            DvbEvent event(eventId, startTime, duration, runningStatus, isScrambled);

            p += 12;
            event.addDescriptors(MpegDescriptor::parseDescriptors(p, descLength));
            eit->addEvent(event);

            p += descLength;
        }
    }

    return eit;
}

/**
 * Build a TOT/TDT table
 *
 * @return TotTable if successful, NULL otherwise
 */
TotTable* SectionList::buildTot()
{
    // Let's create the table object
    TotTable *tot = new TotTable(m_sectionList.front().tableId, m_sectionList.front().extensionId,
                                 m_sectionList.front().version, m_sectionList.front().current);
    uint8_t *p = m_sectionList.front().payload.data();
    if(p)
    {
        uint8_t *payloadEnd = p + m_sectionList.front().payload.size();
        if ((p + 5) <= payloadEnd)
        {
            /* 16-bit MJD and 24 bits coded as 6 digits in 4-bit BCD */
            tot->setUtcTime( ((uint64_t)p[0] << 32) | ((uint64_t)p[1] << 24) | ((uint64_t)p[2] << 16) |
                             ((uint64_t)p[3] << 8) | (uint64_t)p[4] );
            p += 5;
        }

        // Parse descriptors (for TOTs only)
        if(m_sectionList.front().tableId == 0x73)
        {
            uint16_t descLength = ((uint16_t)(p[0] & 0xf) << 8) | p[1];

            p += 2;

            tot->addDescriptors(MpegDescriptor::parseDescriptors(p, descLength));
        }
    }

    return tot;
}

/**
 * Build an SI Table out of the sections in the section list
 *
 * @return SiTable* if table was built successfully, NULL otherwise
 */
SiTable* SectionList::buildTable()
{
    SiTable *tbl = NULL;

    // Sanity check
    if(m_sectionList.empty())
    {
        return tbl;
    }

    TableId tableId = static_cast<TableId>(m_sectionList.front().tableId);

    // Time to build an SI table based on the table identifier
    if((tableId == TableId::NIT) || (tableId == TableId::NIT_OTHER))
    {
        tbl = buildNit();
    }
    else if((tableId == TableId::SDT) || (tableId == TableId::SDT_OTHER))
    {
        tbl = buildSdt();
    }
    else if(tableId == TableId::BAT)
    {
        tbl = buildBat();
    }
    else if((tableId >= TableId::EIT_PF) && (tableId <= TableId::EIT_SCHED_OTHER_END))
    {
        tbl = buildEit();
    }
    else if((tableId == TableId::TDT) || (tableId == TableId::TOT))
    {
        tbl = buildTot();
    }
    else
    {
        OS_LOG(DVB_ERROR, "<%s> Unknown table id: 0x%x\n", __FUNCTION__, tableId);
    }

    return tbl;
}

/**
 * Check if the list is for EIT sections
 *
 * @param tableId
 * @return true if the list contains EIT sections, false otherwise
 */
bool SectionList::isEit(uint8_t id)
{
    TableId tableId = static_cast<TableId>(id);

    if((tableId >= TableId::EIT_PF) && (tableId <= TableId::EIT_SCHED_OTHER_END))
    {
        return true;
    }

    return false;
}

/**
 * Get a string with debug data such as table id, section numbers etc
 *
 * @return string a text string for debug purposes
 */
string SectionList::toString() const
{
    std::stringstream ss;

    if(!m_sectionList.empty())
    {
        ss << "TableId = " << (int) m_sectionList.front().tableId << ", TableExtId = "
           << (int) m_sectionList.front().extensionId << ", ";
        ss << "Section numbers:";
        std::for_each(m_sectionList.begin(), m_sectionList.end(), [&ss](const Section& sec)
        {
            ss << " " << (int)sec.number;
        });
    }
    else
    {
        ss << "Section list is empty";
    }
    return ss.str();
}

/**
 * Check if the section list is complete (all sections of the table are in place)
 *
 * @param lastRcvdSect
 * @return true if the list is complete, false otherwise
 */
bool SectionList::complete(Section& lastRcvdSect)
{
    // Sanity check
    if(m_sectionList.empty())
    {
        return false;
    }

    // If syntax indicator is off, then we can say that the table is complete right away
    if(!m_sectionList.front().syntax)
    {
        return true;
    }

    bool complete = false;

    // We need to handle EIT tables differently
    if(isEit(m_sectionList.front().tableId))
    {
        // Check only if we received the same section second time (full cycle passed)
        // to make sure we got all the sections.
        if(m_firstReceivedNumber == lastRcvdSect.number)
        {
            uint8_t lastSectionNumber = m_sectionList.front().lastNumber;
            auto it = m_sectionList.begin();
            auto end = m_sectionList.end();
            while(it != end)
            {
                if(it->number == lastSectionNumber)
                {
                    complete = true;
                    break;
                }

                // Compare to the segment_last_section_number
                if(it->number == it->payload[4])
                {
                    while((std::next(it) != end) && std::next(it)->number < lastSectionNumber)
                    {
                        ++it;
                    }
                }

                ++it;
            }
        }
        else
        {
            complete = false;
        }
    }
    else // non-EIT table
    {
        uint8_t prev = 0;

        // Let's iterate through the section list and see if the order checks out or not
        for(auto it = m_sectionList.begin(), end = m_sectionList.end(); it != end; ++it)
        {
            if(prev != it->number)
            {
                complete = false;
                break;
            }

            if(it->lastNumber == it->number)
            {
                complete = true;
            }

            prev++;
        }
    }

    return complete;
}
