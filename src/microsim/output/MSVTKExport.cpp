/****************************************************************************/
/// @file    MSVTKExport.cpp
/// @author  Mario Krumnow
/// @date    2012-04-26
/// @version $Id$
///
// Realises VTK Export
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.dlr.de/
// Copyright (C) 2001-2013 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
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

#include <microsim/MSEdgeControl.h>
#include <microsim/MSJunctionControl.h>

#include <microsim/MSVehicle.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>

#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include <utils/iodevices/OutputDevice.h>
#include "MSVTKExport.h"

#ifdef HAVE_MESOSIM
#include <mesosim/MELoop.h>
#include <mesosim/MESegment.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
void
MSVTKExport::write(OutputDevice& of, SUMOTime /* timestep */) {

	std::vector<double> speed = getSpeed();
    std::vector<double> points = getPositions();

	of << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
	of << "<VTKFile type=\"PolyData\" version=\"0.1\" order=\"LittleEndian\">\n";
	of << "<PolyData>\n";
	of << " <Piece NumberOfPoints=\"" << speed.size() << "\" NumberOfVerts=\"1\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\"0\">\n";
	of << "<PointData>\n";
	of << " <DataArray type=\"Float64\" Name=\"speed\" format=\"ascii\">" << List2String(getSpeed()) << "</DataArray>\n";
	of << "</PointData>\n";
	of << "<CellData/>\n";
	of << "<Points>\n";
    of << " <DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">" << List2String(getPositions()) << "</DataArray>\n";
	of << "</Points>\n";
	of << "<Verts>\n";
	of << " <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">" <<  getOffset((int) speed.size()) << "</DataArray>\n";
    of << " <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">" << speed.size() << "</DataArray>\n";
    of << "</Verts>\n";
	of << "<Lines>\n";
	of << " <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\"/>\n";
	of << " <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\"/>\n";
	of << "</Lines>\n";
	of << "<Stripes>\n";
	of << " <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\"/>\n";
	of << " <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\"/>\n";
	of << "</Stripes>\n";
	of << "<Polys>\n";
	of << " <DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\"/>\n";
	of << " <DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\"/>\n";
	of << "</Polys>\n";
	of << "</Piece>\n";
	of << "</PolyData>\n";
	of << "</VTKFile>";

}

std::vector<double>
MSVTKExport::getSpeed() {

    std::vector<double> output;

    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt it = vc.loadedVehBegin();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();


    for (; it != end; ++it) {
        const MSVehicle* veh = static_cast<const MSVehicle*>((*it).second);

        if (veh->isOnRoad()) {

            Position pos = veh->getLane()->getShape().positionAtOffset(veh->getPositionOnLane());
            output.push_back(veh->getSpeed());
        }

    }

    return output;
}

std::vector<double>
MSVTKExport::getPositions() {

    std::vector<double> output;

    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt it = vc.loadedVehBegin();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();


    for (; it != end; ++it) {
        const MSVehicle* veh = static_cast<const MSVehicle*>((*it).second);

        if (veh->isOnRoad()) {

            output.push_back(veh->getPosition().x());
            output.push_back(veh->getPosition().y());
            output.push_back(veh->getPosition().z());

        }

    }

    return output;
}

std::string
MSVTKExport::List2String(std::vector<double> input) {

    std::string output = "";
    for (unsigned i = 0; i < input.size(); i++) {

        std::stringstream ss;

        //for a high precision
        //ss.precision(::std::numeric_limits<double>::digits10);
        //ss.unsetf(::std::ios::dec);
        //ss.setf(::std::ios::scientific);

        ss << input[i] << " ";
        output += ss.str();
    }

    return trim(output);
}

std::string
MSVTKExport::getOffset(int nr) {

    std::string output = "";
    for (int i = 0; i < nr; i++) {

        std::stringstream ss;
        ss << i << " ";
        output += ss.str();
    }

    return trim(output);
}

bool
MSVTKExport::ctype_space(const char c) {
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == 11) {
        return true;
    }
    return false;
}

std::string
MSVTKExport::trim(std::string istring) {
    bool trimmed = false;

    if (ctype_space(istring[istring.length() - 1])) {
        istring.erase(istring.length() - 1);
        trimmed = true;
    }

    if (ctype_space(istring[0])) {
        istring.erase(0, 1);
        trimmed = true;
    }

    if (!trimmed) {
        return istring;
    } else {
        return trim(istring);
    }

}

/****************************************************************************/
