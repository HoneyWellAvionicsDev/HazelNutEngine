#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <numeric>
#include <algorithm>
#include <exception>
#include <functional>

namespace Enyoo
{
	struct TreeNode
	{
		using TreeNodePtr = std::unique_ptr<TreeNode>;

		TreeNode() : Index(-1), Axis(-1)
		{
			Next[0] = nullptr;
			Next[1] = nullptr;
		}

		size_t Index;
		TreeNodePtr Next[2];
		uint32_t Axis;
	};

	using TreeNodePtr = TreeNode::TreeNodePtr;

	template<class T, class Compare = std::less<T>>
	class BoundedPriorityQueue
	{
	public:
		BoundedPriorityQueue() = delete;
		BoundedPriorityQueue(size_t bound) : m_Bound(bound) { m_Elements.reserve(bound + 1); }

		void Push(const T& value);

		const T& Back() const { return m_Elements.back(); }
		const T& operator[](size_t index) const { return m_Elements[index]; }
		constexpr size_t Size() const { return m_Elements.size(); }

	private:
		size_t m_Bound;
		std::vector<T> m_Elements;
	};

	template<class T>
	class KDTree
	{
	public:
		using KnQueue = BoundedPriorityQueue<std::pair<double, size_t>>;

		KDTree() : m_Root(nullptr) {}
		KDTree(const std::vector<T>& points) : m_Root(nullptr) { Build(points); }
		~KDTree() { Clear(); }

		void Build(const std::vector<T>& points);
		void Clear();
		bool Validate() const;

		size_t NearestNeighborSearch(const T& query, double* minDistance = nullptr) const;
		std::vector<size_t> KNearestSearch(const T& query, uint32_t k) const;
		std::vector<size_t> RadiusSearch(const T& query, double radius) const;

	private:
		TreeNodePtr BuildRecursive(size_t* indices, uint32_t numPoints, uint32_t depth);
		void ClearRecursive(TreeNode* node);
		void ValidateRecursive(const TreeNode* node, uint32_t depth) const;
		void NnSearchRecursive(const T& query, const TreeNode* node, size_t* guess, double* minDistance) const;
		void KnSearchRecursive(const T& query, const TreeNode* node, KnQueue& queue, uint32_t k) const;
		void RadiusSearchRecursive(const T& query, const TreeNode* node, std::vector<size_t>& indices, double radius) const;

		static double Distance(const T& p, const T& q);

		class Exception : public std::exception { using std::exception::exception; };

	private:
		TreeNodePtr m_Root;
		std::vector<T> m_Points;
	};

	template<class T, class Compare>
	inline void BoundedPriorityQueue<T, Compare>::Push(const T& value)
	{
		auto it = std::find_if(m_Elements.begin(), m_Elements.end(),
			[&](const T& element) { return Compare()(value, element); });

		m_Elements.insert(it, value);

		if (m_Elements.size() > m_Bound)
			m_Elements.resize(m_Bound);
	}

	template<class T>
	inline void KDTree<T>::Build(const std::vector<T>& points)
	{
		Clear();

		m_Points = points;
		
		std::vector<size_t> indices(points.size());
		std::iota(std::begin(indices), std::end(indices), 0);

		m_Root = BuildRecursive(indices.data(), static_cast<uint32_t>(points.size()), 0);
	}

	template<class T>
	inline void KDTree<T>::Clear()
	{
		ClearRecursive(m_Root.get());
		m_Root = nullptr;
		m_Points.clear();
	}

	template<class T>
	inline bool KDTree<T>::Validate() const
	{
		try
		{
			ValidateRecursive(m_Root.get(), 0);
		}
		catch (const Exception&)
		{
			return false;
		}

		return true;
	}

	template<class T>
	inline size_t KDTree<T>::NearestNeighborSearch(const T& query, double* minimumDistance) const
	{
		size_t guess;
		double minDistance = std::numeric_limits<double>::max();

		NnSearchRecursive(query, m_Root.get(), &guess, &minDistance);

		if (minimumDistance)
			*minimumDistance = minDistance;

		return guess;
	}

	template<class T>
	inline std::vector<size_t> KDTree<T>::KNearestSearch(const T& query, uint32_t k) const
	{
		KnQueue queue(k);
		KnSearchRecursive(query, m_Root.get(), queue, k);

		std::vector<size_t> indices(queue.Size());
		for (size_t i = 0; i < queue.Size(); i++)
			indices[i] = queue[i].second;

		return indices;
	}

	template<class T>
	inline std::vector<size_t> KDTree<T>::RadiusSearch(const T& query, double radius) const
	{
		std::vector<size_t> indices;
		RadiusSearchRecursive(query, m_Root.get(), indices, radius);
		return indices;
	}

	template<class T>
	inline TreeNodePtr KDTree<T>::BuildRecursive(size_t* indices, uint32_t numPoints, uint32_t depth)
	{
		if (numPoints <= 0)
			return nullptr;

		const uint32_t axis = depth % T::DIM;
		const uint32_t mid = (numPoints - 1) / 2;

		std::nth_element(indices, indices + mid, indices + numPoints, [&](size_t lhs, size_t rhs)
		{
			return m_Points[lhs][axis] < m_Points[rhs][axis];
		});

		TreeNodePtr node = std::make_unique<TreeNode>();
		node->Index = indices[mid];
		node->Axis = axis;

		node->Next[0] = BuildRecursive(indices, mid, depth + 1);
		node->Next[1] = BuildRecursive(indices + mid + 1, numPoints - mid - 1, depth + 1);
		
		return node;
	}

	template<class T>
	inline void KDTree<T>::ClearRecursive(TreeNode* node)
	{
		if (!node)
			return;

		if (node->Next[0].get())
			ClearRecursive(node->Next[0].get());

		if (node->Next[1].get())
			ClearRecursive(node->Next[1].get());
	}

	template<class T>
	inline void KDTree<T>::ValidateRecursive(const TreeNode* node, uint32_t depth) const
	{
		if (!node)
			return;

		const uint32_t axis = node->Axis;
		const TreeNode* node0 = node->Next[0].get();
		const TreeNode* node1 = node->Next[1].get();

		if (node0 && node1)
		{
			if (m_Points[node->Index][axis] < m_Points[node0->Index][axis])
				throw Exception();

			if (m_Points[node->Index][axis] > m_Points[node1->Index][axis])
				throw Exception();

			if (node0)
				ValidateRecursive(node0, depth + 1);

			if (node1)
				ValidateRecursive(node1, depth + 1);
		}
	}

	template<class T>
	inline void KDTree<T>::NnSearchRecursive(const T& query, const TreeNode* node, size_t* guess, double* minDistance) const
	{
		if (!node)
			return;

		const T& train = m_Points[node->Index];

		const double distance = Distance(query, train);
		if (distance < *minDistance)
		{
			*minDistance = distance;
			*guess = node->Index;
		}

		const uint32_t axis = node->Axis;
		const uint32_t direction = query[axis] < train[axis] ? 0 : 1;
		NnSearchRecursive(query, node->Next[direction].get(), guess, minDistance);

		const double difference = glm::abs(query[axis] - train[axis]);
		if (difference < *minDistance)
			NnSearchRecursive(query, node->Next[!direction].get(), guess, minDistance);
	}

	template<class T>
	inline void KDTree<T>::KnSearchRecursive(const T& query, const TreeNode* node, KnQueue& queue, uint32_t k) const
	{
		if (!node)
			return;

		const T& train = m_Points[node->Index];

		const double distance = Distance(query, train);
		queue.Push(std::make_pair(distance, node->Index));

		const uint32_t axis = node->Axis;
		const uint32_t direction = query[axis] < train[axis] ? 0 : 1;
		KnSearchRecursive(query, node->Next[direction].get(), queue, k);

		const double difference = glm::abs(query[axis] - train[axis]);
		if (static_cast<uint32_t>(queue.Size()) < k || difference < queue.Back().first)
			KnSearchRecursive(query, node->Next[!direction].get(), queue, k);
	}

	template<class T>
	inline void KDTree<T>::RadiusSearchRecursive(const T& query, const TreeNode* node, std::vector<size_t>& indices, double radius) const
	{
		if (!node)
			return;

		const T& train = m_Points[node->Index];

		const double distance = Distance(query, train);
		if (distance < radius)
			indices.push_back(node->Index);

		const uint32_t axis = node->Axis;
		const uint32_t direction = query[axis] < train[axis] ? 0 : 1;

		RadiusSearchRecursive(query, node->Next[direction].get(), indices, radius);

		const double difference = glm::abs(query[axis] - train[axis]);
		if (difference < radius)
			RadiusSearchRecursive(query, node->Next[!direction].get(), indices, radius);
	}

	template<class T>
	inline double KDTree<T>::Distance(const T& p, const T& q)
	{
		double distance = 0;
		for (size_t i = 0; i < T::DIM; i++)
			distance += (p[i] - q[i]) * (p[i] - q[i]);

		return glm::sqrt(distance);
	}
}
