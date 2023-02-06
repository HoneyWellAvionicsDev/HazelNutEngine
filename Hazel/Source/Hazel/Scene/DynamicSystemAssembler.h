#pragma once

#include "Hazel/Math/Matrix.h"

#include "Entity.h"

#include <map>
#include <queue>

namespace Hazel
{
	class Scene;
	//class Entity;

	class DynamicSystemAssembler
	{
	public:
		DynamicSystemAssembler(Scene* scene);

		void LinkBody(Entity focus, Entity target, const glm::dvec2& focusWorld, const glm::dvec2& targetWorld);
		void FixBody(Entity focus, Entity target, const glm::dvec2& world);

		bool GenerateRigidBodies();
		bool GenerateConstraints();
		bool GenerateForceGens();

	private:
		Scene* m_Scene = nullptr;
		std::unordered_multimap<Entity, Entity> m_AdjacencyList;
		std::vector<Entity> m_HandledEntities;
	};
}

#if 0

std::vector<int> findLeadNodes()
{
	std::vector<int> leadNodes;
	std::vector<int> indegrees(adjacencyList.size(), 0);
	for (int node = 0; node < adjacencyList.size(); node++) {
		for (int neighbor : adjacencyList[node]) {
			indegrees[neighbor]++;
		}
	}

	std::queue<int> bfsQueue;
	for (int node = 0; node < adjacencyList.size(); node++) {
		if (indegrees[node] == 0) {
			bfsQueue.push(node);
		}
	}

	while (!bfsQueue.empty()) {
		int currentNode = bfsQueue.front();
		bfsQueue.pop();
		leadNodes.push_back(currentNode);
		for (int neighbor : adjacencyList[currentNode]) {
			indegrees[neighbor]--;
			if (indegrees[neighbor] == 0) {
				bfsQueue.push(neighbor);
			}
		}
	}

	return leadNodes;
}
#endif
/*
Input::
1
6 11
1 2
1 3
1 5
2 1
2 3
2 4
3 4
4 3
5 6
5 4
6 3
1 4

output:
[ 1 ]
[ 1 2 ]
[ 1 3 ]
[ 1 5 ]
[ 1 2 3 ]
The Required path is:: [ 1 2 4 ]
The Required path is:: [ 1 3 4 ]
[ 1 5 6 ]
The Required path is:: [ 1 5 4 ]
The Required path is:: [ 1 2 3 4 ]
[ 1 2 4 3 ]
[ 1 5 6 3 ]
[ 1 5 4 3 ]
The Required path is:: [ 1 5 6 3 4 ]


*/