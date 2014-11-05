"""
@file    network.py
@author  Yun-Pang Wang
@author  Daniel Krajzewicz
@author  Michael Behrisch
@date    2007-12-25
@version $Id$

This script is to retrive the network data, the district data and the vehicle data, generated by SUMO, from the respective XML files.
Besides, the class 'Net' is also definded here.

SUMO, Simulation of Urban MObility; see http://sumo.dlr.de/
Copyright (C) 2008-2013 DLR (http://www.dlr.de/) and contributors

This file is part of SUMO.
SUMO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
"""

import os, string, sys, datetime, math, operator
from xml.sax import saxutils, make_parser, handler
from elements import Predecessor, Vertex, Edge, Vehicle, Path, TLJunction, Signalphase
from dijkstra import dijkstraPlain, dijkstraBoost, dijkstra
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net

# Net class stores the network (vertex and edge collection). 
# Moreover, the methods for finding k shortest paths and for generating vehicular releasing times
#  are also in this class.

class Net(sumolib.net.Net):
    def __init__(self):
        sumolib.net.Net.__init__(self)
        self._startVertices = []
        self._endVertices = []
        self._paths = {}
        self._detectedLinkCounts = 0.
        self._detectedEdges = []
        
    def addNode(self, id, type=None, coord=None, incLanes=None):
        if id not in self._id2node:
            node = Vertex(id, coord, incLanes)
            self._nodes.append(node)
            self._id2node[id] = node
        self.setAdditionalNodeInfo(self._id2node[id], type, coord, incLanes)
        return self._id2node[id]
    
    def addEdge(self, id, fromID, toID, prio, function, name):
        if id not in self._id2edge:
            fromN = self.addNode(fromID)
            toN = self.addNode(toID)
            edge = Edge(id, fromN, toN, prio, function, name)
            self._edges.append(edge)
            self._id2edge[id] = edge
        return self._id2edge[id]

    def getDetectedEdges(self, datadir):
        for line in open(os.path.join(datadir, "detectedEdges.txt")):
            line = line.split('\n')[0]
            if line != '':
                edgeObj = self.getEdge(line.split(' ')[0])
                edgeObj.detected = int(line.split(' ')[1])
                if edgeObj not in self._detectedEdges:
                    self._detectedEdges.append(edgeObj)

    def initialPathSet(self):
        for startVertex in self._startVertices:
            self._paths[startVertex] = {}
            for endVertex in self._endVertices:
                self._paths[startVertex][endVertex] = []
    
    def cleanPathSet(self):
        for startVertex in self._startVertices:
            for endVertex in self._endVertices:
                self._paths[startVertex][endVertex] = []
                
    def addTLJunctions(self, junctionObj):
        self._junctions[junctionObj.label] = junctionObj
        
    def getJunction(self, junctionlabel):
        junctionObj = None
        if junctionlabel in self._junctions:
            junctionObj = self._junctions[junctionlabel]
        return junctionObj
        
    def getstartCounts(self):
        return len(self._startVertices)
        
    def getendCounts(self):
        return len(self._endVertices)
        
    def getstartVertices(self):
        return self._startVertices
        
    def getendVertices(self):
        return self._endVertices
        
    def geteffEdgeCounts(self):
        return len(self._edges)
        
    def getfullEdgeCounts(self):
        return len(self._fullEdges)
        
    def reduce(self):
        visited = set()
        for link in self._edges.itervalues():
            if link.target in visited:
                continue
            sourceNodes = set([link.target])
            targetNodes = set()
            pendingSources = [link.target]
            pendingTargets = []
            stop = False
            while not stop and (pendingSources or pendingTargets):
                if pendingSources:
                    source = pendingSources.pop()
                    for out in source.outEdges:
                        if out.kind == "real":
                            stop = True
                            break
                        if out.target not in targetNodes:
                            targetNodes.add(out.target)
                            pendingTargets.append(out.target)
                if not stop and pendingTargets:
                    target = pendingTargets.pop()
                    for incoming in target.inEdges:
                        if incoming.kind == "real":
                            stop = True
                            break
                        if incoming.source not in sourceNodes:
                            sourceNodes.add(incoming.source)
                            pendingSources.append(incoming.source)
            if stop:
                continue
            visited.update(sourceNodes)
            complete = True
            for source in sourceNodes:
                if len(source.outEdges) < len(targetNodes):
                    complete = False
                    break
            if complete:
                for target in targetNodes:
                    for edge in target.outEdges:
                        link.target.outEdges.add(edge)
                        edge.source = link.target
                for source in sourceNodes:
                    for edge in source.inEdges:
                        link.target.inEdges.add(edge)
                        edge.target = link.target

    def createBoostGraph(self):
        from boost.graph import Digraph
        self._boostGraph = Digraph()
        for vertex in self._vertices:
            vertex.boost = self._boostGraph.add_vertex()
            vertex.boost.partner = vertex
        self._boostGraph.add_vertex_property('distance')
        self._boostGraph.add_vertex_property('predecessor')
        for edge in self._fullEdges.itervalues():
            edge.boost = self._boostGraph.add_edge(edge.source.boost, edge.target.boost)
            edge.boost.weight = edge.actualtime

    def checkSmallDiff(self, ODPaths, helpPath, helpPathSet, pathcost):
        for path in ODPaths:
            if path.edges == helpPath:
                return False, False
            else:
                sameEdgeCount = 0
                sameTravelTime = 0.0
                for edge in path.edges:
                    if edge in helpPathSet:
                        sameEdgeCount += 1 
                        sameTravelTime += edge.actualtime
                if abs(sameEdgeCount - len(path.edges))/len(path.edges) <= 0.1 and abs(sameTravelTime/3600. - pathcost) <= 0.05:
                    return False, True
        return True, False
                        
    def findNewPath(self, startVertices, endVertices, newRoutes, matrixPshort, gamma, lohse, dk):
        """
        This method finds the new paths for all OD pairs.
        The Dijkstra algorithm is applied for searching the shortest paths.
        """
        newRoutes = 0
        for start, startVertex in enumerate(startVertices):
            endSet = set()
            for end, endVertex in enumerate(endVertices):
                if startVertex._id != endVertex._id and matrixPshort[start][end] > 0.:
                    endSet.add(endVertex)
            if dk == 'boost':
                D,P = dijkstraBoost(self._boostGraph, startVertex.boost)
            elif dk == 'plain':          
                D,P = dijkstraPlain(startVertex, endSet)
            elif dk == 'extend':
                D,P = dijkstra(startVertex, endSet)
            for end, endVertex in enumerate(endVertices):
                if startVertex._id != endVertex._id and matrixPshort[start][end] > 0.:
                    helpPath = []
                    helpPathSet = set()
                    pathcost = D[endVertex]/3600.
                    ODPaths = self._paths[startVertex][endVertex]
                    for path in ODPaths:
                        path.currentshortest = False
                        
                    vertex = endVertex
                    while vertex != startVertex:
                        helpPath.append(P[vertex])
                        helpPathSet.add(P[vertex])
                        vertex = P[vertex]._from
                    helpPath.reverse()
    
                    newPath, smallDiffPath = self.checkSmallDiff(ODPaths, helpPath, helpPathSet, pathcost)

                    if newPath:
                        newpath = Path(startVertex, endVertex, helpPath)
                        ODPaths.append(newpath)
                        newpath.getPathLength()
                        for route in ODPaths:
                            route.updateSumOverlap(newpath, gamma)
                        if len(ODPaths)> 1:
                            for route in ODPaths[:-1]:
                                newpath.updateSumOverlap(route, gamma)
                        if lohse:
                            newpath.pathhelpacttime = pathcost
                        else:    
                            newpath.actpathtime = pathcost
                        for edge in newpath.edges:
                            newpath.freepathtime += edge.freeflowtime
                        newRoutes += 1
                    elif not smallDiffPath:
                        if lohse:
                            path.pathhelpacttime = pathcost
                        else:
                            path.actpathtime = pathcost
                        path.usedcounts += 1
                        path.currentshortest = True
        return newRoutes

#    find the k shortest paths for each OD pair. The "k" is defined by users.
    def calcKPaths(self, verbose, kPaths, newRoutes, startVertices, endVertices, matrixPshort, gamma):
        if verbose:
            foutkpath = file('kpaths.xml', 'w')
            print >> foutkpath, """<?xml version="1.0"?>
<!-- generated on %s by $Id$ -->
<routes>""" % datetime.datetime.now()
        for start, startVertex in enumerate(startVertices):
            for vertex in self.getNodes():
                vertex.preds = []
                vertex.wasUpdated = False
            startVertex.preds.append(Predecessor(None, None, 0))
            updatedVertices = [startVertex]

            while len(updatedVertices) > 0:
                vertex = updatedVertices.pop(0)
                vertex.wasUpdated = False
                for edge in vertex.getOutgoing():
                    if edge._to != startVertex and edge._to.update(kPaths, edge):
                        updatedVertices.append(edge._to)
    
            for end, endVertex in enumerate(endVertices):
                ODPaths = self._paths[startVertex][endVertex]
                if startVertex._id != endVertex._id and matrixPshort[start][end] != 0.:
                    for startPred in endVertex.preds:
                        temppath = []
                        temppathcost = 0.
                        pred = startPred
                        vertex = endVertex
                        while vertex != startVertex:
                            temppath.append(pred.edge)
                            temppathcost += pred.edge.freeflowtime
                            vertex = pred.edge._from
                            pred = pred.pred
                        
                        if len(ODPaths) > 0:
                            minpath = min(ODPaths, key=operator.attrgetter('freepathtime'))
                            if minpath.freepathtime*1.4 < temppathcost/3600.:
                                break
                        temppath.reverse()
                        newpath = Path(startVertex, endVertex, temppath)
                        newpath.getPathLength()
                        ODPaths.append(newpath)
                        for route in ODPaths:
                            route.updateSumOverlap(newpath, gamma)
                        if len(ODPaths)> 1:
                            for route in ODPaths[:-1]:
                                newpath.updateSumOverlap(route, gamma)
                        newpath.freepathtime = temppathcost/3600.
                        newpath.actpathtime = newpath.freepathtime
                        newRoutes += 1
                        if verbose:
                            foutkpath.write('    <path id="%s" source="%s" target="%s" pathcost="%s">\n' %(newpath.label, newpath.source, newpath.target, newpath.actpathtime))
                            foutkpath.write('        <route>')
                            for edge in newpath.edges[1:-1]:
                                foutkpath.write('%s ' %edge.label)
                            foutkpath.write('</route>\n')
                            foutkpath.write('    </path>\n')
        if verbose:
            foutkpath.write('</routes>\n')
            foutkpath.close()
            
        return newRoutes

    def printNet(self, foutnet):
        foutnet.write('Name\t Kind\t FrNode\t ToNode\t length\t MaxSpeed\t Lanes\t CR-Curve\t EstCap.\t Free-Flow TT\t ratio\t Connection\n')
        for edgeName, edgeObj in self._edges.iteritems():
            foutnet.write('%s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %d\n'
            %(edgeName, edgeObj.kind, edgeObj.source, edgeObj.target, edgeObj.length, 
              edgeObj.maxspeed, edgeObj.numberlane, edgeObj.CRcurve, edgeObj.estcapacity, edgeObj.freeflowtime, edgeObj.ratio, edgeObj.connection))


class DistrictsReader(handler.ContentHandler):
    """The class is for parsing the XML input file (districts).
       The data parsed is written into the net.
    """
    def __init__(self, net):
        self._net = net
        self._node = None
        self.I = 100

    def startElement(self, name, attrs):
        if name=='taz':
            self._node = self._net.addNode(attrs['id'])
            self._net._startVertices.append(self._node)
            self._net._endVertices.append(self._node)
        elif name=='tazSink':
            sinklink = self._net.getEdge(attrs['id'])
            self.I += 1
            newEdge = self._net.addEdge(self._node._id + str(self.I), sinklink._to._id, self._node._id, -1, "connector", "")
            newEdge._length = 0.
            newEdge.ratio = attrs['weight']
            newEdge.connection = 1
        elif name=='tazSource':
            sourcelink = self._net.getEdge(attrs['id'])
            self.I += 1
            newEdge = self._net.addEdge(self._node._id + str(self.I), self._node._id, sourcelink._from._id, -1, "connector", "")
            newEdge._length = 0.
            newEdge.ratio = attrs['weight']
            newEdge.connection = 2

## This class is for parsing the additional/updated information about singal timing plans
class ExtraSignalInformationReader(handler.ContentHandler):
    def __init__(self, net):
        self._net = net
        self._junctionObj = None
        self._junctionlabel = None
        self._phaseObj = None
        self._chars = ''
        self._counter = 0
        self._phasenoInfo = True

    def startElement(self, name, attrs):
        self._chars = ''
        if name == 'tl-logic' or name == 'tlLogic':
            if attrs.has_key('id'):
                self._junctionObj = self._net.getJunction(attrs['id'])
                if self._junctionObj:
                    self._junctionObj.phases = []
                else:
                    self._junctionObj = TLJunction()
                    self._junctionObj.label = attrs['id']
                    self._net.addTLJunctions(self._junctionObj)
                self._phasenoInfo = False
        elif name == 'phase':
            if attrs.has_key('state'):
                self._newphase = Signalphase(float(attrs['duration']), attrs['state'])
            else:
                self._newphase = Signalphase(float(attrs['duration']), None, attrs['phase'], attrs['brake'], attrs['yellow'])
            if self._junctionObj:
                self._junctionObj.phases.append(self._newphase)
                self._counter += 1
                self._newphase.label = self._counter
      
    def characters(self, content):
        self._chars += content

    def endElement(self, name):
        if name == 'key':
            self._junctionObj.label = self._chars
            self._junctionObj = self._net.getJunction(self._junctionObj.label)
            if self._junctionObj:
                self._junctionObj.phases = []
            else:
                self._junctionObj = TLJunction()
                self._junctionObj.label = self._junctionObj.label
                self._net.addTLJunctions(self._junctionObj)
            self._chars = ''
        elif name == 'phaseno':
            self._junctionObj.phaseNum = int(self._chars)
            self._chars = ''
        elif name == 'tl-logic' or name == 'tlLogic':
            if not self._phasenoInfo:
                self._junctionObj.phaseNum = self._counter
            self._counter = 0
            self._phasenoInfo = True
            self._junctionObj.label = None
            self._junctionObj = None

class DetectedFlowsReader(handler.ContentHandler):
    def __init__(self, net):
        self._net = net
        self._edge = ''
        self._edgeObj = None
        self._detectorcounts = 0.
        self._renew = False
        self._skip = False
    
    def startElement(self, name, attrs):
        if name == 'edge':
            if self._edge != '' and self._edge == attrs['id']:
                if self._edgeObj.detectorNum < float(attrs['detectors']):
                    self._edgeObj.detectorNum = float(attrs['detectors'])
                    self.renew = True
                elif self._edgeObj.detectorNum > float(attrs['detectors']):
                    self._skip = True
            else:
                self._edge = attrs['id']
                self._edgeObj = self._net.getEdge(self._edge)
                self._edgeObj.detectorNum = float(attrs['detectors'])
             
        elif name == 'flows':
            if self._renew == True:
                self._newdata.label = attrs['weekday-time']
                self._newdata.flowPger = float(attrs['passengercars'])
                self._newdawta.flowTruck = float(attrs['truckflows'])
    
            else:
                if not self._skip:
                    self._newdata = DetectedFlows(attrs['weekday-time'], float(attrs['passengercars']), float(attrs['truckflows']))
                    self._edgeObj.detecteddata[self._newdata.label]= self._newdata
                
    def endElement(self, name):
        if name == 'edge':
            self._renew = False
