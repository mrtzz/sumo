/****************************************************************************/
/// @file    MSTrafficLightLogic.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id$
///
// The parent class for traffic light logics
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// copyright : (C) 2001-2007
//  by DLR (http://www.dlr.de/) and ZAIK (http://www.zaik.uni-koeln.de/AFS)
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <iostream>
#include <map>
#include <microsim/MSLink.h>
#include <microsim/MSLane.h>
#include "MSTrafficLightLogic.h"
#include <microsim/MSEventControl.h>
#include "MSTLLogicControl.h"
#include <utils/helpers/DiscreteCommand.h>
#include <microsim/MSJunctionLogic.h>

#ifdef RAKNET_DEMO
#include <raknet_demo/sumo_add/ampel.h>
#include <raknet_demo/constants.h>
#include <utils/geom/Line2D.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;
#ifdef RAKNET_DEMO
Ampel *myAmpel = 0;
std::map<std::string, int> myIDs;
int myAmpelRunningID = 0;
#endif


// ===========================================================================
// member method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * member method definitions
 * ----------------------------------------------------------------------- */
MSTrafficLightLogic::SwitchCommand::SwitchCommand(MSTLLogicControl &tlcontrol,
        MSTrafficLightLogic *tlLogic)
        : myTLControl(tlcontrol), myTLLogic(tlLogic), myAmValid(true)
{}


MSTrafficLightLogic::SwitchCommand::~SwitchCommand()
{}



SUMOTime
MSTrafficLightLogic::SwitchCommand::execute(SUMOTime)
{
    // check whether this command has been descheduled
    if (!myAmValid) {
        return 0;
    }
    //
    bool isActive = myTLControl.isActive(myTLLogic);
    size_t step1 = myTLLogic->getStepNo();
    SUMOTime next = myTLLogic->trySwitch(isActive);
    size_t step2 = myTLLogic->getStepNo();
    if (step1!=step2) {
        myTLLogic->onSwitch();
        if (isActive) {
            myTLLogic->setLinkPriorities();
        }
    }
    return next;
}


void
MSTrafficLightLogic::SwitchCommand::deschedule(MSTrafficLightLogic *tlLogic)
{
    if (tlLogic==myTLLogic) {
        myAmValid = false;
    }
}


/* -------------------------------------------------------------------------
 * member method definitions
 * ----------------------------------------------------------------------- */
MSTrafficLightLogic::MSTrafficLightLogic(
    MSTLLogicControl &tlcontrol,
    const std::string &id,
    const std::string &subid,
        SUMOTime delay)
        : myID(id), mySubID(subid), myCurrentDurationIncrement(-1)
{
    mySwitchCommand = new SwitchCommand(tlcontrol, this);
    MSNet::getInstance()->getBeginOfTimestepEvents().addEvent(
        mySwitchCommand, delay, MSEventControl::ADAPT_AFTER_EXECUTION);
#ifdef RAKNET_DEMO
    if (myAmpel==0) {
        myAmpel = new Ampel();
    }
#endif
}


MSTrafficLightLogic::~MSTrafficLightLogic()
{
    for(std::vector<DiscreteCommand*>::iterator i=myOnSwitchActions.begin(); i!=myOnSwitchActions.end(); ++i) {
        delete *i;
    }
}


void
MSTrafficLightLogic::addLink(MSLink *link, MSLane *lane, size_t pos)
{
    // !!! should be done within the loader (checking necessary)
    myLinks.reserve(pos+1);
    while (myLinks.size()<=pos) {
        myLinks.push_back(LinkVector());
    }
    myLinks[pos].push_back(link);
    //
    myLanes.reserve(pos+1);
    while (myLanes.size()<=pos) {
        myLanes.push_back(LaneVector());
    }
    myLanes[pos].push_back(lane);

#ifdef RAKNET_DEMO
    myAmpel->addTrafficLight(myIDs[getID()]*1000 + pos,
                             lane->getShape()[-1].x(), 0, lane->getShape()[-1].y(), lane->getShape().getEndLine().atan2DegreeAngle());
#endif
}


const MSTrafficLightLogic::LaneVector &
MSTrafficLightLogic::getLanesAt(size_t i) const
{
    return myLanes[i];
}


const MSTrafficLightLogic::LaneVectorVector &
MSTrafficLightLogic::getLanes() const
{
    return myLanes;
}


const MSTrafficLightLogic::LinkVector &
MSTrafficLightLogic::getLinksAt(size_t i) const
{
    return myLinks[i];
}


const std::string &
MSTrafficLightLogic::getID() const
{
    return myID;
}


const std::string &
MSTrafficLightLogic::getSubID() const
{
    return mySubID;
}


const MSTrafficLightLogic::LinkVectorVector &
MSTrafficLightLogic::getLinks() const
{
    return myLinks;
}


void
MSTrafficLightLogic::addSwitchAction(DiscreteCommand *a)
{
    myOnSwitchActions.push_back(a);
}


void
MSTrafficLightLogic::onSwitch()
{
    for (std::vector<DiscreteCommand*>::iterator i=myOnSwitchActions.begin(); i!=myOnSwitchActions.end();) {
        if (!(*i)->execute()) {
            i = myOnSwitchActions.erase(i);
            // !!! delete???
        } else {
            i++;
        }
    }
#ifdef RAKNET_DEMO
    // get the current traffic light signal combination
    const std::bitset<64> &allowedLinks = allowed();
    const std::bitset<64> &yellowLinks = yellowMask();
    // go through the links
    for (size_t i=0; i<myLinks.size(); i++) {
        // set the states for assigned links
        if (!allowedLinks.test(i)) {
            if (yellowLinks.test(i)) {
                const LinkVector &currGroup = myLinks[i];
                for (LinkVector::const_iterator j=currGroup.begin(); j!=currGroup.end(); j++) {
                    myAmpel->setTrafficLightState(myIDs[getID()]*1000 + i, TRAFFIC_SIGN_YELLOW);
                }
            } else {
                const LinkVector &currGroup = myLinks[i];
                for (LinkVector::const_iterator j=currGroup.begin(); j!=currGroup.end(); j++) {
                    myAmpel->setTrafficLightState(myIDs[getID()]*1000 + i, TRAFFIC_SIGN_RED);
                }
            }
        } else {
            const LinkVector &currGroup = myLinks[i];
            for (LinkVector::const_iterator j=currGroup.begin(); j!=currGroup.end(); j++) {
                myAmpel->setTrafficLightState(myIDs[getID()]*1000 + i, TRAFFIC_SIGN_GREEN);
            }
        }
    }
#endif
}


void
MSTrafficLightLogic::adaptLinkInformationFrom(const MSTrafficLightLogic &logic)
{
    myLinks = logic.myLinks;
    myLanes = logic.myLanes;
}


void
MSTrafficLightLogic::setParameter(const std::map<std::string, std::string> &params)
{
    myParameter = params;
}


std::string
MSTrafficLightLogic::getParameterValue(const std::string &key) const
{
    if (myParameter.find(key)==myParameter.end()) {
        return "";
    }
    return myParameter.find(key)->second;
}


void
MSTrafficLightLogic::addOverridingDuration(SUMOTime duration)
{
    myOverridingTimes.push_back(duration);
}


void
MSTrafficLightLogic::setCurrentDurationIncrement(SUMOTime delay)
{
    myCurrentDurationIncrement = delay;
}


int
MSTrafficLightLogic::getLinkIndex(MSLink *link) const
{
    int index = 0;
    for (LinkVectorVector::const_iterator i1=myLinks.begin(); i1!=myLinks.end(); ++i1, ++index) {
        const LinkVector &l = (*i1);
        for (LinkVector::const_iterator i2=l.begin(); i2!=l.end(); ++i2) {
            if ((*i2)==link) {
                return index;
            }
        }
    }
    return -1;
}


std::map<MSLink*, std::pair<MSLink::LinkState, bool> > 
MSTrafficLightLogic::collectLinkStates() const
{
    std::map<MSLink*, std::pair<MSLink::LinkState, bool> > ret;
    for (LinkVectorVector::const_iterator i1=myLinks.begin(); i1!=myLinks.end(); ++i1) {
        const LinkVector &l = (*i1);
        for (LinkVector::const_iterator i2=l.begin(); i2!=l.end(); ++i2) {
            ret[*i2] = make_pair((*i2)->getState(), (*i2)->havePriority());
        }
    }
    return ret;
}


void 
MSTrafficLightLogic::resetLinkStates(const std::map<MSLink*, std::pair<MSLink::LinkState, bool> > &vals) const
{
    for (LinkVectorVector::const_iterator i1=myLinks.begin(); i1!=myLinks.end(); ++i1) {
        const LinkVector &l = (*i1);
        for (LinkVector::const_iterator i2=l.begin(); i2!=l.end(); ++i2) {
            assert(vals.find(*i2)!=vals.end());
            const std::pair<MSLink::LinkState, bool> &lvals = vals.find(*i2)->second;
            (*i2)->setTLState(lvals.first);
            (*i2)->setPriority(lvals.second);
        }
    }
}

/****************************************************************************/

