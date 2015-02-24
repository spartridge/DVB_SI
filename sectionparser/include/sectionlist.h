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


#ifndef SECTIONLIST_H_
#define SECTIONLIST_H_

// C system includes
#include <stdint.h>

// C++ system includes
#include <vector>
#include <list>
#include <string>

/**
 * SI Section structure
 */
struct Section
{
    /**
     * Constructor
     *
     * @param data section data
     * @param len data length
     */
    Section(uint8_t *data, size_t len);

    /**
     * Table Identifier
     * Defines the structure of the syntax section and other contained data.
     */
    uint8_t tableId;

    /**
     * Section syntax indicator
     * A flag that indicates if the syntax section follows the section length.
     */
    bool syntax;

    /**
     * Section length
     * The number of bytes that follow for the syntax section (with CRC value) and/or table data.
     */
    uint16_t length;

    /**
     * Table ID extension
     * Used for identification of a sub_table
     */
    uint16_t extensionId;

    /**
     * Version number
     * Incremented when data is changed and wrapped around on overflow for values greater than 32.
     */
    uint8_t version;

    /**
     * Current/next indicator
     * Indicates if data is current in effect or is for future use.
     * If the bit is flagged on, then the data is to be used at the present moment.
     */
    bool current;

    /**
     * Section number
     * This is an index indicating which table this is in a related sequence of tables.
     * The first table starts from 0.
     */
    uint8_t number;

    /**
     * Last section number
     * This indicates which table is the last table in the sequence of tables.
     */
    uint8_t lastNumber;

    /**
     * Table data
     */
    std::vector<uint8_t> payload;
};

// Forward declarations
class SiTable;
class NitTable;
class BatTable;
class SdtTable;
class EitTable;
class TotTable;

/**
 * Section list class
 */
class SectionList
{
public:

    /**
      * Constructor
      */
    SectionList();

    /**
     * Destructor
     */
    virtual ~SectionList();

    /**
     * Add a section to the section list
     *
     * @param section
     * @return true if the section was added successfully
     */
    bool add(Section& section);

    /**
     * Build an SI Table out of the sections in the section list
     *
     * @return SiTable* if table was built successfully, NULL otherwise
     */
    SiTable* buildTable();

    /**
     * Get the completeness flag
     *
     * @return true if the list is complete, false otherwise
     */
    bool isComplete() const
    {
        return m_complete;
    }

    /**
     * Get a string with debug data such as table id, section numbers etc
     *
     * @return string a text string for debug purposes
     */
    std::string toString() const;

private:

    /**
     * Insert section into the list
     *
     * @param section SI section
     */
    void insert(Section& section);

    /**
     * Check if the section list is complete (all sections of the table are in place)
     *
     * @param lastRcvdSect
     * @return true if the list is complete, false otherwise
     */
    bool complete(Section& lastRcvdSect);

    /**
     * Initialize the section list
     *
     * @param section SI section
     */
    void init(Section& section);

    /**
     * Check if the list is for EIT sections
     *
     * @param tableId
     * @return true if the list contains EIT sections, false otherwise
     */
    bool isEit(uint8_t tableId);

    /**
     * Build an NIT table
     *
     * @return NitTable if successful, NULL otherwise
     */
    NitTable* buildNit();

    /**
     * Build a BAT table
     *
     * @return BatTable if successful, NULL otherwise
     */
    BatTable* buildBat();

    /**
     * Build an SDT table
     *
     * @return SdtTable if successful, NULL otherwise
     */
    SdtTable* buildSdt();

    /**
     * Build an EIT table
     *
     * @return EitTable if successful, NULL otherwise
     */
    EitTable* buildEit();

    /**
     * Build a TOT/TDT table
     *
     * @return TotTable if successful, NULL otherwise
     */
    TotTable* buildTot();

    /**
     * Completeness flag
     */
    bool m_complete;

    /**
     * First section number that was received
     */
    uint8_t m_firstReceivedNumber;

    /**
     * The list of sections
     */
    std::list<Section> m_sectionList;
};

#endif /* SECTIONLIST_H_ */
