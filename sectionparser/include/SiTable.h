// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

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
