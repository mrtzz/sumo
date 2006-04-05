//---------------------------------------------------------------------------//
//                        MapEdges.h -
//
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Jun 2005
//  copyright            : (C) 2005 by Danilo Boyom
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : danilo.tete-boyom@dlr.com
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.4  2006/04/05 05:35:54  dkrajzew
// debugging
//
// Revision 1.3  2005/10/07 11:42:59  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.2  2005/09/23 06:05:18  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.1  2005/09/15 12:09:27  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.1  2005/09/09 12:53:16  dksumo
// tools added
//
//
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include "MapEdges.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <stdlib.h>
#include <direct.h>
#include <math.h>
#include <utils/common/StdDefs.h>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * static variables
 * ======================================================================= */
MapEdges::DictTypeJunction MapEdges::myJunctionDictA;
MapEdges::DictTypeJunction MapEdges::myJunctionDictB;
std::map<std::string, std::string> MapEdges::myEdge2JunctionAMap;
std::map<std::string, std::string> MapEdges::myEdge2JunctionBMap;


/* =========================================================================
 * member definitions
 * ======================================================================= */
//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

MapEdges::MapEdges(const char *netA, const char *netB)
        : net_a(netA), net_b(netB)
{
}


MapEdges::~MapEdges()
{
}


//////////////////////////////////////////////////////////////////////
// Definitions of the Methods
//////////////////////////////////////////////////////////////////////
/// load net-file and save the Position into a dictionnary
void
MapEdges::load(void)
{
    cout << "Parsing network#1 ('" << net_a << "')..." << endl;
	loadNet(net_a,1);
    cout << "done." << endl;
    cout << "Parsing network#1 ('" << net_b << "')..." << endl;
	loadNet(net_b,2);
    cout << "done." << endl;

}


string
getAttr(string from, string attrName)
{
    size_t beg = from.find(attrName);
    beg = from.find("\"", beg);
    size_t end = from.find("\"", beg+1);
    return from.substr(beg+1, end-beg-1);
}


void
MapEdges::loadNet(const char *net, int dic)
{
	char buffer[_MAX_PATH];
	getcwd(buffer,_MAX_PATH);
	ifstream out(net);
    if (!out) {
        cerr << "cannot open file: " << net << endl;
        exit(-1);
	}

	std::string buff;
	int l = 0;
	while(!out.eof()) {
		getline(out,buff);
		if(buff.find("<junction id=")!=string::npos){
			l = l + 1;
			std::string id = buff.substr(buff.find("=")+2,buff.find(" t")-buff.find("=")-3);
			MapEdges::Junction *junction = new Junction(id);
       		std::string rest = buff.substr(buff.find("x=")+2,buff.find(">")-buff.find("x="));

			std::string  pos1 = rest.substr(1,rest.find(" ")-2);
			std::string  pos2 = rest.substr(rest.find("y=")+3,rest.find(">")-rest.find("y=")-4);

			Position2D pos(atof(pos1.c_str()),atof(pos2.c_str()));
			junction->pos = pos;

			if (dic == 1){
				myJunctionDictA[id] = junction;
			}else{
				myJunctionDictB[id] = junction;
			}
		}
        if(buff.find("<edge id=")!=string::npos) {
            string id = getAttr(buff, "id");
            string from = getAttr(buff, " From");
			if (dic == 1){
				myEdge2JunctionAMap[id] = from;
			}else{
				myEdge2JunctionBMap[from] = id;
			}

        }

	}
	out.close();
}


void
MapEdges::setJunctionA(std::string a,std::string b,std::string c)
{
	juncA1 = a;
	juncA2 = b;
	juncA3 = c;
}

void
MapEdges::setJunctionB(std::string a,std::string b,std::string c)
{
	juncB1 = a;
	juncB2 = b;
	juncB3 = c;

}

void
MapEdges::convertA(void)
{
    cout << "Resetting positions for first network" << endl;
	DictTypeJunction::iterator i;

	i = myJunctionDictA.find(juncA1);
    if(i==myJunctionDictA.end()) {
        cerr << "Could not find junction '" << juncA1 << "'!" << endl;
        throw 1;
    }
	Junction *j1 = (*i).second;

	i = myJunctionDictA.find(juncA2);
    if(i==myJunctionDictA.end()) {
        cerr << "Could not find junction '" << juncA2 << "'!" << endl;
        throw 1;
    }
	Junction *j2 = (*i).second;

	i = myJunctionDictA.find(juncA3);
    if(i==myJunctionDictA.end()) {
        cerr << "Could not find junction '" << juncA3 << "'!" << endl;
        throw 1;
    }
	Junction *j3 = (*i).second;

	SUMOReal xmin = MIN3(j1->pos.x(),j2->pos.x(),j3->pos.x());
	SUMOReal xmax = MAX3(j1->pos.x(),j2->pos.x(),j3->pos.x());
	SUMOReal xw   = xmax - xmin ;
    cout << "first network sizes " << endl;
    cout << " (xmin, xmax, width):" <<  xmin << ", " << xmax << ", " << xw << endl;

    SUMOReal ymin = MIN3(j1->pos.y(),j2->pos.y(),j3->pos.y());
	SUMOReal ymax = MAX3(j1->pos.y(),j2->pos.y(),j3->pos.y());
	SUMOReal yw   = ymax - ymin ;
    cout << " (ymin, ymax, height):" <<  ymin << ", " << ymax << ", " << yw << endl;

	for(DictTypeJunction::iterator j=myJunctionDictA.begin(); j!=myJunctionDictA.end(); j++) {
		SUMOReal nx = ((*j).second->pos.x() -xmin)/xw;
		SUMOReal ny = ((*j).second->pos.y() -ymin)/yw;
        ((*j).second->pos).set(nx,ny);
	}
    cout << "Finished conversion the first network." << endl << endl;
}

/// compare all value to find the MapEdges point
/// write results in a file
void
MapEdges::convertB(void)
{
    cout << "Resetting positions for second network" << endl;

	DictTypeJunction::iterator i;
	i = myJunctionDictB.find(juncB1);
    if(i==myJunctionDictB.end()) {
        cout << "Could not find junction '" << juncB1 << "'!" << endl;
        throw 1;
    }
	Junction *j1 = (*i).second;

	i = myJunctionDictB.find(juncB2);
    if(i==myJunctionDictB.end()) {
        cout << "Could not find junction '" << juncB2 << "'!" << endl;
        throw 1;
    }
	Junction *j2 = (*i).second;

	i = myJunctionDictB.find(juncB3);
    if(i==myJunctionDictB.end()) {
        cout << "Could not find junction '" << juncB3 << "'!" << endl;
        throw 1;
    }
	Junction *j3 = (*i).second;

	SUMOReal xmin = MIN3(j1->pos.x(),j2->pos.x(),j3->pos.x());
	SUMOReal xmax = MAX3(j1->pos.x(),j2->pos.x(),j3->pos.x());
	SUMOReal xw   = xmax - xmin ;
    cout << "second network sizes " << endl;
    cout << " (xmin, xmax, width):" <<  xmin << ", " << xmax << ", " << xw << endl;

    SUMOReal ymin = MIN3(j1->pos.y(),j2->pos.y(),j3->pos.y());
	SUMOReal ymax = MAX3(j1->pos.y(),j2->pos.y(),j3->pos.y());
	SUMOReal yw   = ymax - ymin ;
    cout << " (ymin, ymax, height):" <<  ymin << ", " << ymax << ", " << yw << endl;

	for(DictTypeJunction::iterator j=myJunctionDictB.begin(); j!=myJunctionDictB.end(); j++) {
		SUMOReal nx = ((*j).second->pos.x() -xmin)/xw;
		SUMOReal ny = ((*j).second->pos.y() -ymin)/yw;
        ((*j).second->pos).set(nx,ny);
	}
    cout << "Finished conversion the first network." << endl << endl;
}


void
MapEdges::result(const std::string &file){
	ofstream out(file.c_str());
    if(!out.good()) {
        cerr << "Could not open '" << file << "'." << endl;
        return;
    }
	for(std::map<std::string, std::string>::iterator i=myEdge2JunctionAMap.begin(); i!=myEdge2JunctionAMap.end(); i++) {
		 SUMOReal minAbstand = 77777;
		 std::string id = "";
         string nodeID = (*i).second;
         Position2D posA = myJunctionDictA[nodeID]->pos;
		 for(DictTypeJunction::iterator j=myJunctionDictB.begin(); j!=myJunctionDictB.end(); j++) {
			 SUMOReal X = pow(posA.x() - (*j).second->pos.x(),2);
			 SUMOReal Y = pow(posA.y() - (*j).second->pos.y(),2);
			 SUMOReal nabstand = sqrt(X+Y);
			 if(nabstand < minAbstand ){
				 minAbstand = nabstand;
				 id = myEdge2JunctionBMap[(*j).second->id];
			 }
		 }
		 out<< (*i).first << ";" << id <<endl;
		 cout <<(*i).first <<";"<< id <<endl;
	}
	out.close();
}



/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int main(int argc, char** argv)
{
	if (argc<9) {
        cerr << "Syntax-Error!" << endl;
        cerr << "Syntax: MapEdges <NET_A> <NET_B> <JUNCTION_A1> <JUNCTION_A2> <JUNCTION_A3> \\"
            << endl
            << "   <JUNCTION_B1> <JUNCTION_B2> <JUNCTION_B3> <OUTPUT_FILE>" << endl;
      return -1;
    }

	MapEdges *app = new MapEdges(argv[1], argv[2]);
	app->setJunctionA(argv[3], argv[4], argv[5]);
	app->setJunctionB(argv[6], argv[7], argv[8]);
	app->load();
	app->convertA();
	app->convertB();
	app->result(argv[9]);
	return 0;
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:
