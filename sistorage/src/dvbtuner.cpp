// Copyright (c) 2014 ARRIS Group, Inc. All rights reserved.
//
// This program is confidential and proprietary to ARRIS Group, Inc. (ARRIS),
// and may not be copied, reproduced, modified, disclosed to others, published
// or used, in whole or in part, without the express prior written permission
// of ARRIS.

#include <stdio.h>

// C++ system includes
#include <memory>

// Project's includes

#include "oswrap.h"
#include "dvbtuner.h"
#include "os_dvbtuner.h"

using std::shared_ptr;

/**
 * Constructor
 */
DvbTuner::DvbTuner()
{
}

/**
 * Destructor
 */
DvbTuner::~DvbTuner()
{
}

/**
 * createTuner - A factory method to create a platform specific tuner.
 */
shared_ptr<DvbTuner>  DvbTuner::createTuner()
{
    // creating os_DvbTuner derived object
    shared_ptr<DvbTuner> tuner(new os_DvbTuner());
    return  tuner;
}

/**
 * Tune to given frequency using given modulation and symbol rate
 *
 * @param freq frequency
 * @param mod modulation
 * @param symbol_rate symbol rate
 * @return int32_t 0 if successful
 */
int32_t DvbTuner::tune(uint32_t freq, DvbModulationMode mod, uint32_t symbol_rate)
{
    OS_LOG(DVB_ERROR, "%s Need to define platform tuner\n", __FUNCTION__);
    return 0;
}

/**
 * Untune a previously tuned tuner
 * Note: It destroys the pipeline and frees the resources
 */
void DvbTuner::untune()
{
    OS_LOG(DVB_ERROR, "%s Need to define platform tuner\n", __FUNCTION__);
}
