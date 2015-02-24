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
