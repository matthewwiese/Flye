//(c) 2016 by Authors
//This file is a part of ABruijn program.
//Released under the BSD license (see LICENSE file)

#pragma once

#include "../sequence/sequence_container.h"
#include "../sequence/overlap.h"
#include <list>

struct SequenceSegment
{
	SequenceSegment(FastaRecord::Id seqId, int32_t start, int32_t end):
		seqId(seqId), start(start), end(end) {}

	FastaRecord::Id seqId;
	int32_t start;
	int32_t end;
};

struct GraphNode;

struct GraphEdge
{
	GraphEdge(GraphNode* nodeLeft, GraphNode* nodeRight, 
			  FastaRecord::Id edgeId):
		nodeLeft(nodeLeft), nodeRight(nodeRight), 
		edgeId(edgeId), multiplicity(0), selfComplement(false)
		{}

	bool isRepetitive() {return multiplicity > 1;}
	void addSequence(FastaRecord::Id id, int32_t start, int32_t end)
	{
		seqSegments.emplace_back(id, start, end);
		++multiplicity;
	}
	int32_t length()
	{
		int64_t sumLen = 0;
		for (auto& seqSeg : seqSegments)
		{
			sumLen += seqSeg.end - seqSeg.start;
		}
		return sumLen / seqSegments.size();
	}

	GraphNode* nodeLeft;
	GraphNode* nodeRight;

	FastaRecord::Id edgeId;
	std::vector<SequenceSegment> seqSegments;
	int multiplicity;
	bool selfComplement;
};

struct GraphNode
{
	bool isBifurcation() {return outEdges.size() != 1 || inEdges.size() != 1;}
	std::vector<GraphNode*> neighbors()
	{
		std::unordered_set<GraphNode*> result;
		for (auto& edge : inEdges) result.insert(edge->nodeLeft);
		for (auto& edge : outEdges) result.insert(edge->nodeRight);

		return std::vector<GraphNode*>(result.begin(), result.end());
	}

	std::vector<GraphEdge*> inEdges;
	std::vector<GraphEdge*> outEdges;
};

typedef std::vector<GraphEdge*> GraphPath;

class RepeatGraph
{
public:
	RepeatGraph(const SequenceContainer& asmSeqs):
		_asmSeqs(asmSeqs), _nextEdgeId(0)
	{}

	void build();
	void outputDot(const std::string& filename, bool collapseRepeats);

	friend class GraphProcessor;	//temporary
	friend class RepeatResolver;

private:
	struct GluePoint
	{
		GluePoint(size_t id = 0, FastaRecord::Id seqId = FastaRecord::ID_NONE,
				  int32_t position = 0):
			pointId(id), seqId(seqId), position(position) {}

		size_t 	pointId;
		FastaRecord::Id seqId;
		int32_t	position;
	};

	struct RepeatCluster
	{
		RepeatCluster(FastaRecord::Id seqId = FastaRecord::ID_NONE,
					  size_t clusterId = 0, int32_t start = 0,
					  int32_t end = 0):
			seqId(seqId), clusterId(clusterId), start(start), end(end) {}

		FastaRecord::Id seqId;
		size_t clusterId;
		int32_t start;
		int32_t end;
	};

	GraphPath complementPath(const GraphPath& path);

	void getGluepoints(const OverlapContainer& ovlps);
	void getRepeatClusters(const OverlapContainer& ovlps);
	void initializeEdges();
	bool isRepetitive(GluePoint gpLeft, GluePoint gpRight);
	
	const int _maxSeparation = 500;

	const SequenceContainer& _asmSeqs;

	std::unordered_map<FastaRecord::Id, 
					   std::vector<GluePoint>> _gluePoints;
	std::unordered_map<FastaRecord::Id, 
					   std::vector<RepeatCluster>> _repeatClusters;
	std::list<GraphNode> _graphNodes;
	std::list<GraphEdge> _graphEdges;

	size_t _nextEdgeId;
};