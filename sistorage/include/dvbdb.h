// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#ifndef _DVBDB_H
#define _DVBDB_H

// C system includes
#include <sys/time.h>

// C++ system includes
#include <string>
#include <list>
#include <mutex>

// Other libraries' includes
#include "sqlite3pp.h"


// Forward declarations
class MpegDescriptor;

/**
 * ElapseTime class. Debug class used for runtime performance measurements.
 */
class ElapseTime
{
  public:
    friend std::ostream& operator<<(std::ostream& os, ElapseTime& et);

    /**
     * Constructor
     */
    ElapseTime()
        : m_begin(0),
          m_end(0)
    {
    };

    /**
     * Destructor
     */
    ~ElapseTime()
    {
    };

    /**
     * Start time measurement
     */
    void start()
    {
        gettimeofday(&m_timeval, NULL);
        m_begin = m_timeval.tv_sec + (m_timeval.tv_usec / 1000000.0); \
    };
     
    /**
     * Finish time measurement
     */
    void finish()
    {
        gettimeofday(&m_timeval, NULL);
        m_end = m_timeval.tv_sec + (m_timeval.tv_usec / 1000000.0);
        m_end -= m_begin;
    };

    /**
     * Display elapsed time from start to finish
     */
    void display();

  private:
    /**
     * system time data structure
     */ 
    struct timeval m_timeval;

    /**
     * begin time value
     */ 
    double m_begin;

    /**
     * end time value
     */ 
    double m_end;
};

/** 
 * Vector of strings
 */
typedef std::vector<std::string>  StringVector;

/**
 * DVB Db class. Provides an application layer support for a Database suport.
 */
class DvbDb
{
  public:
    enum
    {
        CMD_STR_LEN = 512
    };

    /**
     * FileStatus - DvbDb open return codes
     */
    enum FileStatus
    {
        FS_ERROR,
        INVALID_NAME,
        OPENED,
        CREATED
    };

    /**
     * Database - sanity return codes
     */
    enum SanityStatus
    {
        UNKNOWN,
        EMPTY,
        POPULATED,
        CORRUPT,
        FULL,
        STALE
    };

    /**
     * Constructor
     */
    DvbDb();

    /**
     * Destructor
     */
    ~DvbDb();

    /**
     * Open or create a connection to a database file
     *
     * @param dbname string containing database file name
     * @return FileStatus OPENED or CREATED if successful
     */
    FileStatus open(std::string dbname);

    /**
     * Close the connection to a database file
     */
    void close();

    /**
     * Set status of whether we have received a TOT si table
     * 
     * @param val bool true means TOT tables has been received
     */
    void setTotReceived(bool val)
    {
        m_totReceived = val;
    }

    /**
     * Get status of whether we have received a TOT table
     */
    bool getTotReceived()
    {
        return  m_totReceived;
    }

    /**
     * Retrieve all scanner environmental variables stored in the datatbase
     */
    size_t loadSettings();

    /**
     * Retrieve a scan setting value
     * 
     * @param variable constant char pointer of variable name
     * @return constant char pointer of variable's value
     */
    const char* getSetting(const char* variable);

    /**
     * Set a scan setting value
     *
     * @param variable constant char pointer of variable name
     * @param variable constant char pointer of variable value
     */
    void setSetting(const char* variable, const char* value);

    /**
     * Clear all scan settings
     */
    void clearSettings();

    /**
     * Create defined database tables
     */
    void createTables();

    /**
     * Drop defined database tables
     */
    void dropTables();

    /**
     * Determine the number of rows modified in the last database operation.
     *
     * @return int32_t returns the number of modified rows
     */
    int32_t modifications();

    /**
     * Retreive the last rowid value from the previous insert db operation
     *
     * @return int64_t returns the rowid value
     */
    const int64_t last_insert_rowid ();

    /**
     * Find the primary key (rowid) value
     *
     * @param queryStr string containing the SQL query to find the primary key
     * @return int64_t returns the rowid value from the last db insert operation
     */
    const int64_t findKey(std::string& queryStr);

    /**
     * Find the primary key (rowid) and version
     *
     * @param queryStr string containing the SQL query to find the primary key
     * @param version int8_t retreive the version column value specified in the queryStr parameter
     * @return int64_t returns the rowid value from the last db insert operation
     */
    const int64_t findKey(std::string& queryStr, int8_t& version);

    /**
     * execute a single step SQL command
     *
     * @param cmdStr string containing the SQL command
     * @return int32_t returns the status of the operation 0 if successful, -1 for failure
     */
    int32_t sqlCommand(std::string cmdStr);

    /**
     * Find descriptors specified in SQL statement
     *
     * @param queryStr string containing the SQL command
     * @return int32_t returns the status of the operation 0 if successful, -1 for failure
     */
    std::vector<std::shared_ptr<MpegDescriptor>> findDescriptor(std::string queryStr);

    /**
     * Insert a vector of descriptors 
     *
     * @param tablename const char pointer of the name of the descriptor table
     * @param fkey int64_t parent foreign key value
     * @param descList vector of MpegDescriptors
     * @return int32_t returns the status of the operation 0 if successful, -1 for failure
     */
    int32_t insertDescriptor(const char* tableName, const int64_t& fkey, const std::vector<MpegDescriptor>& descList);

    /**
     * Insert Component vector of descriptors 
     *
     * @param tablename const char pointer of the name of the descriptor table
     * @param fkey int64_t parent foreign key value
     * @param descList vector of Component descriptors
     * @return int32_t returns the status of the operation 0 if successful, -1 for failure
     */
    int32_t insertComponent(const char* tableName, const int64_t& fkey, const std::vector<MpegDescriptor>& descList);

    /**
     * Add an update command for processing at a later time.
     *
     * @param updateCmd string containing SQL update command
     */
    void addUpdate(std::string updateCmd);

    /**
     * Dump the update command collection
     */
    void dumpUpdate();

    /**
     * Clear the update command collection
     */
    void clearUpdate();

    /**
     * Execute update commands contained in the collection
     */
    void performUpdate();

    /**
     * Audits for database maintenance
     */
    void audits();

    /**
     * Query the database given a SQL statement
     * 
     * @param cmd string containing the SQL command (select)
     * @return a vector of vector of strings containing the results of the query
     */
    std::vector<std::vector<std::string>> query(std::string& cmd);

    /**
     * Command class. Used for binding variables to SQL statements.
     */
    class Command
    {
      public:
        /**
         * Constructor
         *
         * @param handle reference of DvbDb object
         * @param cmdBuf string containing the SQL command
         */
        Command(DvbDb& handle, std::string cmdBuf);

        /**
         * Destructor
         */
        ~Command();

        /**
         * Is Command object valid.
         *
         * @return bool true for valid
         */
        bool isValid()
        {
            return m_valid;
        };

        /**
         * Bind variable to SQL command statement.
         *
         * @param int32_t index (1's base) of the argument to bind the NULL TYPE variable to
         * @return int32_t returns the status of the operation 0 if successful, -1 for failure
         */
        int32_t bind(int32_t index);

        /**
         * Bind variable to SQL command statement.
         *
         * @param int32_t index (1's base) of the argument to bind the variable to
         * @param int variable to bind
         * @return int32_t returns the status of the operation 0 if successful, -1 for failure
         */
        int32_t bind(int32_t index, int value);

        /**
         * Bind variable to SQL command statement.
         *
         * @param int32_t index (1's base) of the argument to bind the variable to
         * @param long long int variable to bind
         * @return int32_t returns the status of the operation 0 if successful, -1 for failure
         */
        int32_t bind(int32_t index, long long int value);

        /**
         * Bind variable to SQL command statement.
         *
         * @param int32_t index (1's base) of the argument to bind the variable to
         * @param string variable to bind
         * @return int32_t returns the status of the operation 0 if successful, -1 for failure
         */
        int32_t bind(int32_t index, std::string& value);

        /**
         * Execute SQL command statement.
         *
         * @return int32_t returns the status of the operation 0 if successful, -1 for failure
         */
        int32_t execute();

        /**
         * Execute SQL command statement.
         *
         * @param int64_t foreign key value from Db insert operation
         * @return int32_t returns the status of the operation 0 if successful, -1 for failure
         */
        int32_t execute(int64_t& fkey);
          
      private:
        /**
         * Default Constructor
         */
        Command();

        /**
         * Copy Constructor
         */
        Command(const Command& other) = delete;

        /**
         * Assignment operator
         */
        Command& operator=(const Command&) = delete;

        /**
         * Command object status
         */ 
        bool m_valid;

        /**
         * DvbDb object reference
         */ 
        DvbDb& m_handle;

        /**
         * shared pointer of a sqlite3pp command object
         */ 
        std::shared_ptr<sqlite3pp::command> m_command;
    };

    /**
     * Transaction class. Used to group mutiple SQL statements as one Db operation
     */
    class Transaction
    {
      public:
        /**
         * Constructor
         *
         * @param handle reference of DvbDb object
         */
        Transaction(DvbDb& handle);

        /**
         * Destructor
         */
        ~Transaction();

        /**
         * Commit group of SQL statements to the database
         */
        void commit();

        /**
         * Rollback or do NOT commit the group of SQL statement to the database
         */
        void rollback();

        /**
         * Is Transaction object valid.
         *
         * @return bool true for valid
         */
        bool isValid()
        {
            return  m_valid;
        };

      private:
        /**
         * Default Constructor
         */
        Transaction();

        /**
         * Copy Constructor
         */
        Transaction(const Transaction& other) = delete;

        /**
         * Assignment operator
         */
        Transaction& operator=(const Transaction&) = delete;

        /**
         * Transaction object status
         */ 
        bool m_valid;

        /**
         * DvbDb object reference
         */ 
        DvbDb& m_handle;

        /**
         * shared pointer of a sqlite3pp transaction object
         */ 
        std::shared_ptr<sqlite3pp::transaction> m_transaction;
    };
    

  private:

    /**
     * Repair class. Used to perform update SQL commands later
     */
    class Repair
    {
      public:
        enum
        {
            NUM_TRIES = 3  // Maximum number of times to attempt to peform Update SQL command
        };

        /** 
         * Constructor
         */
        Repair (std::string fStr)
          : fixed (false),
            count (0),
            fixStr (fStr)
        {
        }

        /**
         * Update command succeded. Predicate function for remove_if operation to remove from collection
         * 
         * @param rhs reference to a constant Repair object in collection
         * @return bool true means to remove from collection; false to keep
         */
        static bool isFixed (const Repair& rhs)
        {
            return  rhs.fixed;
        }

        /**
         * Maximum number of attempts to perform update comamand. Predicate function for remove_if operation to remove from collection
         *
         * @param rhs reference to a constant Repair object in collection
         * @return bool true means to remove from collection; false to keep
         */
        static bool isExpired (const Repair& rhs)
        {
            return  (rhs.count > static_cast<int32_t> (NUM_TRIES));
        }


        /**
         * status of update SQL command.
         */
        bool         fixed;

        /**
         * number of times update SQL command was executed.
         */
        int32_t      count;

        /**
         * SQL update command string
         */
        std::string  fixStr;
    };

    /**
     * DbSetting class. Environment variables to save in the database.
     */
    class DbSetting
    {
      public:
        /**
         * Default Constructor
         */
        DbSetting()
        {
        };

        /**
         * Constructor
         */
        DbSetting(std::string var, std::string val)
          : variable (var),
            value (val)
        {
        };

        /**
         * Destructor
         */
        ~DbSetting()
        {
        }

        /**
         * Environment variable name
         */
        std::string  variable;

        /**
         * Environment variabl value 
         */
        std::string  value;
    };


    /**
     * Copy Constructor
     */
    DvbDb (const DvbDb& rhs) = delete;

    /**
     * Assignment operator
     */
    DvbDb& operator= (const DvbDb& rhs) = delete;

    /**
     * Retrieve sqlite3 reference from sqlite3pp wrapper
     */
    sqlite3pp::database& handle()
    {
        return m_sqlDb;
    };

    /**
     * Create database file
     *
     * @return FileStatus CREATE enumeration upon success otherwise an error.
     */
    FileStatus create();

    /**
     * Create DVB database schema
     */
    void createSchema();

    /**
     * Drop DVB database schema
     */
    void dropSchema();

    /**
     * Remove expired database records
     */
    void purgeAudit();

    /**
     * Perform garbage collection on database file.
     */
    void vacuumAudit();

    /**
     * Determine if contents have expired
     */
    void staleAudit();

    /**
     * Sanity check to determine if the database file is valid
     */
    SanityStatus sanityCheck();

    /**
     * Content check to determine if the database has DVB SI data
     */
    SanityStatus contentCheck();

    /**
     * Stale check to determine if the database contents have expired
     */
    SanityStatus staleCheck();

    /**
     * Most recent start timestamp stored
     * 
     * @return uint64_t UTC timestamp
     */
    uint64_t latestStartTime();

    /**
     * Execute a single SQL command.
     *
     * @param cmdStr constant char pointer of SQL command
     * NOTE: Private method does not lock mutex
     */
    int32_t sqlCommand(const char* cmdStr);

    /**
     * DVB database schema as vector of strings
     */
    static const StringVector m_schema;

    /**
     * DVB database schema table names - vector of strings
     */
    static const StringVector m_tables;
  
    /**
     * boolean whether TOT has been received
     */ 
    bool m_totReceived;

    /**
     * boolean whether data has expired
     */ 
    bool m_staleDataCheck;

    /**
     * DvbDb mutex
     */
    std::mutex m_mutex;

    /**
     * DvbDb file name
     */
    std::string m_dbName;

    /**
     * sqlite3pp object
     */
    sqlite3pp::database m_sqlDb;

    /**
     * Update commands collection
     */
    std::list<Repair> m_updateList;

    /**
     * Vector of DbSetting objects
     */
    std::vector<DbSetting> m_settings;
};

#endif // _DVBDB_H


