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


#include "dvbdb.h"

// C system includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Other libraries' includes

// Project's includes
#include "oswrap.h"
#include "MpegDescriptor.h"
#include "ComponentDescriptor.h"

using namespace std;
using namespace sqlite3pp;

/**
 * Static Initialization
 */

const StringVector DvbDb::m_tables =
{
    "EitDescriptor", 
    "Eit",
    "SdtDescriptor",
    "Sdt",
    "BatDescriptor",
    "BatTransportDescriptor",
    "BatTransport", 
    "Bat",
    "NitDescriptor",
    "NitTransportDescriptor",
    "NitTransport",
    "Nit",
    "EventItem",
    "EventComponent",
    "Event",
    "ServiceComponent",
    "Service",
    "Transport",
    "Bouquet",
    "Network"
};

const StringVector DvbDb::m_schema =
{
    "CREATE TABLE IF NOT EXISTS Nit ("          \
    "nit_pk INTEGER PRIMARY KEY NOT NULL, "     \
    "network_id INTEGER  NOT NULL, "            \
    "version INTEGER NOT NULL);",

    "CREATE TABLE IF NOT EXISTS NitDescriptor ( " \
    "fkey INTEGER NOT NULL,"                      \
    "descriptor_id INTEGER NOT NULL,"             \
    "descriptor BLOB NOT NULL,"                   \
    "FOREIGN KEY (fkey) REFERENCES Nit (nit_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS NitDescriptor_index ON NitDescriptor (" \
    "fkey,"                                                                    \
    "descriptor_id,"                                                           \
    "descriptor);",

    "CREATE TABLE IF NOT EXISTS NitTransport ("      \
    "nit_transport_pk INTEGER PRIMARY KEY NOT NULL," \
    "original_network_id INTEGER NOT NULL,"          \
    "transport_id INTEGER NOT NULL,"                 \
    "nit_fk INTEGER NOT NULL,"                       \
    "FOREIGN KEY (nit_fk) REFERENCES Nit (network_id));",

    "CREATE UNIQUE INDEX IF NOT EXISTS NitTransport_index ON NitTransport (" \
    "original_network_id,"                                                   \
    "transport_id,"                                                          \
    "nit_fk);",

    "CREATE TABLE IF NOT EXISTS NitTransportDescriptor ( "  \
    "fkey INTEGER NOT NULL,"                                \
    "descriptor_id INTEGER NOT NULL,"                       \
    "descriptor BLOB NOT NULL,"                             \
    "FOREIGN KEY (fkey) REFERENCES NitTransport (nit_transport_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS NitTransportDescriptor_index ON NitTransportDescriptor (" \
    "fkey,"                                                                                      \
    "descriptor_id,"                                                                             \
    "descriptor);",

    "CREATE TABLE IF NOT EXISTS Sdt ("             \
    "sdt_pk INTEGER PRIMARY KEY NOT NULL,"         \
    "service_id INTEGER NOT NULL,"                 \
    "nit_transport_fk INTEGER,"                    \
    "version INTEGER NOT NULL,"                    \
    "schedule INTEGER NOT NULL,"                   \
    "present_following INTEGER NOT NULL,"          \
    "scrambled INTEGER NOT NULL,"                  \
    "running INTEGER NOT NULL, "                   \
    "FOREIGN KEY (nit_transport_fk) "              \
    "REFERENCES NitTransport (nit_transport_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS Sdt_index ON Sdt (" \
    "service_id,"                                          \
    "nit_transport_fk);",

    "CREATE TABLE IF NOT EXISTS SdtDescriptor ( " \
    "fkey INTEGER NOT NULL,"                      \
    "descriptor_id INTEGER NOT NULL,"             \
    "descriptor BLOB NOT NULL,"                   \
    "FOREIGN KEY (fkey) REFERENCES Sdt (sdt_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS SdtDescriptor_index ON SdtDescriptor (" \
    "fkey,"                                                                    \
    "descriptor_id,"                                                           \
    "descriptor);",

    "CREATE TABLE IF NOT EXISTS Eit ("             \
    "eit_pk INTEGER PRIMARY KEY NOT NULL,"         \
    "network_id INTEGER NOT NULL,"                 \
    "transport_id INTEGER NOT NULL,"               \
    "service_id INTEGER NOT NULL,"                 \
    "event_id INTEGER NOT NULL,"                   \
    "version INTEGER NOT NULL,"                    \
    "start_time INTEGER NOT NULL,"                 \
    "duration INTEGER NOT NULL,"                   \
    "scrambled INTEGER NOT NULL,"                  \
    "running INTEGER NOT NULL);"                   \

    "CREATE UNIQUE INDEX IF NOT EXISTS Eit_index ON Eit (" \
    "network_id,"                                          \
    "transport_id,"                                        \
    "service_id,"                                          \
    "event_id);",

    "CREATE TABLE IF NOT EXISTS EitDescriptor ("   \
    "fkey INTEGER NOT NULL,"                       \
    "descriptor_id INTEGER NOT NULL,"              \
    "descriptor BLOB NOT NULL,"                    \
    "FOREIGN KEY (fkey) REFERENCES Eit (eit_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS EitDescriptor_index ON EitDescriptor (" \
    "fkey,"                                                                    \
    "descriptor_id,"                                                           \
    "descriptor);",

    "CREATE TABLE IF NOT EXISTS Bat ("             \
    "bat_pk INTEGER PRIMARY KEY NOT NULL,"         \
    "bouquet_id INTEGER NOT NULL,"                 \
    "version INTEGER NOT NULL);",

    "CREATE TABLE IF NOT EXISTS BatDescriptor ( "  \
    "fkey INTEGER NOT NULL,"                       \
    "descriptor_id INTEGER NOT NULL,"              \
    "descriptor BLOB NOT NULL,"                    \
    "FOREIGN KEY (fkey) REFERENCES Bat (bat_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS BatDescriptor_index ON BatDescriptor (" \
    "fkey,"                                                                    \
    "descriptor_id,"                                                           \
    "descriptor);",

    "CREATE TABLE IF NOT EXISTS BatTransport ("      \
    "bat_transport_pk INTEGER PRIMARY KEY NOT NULL," \
    "original_network_id INTEGER NOT NULL,"          \
    "transport_id INTEGER NOT NULL,"                 \
    "bat_fk INTEGER NOT NULL, "                      \
    "nit_transport_fk INTEGER,"                      \
    "FOREIGN KEY (nit_transport_fk) REFERENCES "     \
    "  NitTransport (nit_transport_pk)  "            \
    "FOREIGN KEY (bat_fk) REFERENCES Bat (bat_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS BatTransport_index ON BatTransport (" \
    "original_network_id,"                                                   \
    "transport_id,"                                                          \
    "bat_fk);",

    "CREATE TABLE IF NOT EXISTS BatTransportDescriptor ( "  \
    "fkey INTEGER NOT NULL,"                                \
    "descriptor_id INTEGER NOT NULL,"                       \
    "descriptor BLOB NOT NULL,"                             \
    "FOREIGN KEY (fkey) REFERENCES BatTransport (bat_transport_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS BatTransportDescriptor_index ON BatTransportDescriptor (" \
    "fkey,"                                                                                      \
    "descriptor_id,"                                                                             \
    "descriptor);",

    // Parsed

    "CREATE TABLE IF NOT EXISTS Bouquet ("      \
    "bouquet_pk INTEGER PRIMARY KEY NOT NULL, " \
    "bouquet_id INTEGER NOT NULL,"              \
    "version INTEGER NOT NULL,"                 \
    "iso_639_language_code TEXT,"               \
    "name TEXT);",

    "CREATE UNIQUE INDEX IF NOT EXISTS Bouquet_index ON Bouquet (" \
    "bouquet_id);",

    "CREATE TABLE IF NOT EXISTS Network ("      \
    "network_pk INTEGER PRIMARY KEY NOT NULL, " \
    "network_id INTEGER NOT NULL,"              \
    "version INTEGER NOT NULL,"                 \
    "iso_639_language_code TEXT,"               \
    "name TEXT);",

    "CREATE UNIQUE INDEX IF NOT EXISTS Network_index ON Network (" \
    "network_id);",

    "CREATE TABLE IF NOT EXISTS Transport ("                    \
    "transport_pk INTEGER PRIMARY KEY NOT NULL,"                \
    "original_network_id INTEGER NOT NULL,"                     \
    "transport_id INTEGER NOT NULL,"                            \
    "network_fk INTEGER NOT NULL,"                              \
    "bouquet_fk INTEGER,"                                       \
    "frequency INTEGER NOT NULL,"                               \
    "modulation INTEGER NOT NULL,"                              \
    "symbol_rate INTEGER NOT NULL,"                             \
    "fec_outer INTEGER NOT NULL,"                               \
    "fec_inner INTEGER NOT NULL,"                               \
    "FOREIGN KEY (network_fk) REFERENCES Network (network_pk)," \
    "FOREIGN KEY (bouquet_fk) REFERENCES Bouquet (bouquet_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS Transport_index ON Transport (" \
    "original_network_id,"                                             \
    "transport_id,"                                                    \
    "network_fk);",

    "CREATE TABLE IF NOT EXISTS Service ("         \
    "service_pk INTEGER PRIMARY KEY NOT NULL,"     \
    "service_id INTEGER NOT NULL,"                 \
    "transport_fk INTEGER,"                        \
    "version INTEGER NOT NULL,"                    \
    "service_type INTEGER NOT NULL,"               \
    "logical_channel_number INTEGER,"              \
    "running INTEGER NOT NULL, "                   \
    "scrambled INTEGER NOT NULL,"                  \
    "schedule INTEGER NOT NULL,"                   \
    "present_following INTEGER NOT NULL,"          \
    "iso_639_language_code TEXT,"                  \
    "service_name TEXT,"                           \
    "provider_name TEXT,"                          \
    "FOREIGN KEY (transport_fk) REFERENCES Transport (transport_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS Service_index ON Service (" \
    "service_id,"                                                  \
    "transport_fk);",

    "CREATE TABLE IF NOT EXISTS ServiceComponent ("   \
    "fkey INTEGER NOT NULL,"                          \
    "stream_content INTEGER NOT NULL,"                \
    "component_type INTEGER NOT NULL,"                \
    "component_tag INTEGER NOT NULL,"                 \
    "iso_639_language_code TEXT,"                     \
    "description TEXT, "                              \
    "FOREIGN KEY (fkey) REFERENCES Service (service_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS ServiceComponent_index ON ServiceComponent (" \
    "fkey,"                                                                          \
    "stream_content,"                                                                \
    "component_type,"                                                                \
    "component_tag);",

    "CREATE TABLE IF NOT EXISTS Event ("           \
    "event_pk INTEGER PRIMARY KEY NOT NULL,"       \
    "network_id INTEGER NOT NULL,"                 \
    "transport_id INTEGER NOT NULL,"               \
    "service_id INTEGER NOT NULL,"                 \
    "event_id INTEGER NOT NULL,"                   \
    "version INTEGER NOT NULL,"                    \
    "start_time INTEGER NOT NULL,"                 \
    "duration INTEGER NOT NULL,"                   \
    "scrambled INTEGER NOT NULL,"                  \
    "running INTEGER NOT NULL, "                   \
    "parental_rating TEXT,"                        \
    "content TEXT);",

    "CREATE UNIQUE INDEX IF NOT EXISTS Event_index ON Event (" \
    "network_id,"                                              \
    "transport_id,"                                            \
    "service_id,"                                              \
    "event_id);",

    "CREATE TABLE IF NOT EXISTS EventComponent ("     \
    "fkey INTEGER NOT NULL,"                          \
    "stream_content INTEGER NOT NULL,"                \
    "component_type INTEGER NOT NULL,"                \
    "component_tag INTEGER NOT NULL,"                 \
    "iso_639_language_code TEXT,"                     \
    "description TEXT, "                              \
    "FOREIGN KEY (fkey) REFERENCES Event (event_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS EventComponent_index ON EventComponent (" \
    "fkey,"                                                                      \
    "stream_content,"                                                            \
    "component_type,"                                                            \
    "component_tag);",

    "CREATE TABLE IF NOT EXISTS EventItem ("          \
    "event_fk INTEGER NOT NULL,"                      \
    "iso_639_language_code TEXT,"                     \
    "title TEXT, "                                    \
    "description TEXT, "                              \
    "FOREIGN KEY (event_fk) REFERENCES Event (event_pk));",

    "CREATE UNIQUE INDEX IF NOT EXISTS EventItem_index ON EventItem (" \
    "event_fk,"                                                        \
    "iso_639_language_code);"
};

/**
 *  ElapseTime Methods
 */

/**
 * global ostream method for ElapseTime object
 */
ostream& operator<< (ostream& os, ElapseTime& et)
{
    os << et.m_end << " seconds " << et.m_end / 60.0 << " minutes.";
    return os;
}

/**
 * Display elapsed time in seconds
 */
void ElapseTime::display()
{
   OS_LOG(DVB_DEBUG, "<%s> - Elapsed time: %lf seconds\n", __FUNCTION__, m_end);
}

/**
 *  DvbDb Methods
 */


/**
 *  Constructor
 */
DvbDb::DvbDb()
  : m_totReceived(false),
    m_staleDataCheck(false)
{
}

/**
 *  Destructor
 */
DvbDb::~DvbDb()
{
    // clear out all collections
    clearUpdate();
    clearSettings();
}

/**
 * Open or create a connection to a database file
 *
 * @param dbname string containing database file name
 * @return FileStatus OPENED or CREATED if successful
 */
DvbDb::FileStatus  DvbDb::open(std::string dbname)
{
    FileStatus  status = FS_ERROR;

    if(dbname.empty())
    {
        OS_LOG(DVB_ERROR, "<%s> - Invalid database file name. Using default.\n", __FUNCTION__);
        dbname = "/opt/persistent/si/dvb.db"; // use default name if not specified.
    }

    m_dbName = dbname;

    if(m_sqlDb.connect_v2(m_dbName.c_str(), SQLITE_OPEN_READWRITE) == SQLITE_OK)
    {
        status = OPENED;
        OS_LOG(DVB_DEBUG, "<%s> - Opened database %s, Performing sanity checks.\n", __FUNCTION__, m_dbName.c_str());

        SanityStatus sanity = sanityCheck();
        switch(sanity)
        {
            case POPULATED:
                OS_LOG(DVB_DEBUG, "<%s> - Database %s is populated!\n", __FUNCTION__, m_dbName.c_str(), 
                    static_cast<int>(sanity), m_sqlDb.error_code(), m_sqlDb.error_msg());
                status = OPENED;
                break;

            case EMPTY:
            case CORRUPT:
            case FULL:
            case STALE:
            case UNKNOWN:
            default:
                OS_LOG(DVB_DEBUG, "<%s> - Empty or corrupt Database %s Error:%d, %d, %s\n",
                       __FUNCTION__, m_dbName.c_str(), static_cast<int>(sanity), m_sqlDb.error_code(), m_sqlDb.error_msg());
                status = FS_ERROR;
                break;
        } 
    }

    if(status != OPENED)
    {
        std::remove(m_dbName.c_str()); // remove db file
        status = create();             // create db file and create tables
    }

    return  status;
}

/**
 * Create database file
 *
 * @return FileStatus CREATE enumeration upon success otherwise an error.
 */
DvbDb::FileStatus  DvbDb::create()
{
    FileStatus  status = FS_ERROR;

    if(m_sqlDb.connect_v2(m_dbName.c_str(), (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)) == SQLITE_OK)  // Create Db
    {
        OS_LOG(DVB_DEBUG, "<%s> - Created database: %s\n", __FUNCTION__, m_dbName.c_str());
        createSchema();
        status = CREATED;
    }
    else
    {
        OS_LOG(DVB_ERROR, "<%s> - Unable to create database file: %s : error: %d, %s\n",
                 __FUNCTION__, m_dbName.c_str(), m_sqlDb.error_code(), m_sqlDb.error_msg());
        status = FS_ERROR;
    }

    return  status;
}

/**
 * Close the connection to a database file
 */
void DvbDb::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_sqlDb.disconnect();
}

/**
 * Stale check to determine if the database contents have expired
 */
DvbDb::SanityStatus  DvbDb::staleCheck()
{
    SanityStatus  status = UNKNOWN;

    uint64_t latestStart = latestStartTime();
    if(latestStart > 0)
    {
        status = POPULATED;

        struct timeval timeVal = {};
        int rc = gettimeofday(&timeVal, NULL);
        // if current time is greater than last Eit event - DvbDb is stale.
        if(rc == 0 && timeVal.tv_sec > latestStart)
        {
            OS_LOG(DVB_INFO, "<%s> STALE Db! latest start time %llu, %d\n", __FUNCTION__, latestStart, timeVal.tv_sec);

            status = STALE;
        }
    }

    return  status;
}

/**
 * Determine if contents have expired
 */
void  DvbDb::staleAudit()
{
    // received TOT event and have NOT checked for stale condition.
    if(m_totReceived && !m_staleDataCheck)
    {
        m_staleDataCheck = true; // check stale condition once!

        DvbDb::SanityStatus sanity = staleCheck();
        if(sanity == DvbDb::STALE)
        {
            OS_LOG(DVB_INFO, "<%s> STALE Db detected!\n", __FUNCTION__);

            dropSchema();
            createSchema();
        }
    }
}

/**
 * Sanity check to determine if the database file is valid
 */
DvbDb::SanityStatus  DvbDb::sanityCheck()
{
    SanityStatus  status = UNKNOWN;

    // Run VACUUM on DvbDb for a sanity check. Then check for an error code.
    vacuumAudit();

    switch(m_sqlDb.error_code())
    {
        case SQLITE_NOTADB:
        case SQLITE_FORMAT:
        case SQLITE_CORRUPT:
        case SQLITE_IOERR:
        case SQLITE_PROTOCOL:
        case SQLITE_INTERNAL:
            status = CORRUPT;
            OS_LOG(DVB_ERROR, "<%s> - Db is corrupt!\n", __FUNCTION__);
            break;

        case SQLITE_FULL:
            OS_LOG(DVB_ERROR, "<%s> - Db is FULL!\n", __FUNCTION__);
            status = FULL;  // file limit size exceeded - not good.
            break;

        case SQLITE_OK:
            OS_LOG(DVB_DEBUG, "<%s> - Db OK, checking content!\n", __FUNCTION__);
            status = contentCheck();
            break;

        default:
            status = UNKNOWN; // could be benign
            break;
    }

    return  status;
}

/**
 * Content check to determine if the database has DVB SI data
 */
DvbDb::SanityStatus DvbDb::contentCheck()
{
    SanityStatus status = EMPTY;
    const char*  queryStr = "SELECT nit_pk FROM Nit;";

    try
    {
        sqlite3pp::query qry(m_sqlDb, queryStr);

        if(qry.column_count() != 1)
        {
            OS_LOG(DVB_ERROR, "<%s> - Query must contain 1 column - it has %d: %s\n", __FUNCTION__, qry.column_count(), queryStr);
            throw new logic_error("Query should contain only one column.");
        }

        int32_t  nitCount = 0;
        for(query::iterator it=qry.begin(); it != qry.end(); ++it)
        {
            int64_t  nit_pk = 0;

            long long int tmp;
            (*it).getter() >> tmp;
            nit_pk = static_cast<int64_t>(tmp);
            if(nit_pk > 0)
            {
                ++nitCount;
            }
        }

        if(nitCount > 0)
        {
            status = POPULATED;
        }
    }

    catch(exception& ex)
    {
        OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), queryStr);
    }

    catch(...)
    {
        OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, queryStr);
    }

    return  status;
}

/**
 * Most recent start timestamp stored
 * 
 * @return uint64_t UTC timestamp
 */
uint64_t  DvbDb::latestStartTime()
{
    long long int latestStartTime = 0;
    const char*  queryStr = "SELECT MAX(start_time) FROM Eit;";

    try
    {
        sqlite3pp::query qry(m_sqlDb, queryStr);

        if(qry.column_count() != 1)
        {
            OS_LOG(DVB_ERROR, "<%s> - Query must contain 1 column - it has %d: %s\n",
                   __FUNCTION__,qry.column_count(),queryStr);
            throw new logic_error("Query should contain only one column.");
        }

        for(query::iterator it=qry.begin(); it != qry.end(); ++it)
        {
            (*it).getter() >> latestStartTime;
OS_LOG(DVB_ERROR, "<%s> - querylatestStartTime = %lld, %s\n", __FUNCTION__, latestStartTime, queryStr);
        }
    }

    catch(exception& ex)
    {
        OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), queryStr);
    }

    catch(...)
    {
        OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, queryStr);
    }

    return  static_cast<uint64_t>(latestStartTime);
}

/**
 * Retrieve all scanner environmental variables stored in the datatbase
 */
size_t  DvbDb::loadSettings()
{
    clearSettings();

    sqlCommand("CREATE TABLE IF NOT EXISTS ScanSettings (" \
               "variable TEXT,"                            \
               "value TEXT);");

    const char* queryStr = "SELECT variable, value FROM ScanSettings";

    try
    {
        sqlite3pp::query qry(m_sqlDb, queryStr);

        if(qry.column_count() != 2)
        {
            OS_LOG(DVB_ERROR, "<%s> - Query must contain 2 columns - it has %d: %s\n",
                   __FUNCTION__, qry.column_count(), queryStr);
            throw new logic_error("Query should contain only one column.");
        }

        for(query::iterator it=qry.begin(); it != qry.end(); ++it)
        {
            DbSetting setting;

            (*it).getter() >> setting.variable >> setting.value;

            OS_LOG(DVB_DEBUG, "<%s> - Retrieved variable: %s = %s\n",
                    __FUNCTION__, setting.variable.c_str(), setting.value.c_str());

            m_settings.push_back(setting);
        }
    }

    catch(exception& ex)
    {
        OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), queryStr);
    }

    catch(...)
    {
        OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, queryStr);
    }

    return  static_cast<size_t>(m_settings.size());
}

/**
 * Retrieve a scan setting value
 * 
 * @param variable constant char pointer of variable name
 * @return constant char pointer of variable's value
 */
const char*  DvbDb::getSetting(const char* varStr)
{

    for(auto it = m_settings.cbegin(); it != m_settings.cend(); ++it)
    {
        const DbSetting& setting = (*it);

        if(setting.variable.compare(varStr) == 0)
        {
            return  setting.value.c_str();
        }
    }

    return  NULL;
}

/**
 * Set a scan setting value
 *
 * @param variable constant char pointer of variable name
 * @param variable constant char pointer of variable value
 */
void DvbDb::setSetting(const char* variable, const char* value)
{

    if(variable && value)
    {
        OS_LOG(DVB_DEBUG, "<%s> - Saving variable: %s = %s\n", __FUNCTION__, variable, value);

        string cmdStr("INSERT OR REPLACE INTO ScanSettings (variable, value ) VALUES (?, ?);");

        try
        {
            command cmd(m_sqlDb, cmdStr.c_str());

            cmd.binder() << variable << value;

            cmd.execute();
        }

        catch(exception& ex)
        {
            OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), cmdStr.c_str());
        }

        catch(...)
        {
            OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, cmdStr.c_str());
        }
    }
}

/**
 * Clear all scan settings
 */
void  DvbDb::clearSettings()
{
    m_settings.clear();
}

/**
 * Find the primary key (rowid) value
 *
 * @param queryStr string containing the SQL query to find the primary key
 * @return int64_t returns the rowid value from the last db insert operation
 */
const int64_t DvbDb::findKey(string& queryStr)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t key = 0;  // invalid key

    try
    {
        sqlite3pp::query qry(m_sqlDb, queryStr.c_str());

        if(qry.column_count() != 1)
        {
            OS_LOG(DVB_ERROR, "<%s> - Query must contain 1 column - it has %d: %s\n", __FUNCTION__, qry.column_count(), queryStr.c_str());
            throw new logic_error("Query should contain only one column.");
        }

        for(query::iterator it=qry.begin(); it != qry.end(); ++it)
        {
            long long int tmp;
            (*it).getter() >> tmp;
            key = static_cast<int64_t>(tmp);
        }
    }

    catch(exception& ex)
    {
        OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), queryStr.c_str());
    }

    catch(...)
    {
        OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, queryStr.c_str());
    }

    return  key;
}

/**
 * Find the primary key (rowid) and version
 *
 * @param queryStr string containing the SQL query to find the primary key
 * @param version int8_t retreive the version column value specified in the queryStr parameter
 * @return int64_t returns the rowid value from the last db insert operation
 */
const int64_t DvbDb::findKey(string& queryStr, int8_t& version)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t key = 0;  // invalid key

    try
    {
        sqlite3pp::query qry(m_sqlDb, queryStr.c_str());

        if(qry.column_count() != 2)
        {
            OS_LOG(DVB_ERROR, "<%s> - Query must contain 2 columns - it has %d: %s\n", __FUNCTION__, qry.column_count(), queryStr.c_str());
            throw new logic_error("Query should contain two columns.");
        }

        for(query::iterator it=qry.begin(); it != qry.end(); ++it)
        {
            long long int tmp;
            (*it).getter() >> tmp >> version;
            key = static_cast<int64_t>(tmp);
        }
    }

    catch(exception& ex)
    {
        OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), queryStr.c_str());
    }

    catch(...)
    {
        OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, queryStr.c_str());
    }

    return  key;
}

/**
 * Retreive the last rowid value from the previous insert db operation
 *
 * @return int64_t returns the rowid value
 */
const int64_t DvbDb::last_insert_rowid()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return  static_cast<const int64_t>(m_sqlDb.last_insert_rowid());
}

/**
 * Determine the number of rows modified in the last database operation.
 *
 * @return int32_t returns the number of modified rows
 */
int32_t DvbDb::modifications()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return  static_cast<int32_t>(sqlite3_changes(m_sqlDb.getHandle()));
}

/**
 * Find descriptors specified in SQL statement
 *
 * @param queryStr string containing the SQL command
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
vector<shared_ptr<MpegDescriptor>>  DvbDb::findDescriptor(string queryStr)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    vector<shared_ptr<MpegDescriptor>>  results;

    try
    {
        if(!queryStr.empty())
        {
            OS_LOG(DVB_ERROR, "<%s> - No query specified.\n", __FUNCTION__);
            throw new logic_error("Invalid query string.");
        }

        sqlite3pp::query qry(m_sqlDb, queryStr.c_str());

        if(qry.column_count() != 2 || 
            strcmp(qry.column_name(0), "descriptor_id") != 0 || 
            strcmp(qry.column_name(1), "descriptor") != 0)
        {
            OS_LOG(DVB_ERROR, "<%s> - Invalid columns specified.\n", __FUNCTION__);
            throw new logic_error("Invalid column selected.");
        }

        for(query::iterator it=qry.begin(); it != qry.end(); ++it)
        {
            string descriptor;
            int8_t descriptor_id;

            (*it).getter() >> descriptor_id >> descriptor;

            shared_ptr<MpegDescriptor> mpeg(new MpegDescriptor(static_cast<DescriptorTag>(descriptor_id), 
                                                               (uint8_t*)descriptor.data(), 
                                                                static_cast<uint8_t>(descriptor.size())));
            results.push_back(mpeg);
        }
    }

    catch(exception& ex)
    {
        OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), queryStr.c_str());
    }

    catch(...)
    {
        OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, queryStr.c_str());
    }

    return  results;
}

/**
 * Insert a vector of descriptors 
 *
 * @param tablename const char pointer of the name of the descriptor table
 * @param fkey int64_t parent foreign key value
 * @param descList vector of MpegDescriptors
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t  DvbDb::insertDescriptor(const char* tableName, const int64_t& fkey, const std::vector<MpegDescriptor>& descList)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    int32_t rc = -1;

    if(tableName && fkey > 0 && !descList.empty())
    {
        // Using 'INSERT OR IGNORE' so duplicate entries do not trigger an exception or error.
        string cmdStr("INSERT OR IGNORE INTO ");
        string tableStr(tableName);
        cmdStr += tableStr;
        cmdStr += " (fkey, descriptor_id, descriptor) VALUES (?, ?, ?);";

        try
        {
            for(auto it = descList.cbegin(); it != descList.cend(); ++it)
            {
                const MpegDescriptor& md = *it;

                command cmd(m_sqlDb, cmdStr.c_str());

                cmd.binder() << static_cast<long long int>(fkey) << static_cast<uint8_t>(md.getTag());

                const std::vector<uint8_t>& descData = md.getData();
                cmd.bind(3, static_cast<const void*>(descData.data()), descData.size());

                cmd.execute();
                rc = 0;
            }
        }

        catch(exception& ex)
        {
            OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), cmdStr.c_str());
        }

        catch(...)
        {
            OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, cmdStr.c_str());
        }

    }

    return rc;
}

/**
 * Insert Component vector of descriptors 
 *
 * @param tablename const char pointer of the name of the descriptor table
 * @param fkey int64_t parent foreign key value
 * @param descList vector of Component descriptors
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t  DvbDb::insertComponent(const char* tableName, const int64_t& fkey, const std::vector<MpegDescriptor>& descList)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    int32_t rc = -1;

    for(auto it = descList.cbegin(); it != descList.cend(); ++it)
    {
        const MpegDescriptor& md = *it;
        if(md.getTag() == DescriptorTag::COMPONENT)
        {
            ComponentDescriptor cd(md);

            OS_LOG(DVB_DEBUG, "<%s> %s COMPONENT fkey %lld con: %d type: %d tag: %d lang: %s text: %s\n",
                __FUNCTION__, tableName, fkey, static_cast<int>(cd.getStreamContent()), static_cast<int>(cd.getComponentType()), 
                static_cast<int>(cd.getComponentTag()), cd.getLanguageCode().c_str(), cd.getText().c_str());

            string cmdStr = "INSERT OR IGNORE INTO ";
            cmdStr += tableName;
            cmdStr += " (fkey, stream_content, component_type, component_tag, iso_639_language_code, description) " \
                      " VALUES (?, ?, ?, ?, ?, ?);";

            try
            {
                command cmd(m_sqlDb, cmdStr.c_str());

                cmd.binder() << static_cast<long long int>(fkey)
                             << static_cast<int>(cd.getStreamContent())
                             << static_cast<int>(cd.getComponentType())
                             << static_cast<int>(cd.getComponentTag())
                             << cd.getLanguageCode()
                             << cd.getText();

                cmd.execute();

                int64_t component_fk = last_insert_rowid();
                OS_LOG(DVB_DEBUG, "<%s> %s component_fk : %lld\n", __FUNCTION__, tableName, component_fk);
            }

            catch(std::exception& ex)
            {
                OS_LOG(DVB_ERROR, "<%s> - Exception: %s, %s\n", __FUNCTION__, ex.what(), cmdStr.c_str());
            }

            catch(...)
            {
                OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, cmdStr.c_str());
            }

        }
    }

    return rc;
}

/**
 * Audits for database maintenance
 */
void DvbDb::audits()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // TODO: Remove debug code.
    ElapseTime elapse;
    elapse.start();


    // TODO: Enable audits
    // performUpdate();
    // staleAudit();
    // purgeAudit();
    // vacuumAudit();

    elapse.finish();
    elapse.display();
}

/**
 * Add an update command for processing at a later time.
 *
 * @param updateCmd string containing SQL update command
 */
void DvbDb::addUpdate(string updateCmd)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(!updateCmd.empty())
    {
        bool found = false;

        for(auto it = m_updateList.begin(); it != m_updateList.end(); ++it)
        {
            Repair& repair = (*it);
            if(repair.fixStr.compare(updateCmd) == 0)
            {
                found = true;
                break;
            }
        }

        if(!found) 
        {
            Repair* repair = new Repair(updateCmd);
            if(repair)
            {
                m_updateList.push_back(*repair);
            }
        }
    }
}

/**
 * Dump the update command collection
 */
void DvbDb::dumpUpdate()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    int32_t i = 0;
    for(auto it = m_updateList.begin(); it != m_updateList.end(); ++it,++i)
    {
        Repair& repair = (*it);
        OS_LOG(DVB_DEBUG, "<%s> %d> fixed %d, count %d, %s\n",
              __FUNCTION__,i,repair.fixed,repair.count,repair.fixStr.c_str());
    }
}

/**
 * Clear the update command collection
 */
void DvbDb::clearUpdate()
{
    m_updateList.clear();
}

/**
 * Execute update commands contained in the collection
 */
void DvbDb::performUpdate()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    OS_LOG(DVB_DEBUG, "<%s> Repair size: %u\n", __FUNCTION__, m_updateList.size());

    for(auto it = m_updateList.begin(); it != m_updateList.end(); ++it)
    {
        Repair& repair = (*it);
        if(!repair.fixed)
        {
            int32_t rc = sqlCommand(repair.fixStr.c_str());
            int32_t mods = modifications();
            repair.count++;
            if(rc == 0 && mods > 0)
            {
                repair.fixed = true;  // mark as fixed.
                OS_LOG(DVB_DEBUG, "<%s> FIXED! rc=%d,count=%d,fixed=%d,mods=%d\n", __FUNCTION__, rc, 
                    repair.count, repair.fixed, mods);
            }
            else
            {
                OS_LOG(DVB_DEBUG, "<%s> rc=%d,count=%d,mods=%d\n", __FUNCTION__, rc, repair.count, mods);
            }
        }
    }

    // Update sometimes reports a changed tuple.
    m_updateList.remove_if(Repair::isFixed);
    m_updateList.remove_if(Repair::isExpired);
}

/**
 * execute a single step SQL command
 *
 * @param cmdStr string containing the SQL command
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t DvbDb::sqlCommand(string cmdStr)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return  sqlCommand(cmdStr.c_str());
}

/**
 * Execute a single SQL command.
 * NOTE: Private method does not lock mutex
 */
int32_t DvbDb::sqlCommand(const char* cmdStr)
{
    int32_t rc = -1;

    if(cmdStr)
    {
        try
        {
            command cmd(m_sqlDb, cmdStr);

            cmd.execute();
            rc = 0;  // success
        }

        catch(exception& ex)
        {
            OS_LOG(DVB_ERROR, "<%s> - Exception: %s cmd: %s\n", __FUNCTION__, ex.what(), cmdStr);
        }

        catch(...)
        {
            OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: cmd: %s\n", __FUNCTION__, cmdStr);
        }

    }

    return  rc;
}

/**
 * Create defined database tables
 */
void DvbDb::createTables()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    createSchema();
}

/**
 * Create DVB database schema
 */
void DvbDb::createSchema()
{
    // DO NOT LOCK Mutex!

    if(m_sqlDb.error_code() != 0)
    {
        OS_LOG(DVB_ERROR, "<%s> - invalid database: %s\n", __FUNCTION__, m_dbName.c_str());
        return;
    }

    OS_LOG(DVB_DEBUG, "<%s> - Creating schema.\n", __FUNCTION__);

    for(auto it=m_schema.cbegin(); it != m_schema.cend(); ++it)
    {
        const string& cmd = *it;
        if(!cmd.empty())
        {
            sqlCommand(cmd.c_str());
        }
    }
}

/**
 * Drop defined database tables
 */
void DvbDb::dropTables()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    dropSchema();
}

/**
 * Drop DVB database schema
 */
void DvbDb::dropSchema()
{
    // Drop the leaf (child) tables first then drop the parent tables 
    // otherwise a foreign key constraint exception will be thrown.

    OS_LOG(DVB_DEBUG, "<%s> - Dropping tables\n", __FUNCTION__);

    for(auto it=m_tables.cbegin(); it != m_tables.cend(); ++it)
    {
        const string& tableName = *it;
        if(!tableName.empty())
        {
            string cmdStr("DROP TABLE IF EXISTS ");
            cmdStr += tableName;
            cmdStr += ";";   // terminate SQL command
            sqlCommand(cmdStr.c_str());
        }
    }

}

/**
 * Perform garbage collection on database file.
 */
void DvbDb::vacuumAudit()
{
    sqlCommand("VACUUM;");
}

/**
 * Remove expired database records
 */
void DvbDb::purgeAudit()
{
    transaction trans(m_sqlDb);

    // TODO: make the expired time configurable.
    sqlCommand("DELETE FROM Eit WHERE start_time + duration < date('now','-1 hour');"); 

    sqlCommand("DELETE FROM EitDescriptor WHERE fkey NOT IN " \
                " (SELECT DISTINCT eit_pk FROM Eit);");

    sqlCommand("DELETE FROM Event WHERE start_time + duration < date('now','-1 hour');"); 

    sqlCommand("DELETE FROM EventItem WHERE event_fk NOT IN " \
                " (SELECT DISTINCT event_pk FROM Event);");

    sqlCommand("DELETE FROM EventComponent WHERE fkey NOT IN " \
                " (SELECT DISTINCT event_pk FROM Event);");

    trans.commit();
}

/**
 * Query the database given a SQL statement
 * 
 * @param cmd string containing the SQL command (select)
 * @return a vector of vector of strings containing the results of the query
 */
vector<vector<string>> DvbDb::query(string& cmd)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    vector<vector<string>>  results;

    if(!cmd.empty())
    {
        try
        {
            sqlite3_stmt* statement;

            if(sqlite3_prepare_v2(m_sqlDb.getHandle(), cmd.c_str(), cmd.size(), &statement, NULL) == SQLITE_OK)
            {
                size_t numColumns = sqlite3_column_count(statement);

                while(true)
                {
                    int rc = sqlite3_step(statement);  
                    if(rc != SQLITE_ROW)
                    {
                        break;
                    }

                    vector<string>  row;
                    for(size_t i=0; i < numColumns; ++i)
                    {
                        string column;
                        const char* text = (const char*)sqlite3_column_text(statement, i);
                        if(text == NULL)
                        {
                            column = "";
                        }
                        else
                        {
                            column = text;
                        }

                        row.emplace_back(column);
                    }

                    if(!row.empty())
                    {
                        results.emplace_back(row);
                    }
                }

                sqlite3_finalize(statement);
            }
            else
            {
                OS_LOG(DVB_ERROR, "<%s> Unable to prepare statement: %s\n", __FUNCTION__, cmd.c_str());
            }
        }

        catch(exception& ex)
        {
            OS_LOG(DVB_ERROR, "<%s> - Exception: %s\n", __FUNCTION__, ex.what());
        }

        catch(...)
        {
            OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: %s\n", __FUNCTION__);
        }
    }

    return results;
}

/**
 * Constructor
 *
 * @param handle reference of DvbDb object
 * @param cmdBuf string containing the SQL command
 */
DvbDb::Command::Command(DvbDb& db, string cmdStr)
  : m_handle(db)
{
    std::lock_guard<std::mutex> lock(db.m_mutex);

    if(!cmdStr.empty())
    {
        m_command.reset(new command(m_handle.m_sqlDb, cmdStr.c_str()));
        if(m_command)
        {
            m_valid = true;
        }
    }
}

/**
 * Destructor
 */
DvbDb::Command::~Command()
{
    if(m_command)
    {
        m_command.reset();
    }
}

/**
 * Bind variable to SQL command statement.
 *
 * @param int32_t index (1's base) of the argument to bind the NULL TYPE variable to
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t DvbDb::Command::bind(int index)
{
    int32_t rc = -1;

    if(m_command)
    {
        // NOTE: binding Database NULL type
        m_command->bind(index, null_type()); 
        rc = 0;
    }

    return rc;
}

/**
 * Bind variable to SQL command statement.
 *
 * @param int32_t index (1's base) of the argument to bind the variable to
 * @param int variable to bind
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t DvbDb::Command::bind(int index, int value)
{
    int32_t rc = -1;

    if(m_command)
    {
        m_command->bind(index, value);
        rc = 0;
    }

    return rc;
}

/**
 * Bind variable to SQL command statement.
 *
 * @param int32_t index (1's base) of the argument to bind the variable to
 * @param long long int variable to bind
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t DvbDb::Command::bind(int index, long long int value)
{
    int32_t rc = -1;

    if(m_command)
    {
        m_command->bind(index, value);
        rc = 0;
    }

    return rc;
}

/**
 * Bind variable to SQL command statement.
 *
 * @param int32_t index (1's base) of the argument to bind the variable to
 * @param string variable to bind
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t DvbDb::Command::bind(int index, string& value)
{
    int32_t rc = -1;

    if(m_command)
    {
        m_command->bind(index, value);
        rc = 0;
    }

    return rc;
}

/**
 * Execute SQL command statement.
 *
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t DvbDb::Command::execute()
{
    std::lock_guard<std::mutex> lock(m_handle.m_mutex);

    int32_t rc = -1;

    if(m_command)
    {
        try
        {
            m_command->execute();
            rc = 0;
        }

        catch(exception& ex)
        {
            OS_LOG(DVB_ERROR, "<%s> - Exception: %s\n", __FUNCTION__, ex.what());
        }

        catch(...)
        {
            OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: %s\n", __FUNCTION__);
        }
    }

    return rc;
}

/**
 * Execute SQL command statement.
 *
 * @param int64_t foreign key value from Db insert operation
 * @return int32_t returns the status of the operation 0 if successful, -1 for failure
 */
int32_t DvbDb::Command::execute(int64_t& fkey)
{
    std::lock_guard<std::mutex> lock(m_handle.m_mutex);

    fkey = 0;
    int32_t rc = -1;

    if(m_command)
    {
        try
        {
            m_command->execute();

            fkey = static_cast<const int64_t>(m_handle.m_sqlDb.last_insert_rowid());
            rc = 0;
        }

        catch(exception& ex)
        {
            OS_LOG(DVB_ERROR, "<%s> - Exception: %s\n", __FUNCTION__, ex.what());
        }

        catch(...)
        {
            OS_LOG(DVB_ERROR, "<%s> - Unknown Exception: %s\n", __FUNCTION__);
        }
    }

    return rc;
}

/**
 * Constructor
 *
 * @param handle reference of DvbDb object
 */
DvbDb::Transaction::Transaction(DvbDb& handle)
  : m_handle(handle),
    m_valid(false)
{
    m_transaction.reset(new transaction(m_handle.m_sqlDb));
    if(m_transaction)
    {
        m_valid = true;
    }
}

/**
 * Destructor
 */
DvbDb::Transaction::~Transaction()
{
    m_transaction.reset();
}

/**
 * Commit group of SQL statements to the database
 */
void DvbDb::Transaction::commit()
{
    if(m_transaction)
    {
        m_transaction->commit();
    } 
}

/**
 * Rollback or do NOT commit the group of SQL statement to the database
 */
void DvbDb::Transaction::rollback()
{
    if(m_transaction)
    {
        m_transaction->rollback();
    } 
}

