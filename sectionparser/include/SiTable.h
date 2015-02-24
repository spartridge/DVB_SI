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


#ifndef __RMF_DVBSITABLE_H__
#define __RMF_DVBSITABLE_H__

// C system includes
#include <stdint.h>

/**
 * Table identifier enumeration
 */
enum class TableId : uint8_t
{
    PAT = 0x00,                  //!< PAT
    CAT = 0x01,                  //!< CAT
    PMT = 0x02,                  //!< PMT
    TSDT = 0x03,                 //!< TSDT
    NIT = 0x40,                  //!< NIT
    NIT_OTHER = 0x41,            //!< NIT_OTHER
    SDT = 0x42,                  //!< SDT
    SDT_OTHER = 0x46,            //!< SDT_OTHER
    BAT = 0x4a,                  //!< BAT
    EIT_PF = 0x4e,               //!< EIT_PF
    EIT_PF_OTHER = 0x4f,         //!< EIT_PF_OTHER
    EIT_SCHED_START = 0x50,      //!< EIT_SCHED_START
    EIT_SCHED_END = 0x5f,        //!< EIT_SCHED_END
    EIT_SCHED_OTHER_START = 0x60,//!< EIT_SCHED_OTHER_START
    EIT_SCHED_OTHER_END = 0x6f,  //!< EIT_SCHED_OTHER_END
    TDT = 0x70,                  //!< TDT
    RST = 0x71,                  //!< RST
    ST = 0x72,                   //!< ST
    TOT = 0x73,                  //!< TOT
    DIT = 0x7e,                  //!< DIT
    SIT = 0x7f,                  //!< SIT
    USER_DEFINED_START = 0x80,   //!< USER_DEFINED_START
    USER_DEFINED_END = 0xfe      //!< USER_DEFINED_END
};

/**
 * Service Information(SI) table base class
 */
class SiTable
{
public:
    /**
     * Constructor
     *
     * @param id table id
     * @param extId extension id
     * @param ver version number
     * @param cur current/next indicator
     */
    SiTable(uint8_t id, uint16_t extId, uint8_t ver, bool cur)
        : m_tableId(id),
          m_extensionId(extId),
          m_version(ver),
          m_current(cur)
    {
    }

    /**
     * Desctructor
     */
    virtual ~SiTable()
    {
    }

    /**
     * Check if table is current
     *
     * @return true if current, false otherwise
     */
    bool isCurrent() const
    {
        return m_current;
    }

    /**
     * Get extension identifier
     *
     * @return
     */
    uint16_t getExtensionId() const
    {
        return m_extensionId;
    }

    /**
     * Get table identifier
     *
     * @return TableId
     */
    TableId getTableId() const
    {
        return static_cast<TableId>(m_tableId);
    }

    /**
     * Get version number
     *
     * @return version number
     */
    uint8_t getVersion() const
    {
        return m_version;
    }

private:
    /**
     * Table identifier
     */
    uint8_t m_tableId;

    /**
     * Version number
     */
    uint8_t m_version;

    /**
     * Table identifier extension
     */
    uint16_t m_extensionId;

    /**
     * Current/next indicator
     */
    bool m_current;
};

#endif
