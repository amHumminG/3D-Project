#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>
#include <memory>
#include <string>

#define MAX_ELEMENTS 4
#define DEFAULT_EXTENT 100.0f

template<typename T>
class QuadTree
{
private:
	struct Node
	{
		bool isLeaf;
		int depth;											// the depth of the node in the tree
		DirectX::BoundingBox boundingBox;					// the bounding box of the node
		std::unique_ptr<Node> children[4];					// the children of the node (max 4)

		T* elements[MAX_ELEMENTS];							// holds the elements in a leaf node
		DirectX::BoundingBox elementBoxes[MAX_ELEMENTS];    // holds the bounding boxes of the elements in a leaf node
		UINT size;											// the number of elements in the node

	};

	std::unique_ptr<Node> root;

	std::vector<DirectX::BoundingBox> GetChildBoundingBoxes(const DirectX::BoundingBox& parentBox);

	void AddToNode(const T* elementAddress, const DirectX::BoundingBox& boundingBox, std::unique_ptr<Node>& node);
	void CheckNode(const DirectX::BoundingFrustum& frustum, const std::unique_ptr<Node>& node, std::vector<const T*>& foundObjects);


public:
	
	QuadTree() = default;
	QuadTree(float extent);
	~QuadTree() = default;
	QuadTree(const QuadTree& other) = delete;
	QuadTree& operator=(const QuadTree& other) = delete;
	QuadTree(QuadTree&& other) = delete;
	QuadTree& operator=(QuadTree&& other) = delete;

	void initialize(float extent);

	void AddElement(const T* elementAddress, const DirectX::BoundingBox& boundingBox);
	std::vector<const T*> CheckTree(const DirectX::BoundingFrustum& frustum);
};





// PRIVATE FUNCTIONS

// Divides the bounding box of the current node across the x and z axis to create the bounding boxes of the child nodes
template<typename T>
std::vector<DirectX::BoundingBox> QuadTree<T>::GetChildBoundingBoxes(const DirectX::BoundingBox& parentBox)
{
	std::vector<DirectX::BoundingBox> childBoxes;

	DirectX::XMFLOAT3 halfExtents = DirectX::XMFLOAT3(parentBox.Extents.x / 2.0f, parentBox.Extents.y, parentBox.Extents.z / 2.0f);

	DirectX::BoundingBox childBox1 = DirectX::BoundingBox(DirectX::XMFLOAT3(parentBox.Center.x - halfExtents.x, parentBox.Center.y, parentBox.Center.z - halfExtents.z), halfExtents);
	childBoxes.push_back(childBox1);
	DirectX::BoundingBox childBox2 = DirectX::BoundingBox(DirectX::XMFLOAT3(parentBox.Center.x + halfExtents.x, parentBox.Center.y, parentBox.Center.z - halfExtents.z), halfExtents);
	childBoxes.push_back(childBox2);
	DirectX::BoundingBox childBox3 = DirectX::BoundingBox(DirectX::XMFLOAT3(parentBox.Center.x - halfExtents.x, parentBox.Center.y, parentBox.Center.z + halfExtents.z), halfExtents);
	childBoxes.push_back(childBox3);
	DirectX::BoundingBox childBox4 = DirectX::BoundingBox(DirectX::XMFLOAT3(parentBox.Center.x + halfExtents.x, parentBox.Center.y, parentBox.Center.z + halfExtents.z), halfExtents);
	childBoxes.push_back(childBox4);

	return childBoxes;
}

template<typename T>
void QuadTree<T>::AddToNode(const T* elementAddress, const DirectX::BoundingBox& boundingBox, std::unique_ptr<Node>& node)
{
	Node* currentNode = node.get();
	bool collision = currentNode->boundingBox.Intersects(boundingBox);
	std::cerr << "AddToNode()" << std::endl;
	std::cerr << "CurrentNode Depth: " << currentNode->depth << std::endl;

	std::cerr << "CurrentNode Extents: " << currentNode->boundingBox.Extents.x << ", " << currentNode->boundingBox.Extents.y << ", " << currentNode->boundingBox.Extents.z << std::endl;
	std::cerr << "CurrentNode Center: " << currentNode->boundingBox.Center.x << ", " << currentNode->boundingBox.Center.y << ", " << currentNode->boundingBox.Center.z << std::endl;

	std::cerr << "InsertNode Extents: " << boundingBox.Extents.x << ", " << boundingBox.Extents.y << ", " << boundingBox.Extents.z << std::endl;
	std::cerr << "InsertNode Center: " << boundingBox.Center.x << ", " << boundingBox.Center.y << ", " << boundingBox.Center.z << std::endl;


	if (!collision)	// No collission found, node and potential children are not relevant
	{
	std::cerr << "No Collision" << std::endl;
		return;
	}
	std::cerr << "Collision!" << std::endl;

	if (currentNode->isLeaf) // The current node is a leaf node
	{
		std::cerr << "Node is leaf node" << std::endl;
		if (currentNode->size < MAX_ELEMENTS) // Check if the node is not full yet
		{
			std::cerr << "Node is not full" << std::endl;
			// Add the object and volume to the node
			currentNode->elements[currentNode->size] = (T*)elementAddress;
			currentNode->elementBoxes[currentNode->size] = boundingBox;
			currentNode->size++;
			std::cerr << "Added element to node" << std::endl;
			return;
		}
		else // The node is a leaf node, but there is no more room for elements
		{
			std::cerr << "Node is full" << std::endl;
			// Add child nodes to the node based on this node's covered volume
			std::vector<DirectX::BoundingBox> childBoxes = GetChildBoundingBoxes(currentNode->boundingBox);

			// Initialize the child nodes with their respective bounding volumes
			std::cerr << "Initializing child nodes" << std::endl;
			for (int i = 0; i < 4; i++)
			{
				currentNode->children[i] = std::make_unique<Node>();
				Node* childNode = currentNode->children[i].get();
				childNode->boundingBox = childBoxes[i];
				std::cerr << "ChildNode Extents: " << childNode->boundingBox.Extents.x << ", " << childNode->boundingBox.Extents.y << ", " << childNode->boundingBox.Extents.z << std::endl;
				std::cerr << "ChildNode Center: " << childNode->boundingBox.Center.x << ", " << childNode->boundingBox.Center.y << ", " << childNode->boundingBox.Center.z << std::endl;
				childNode->isLeaf = true;
				childNode->depth = currentNode->depth + 1;
				childNode->size = 0;
			}
			currentNode->isLeaf = false;	// make the current node a parent node

			// For each of the currently stored elements in this node, attempt to add them to the new child nodes
			for (int i = 0; i < currentNode->size; i++)
			{
				std::cerr << "Adding parent element " << i << " out of " << currentNode->size << " to child nodes" << std::endl;
				AddToNode(currentNode->elements[i], currentNode->elementBoxes[i], currentNode->children[0]);
				AddToNode(currentNode->elements[i], currentNode->elementBoxes[i], currentNode->children[1]);
				AddToNode(currentNode->elements[i], currentNode->elementBoxes[i], currentNode->children[2]);
				AddToNode(currentNode->elements[i], currentNode->elementBoxes[i], currentNode->children[3]);


				//AddElement(currentNode->elements[i], currentNode->elementBoxes[i]);

				// maybe you could just do one call to root node instead of 4 calls to the children? Probably does the same thing
			}
		}
	}

	std::cerr << "Node is not leaf node" << std::endl;
	std::cerr << "Adding element to child nodes" << std::endl;

	// The current node either was a parent node all along, or it was a full leaf node and turned into a parent node
	// For each of the child nodes of this node, recursively call this function with the same element and volume that
	// were recieved by this function call
	for (int i = 0; i < 4; i++)
	{
		AddToNode(elementAddress, boundingBox, currentNode->children[i]);
	}
}

template<typename T>
void QuadTree<T>::CheckNode(const DirectX::BoundingFrustum& frustum, const std::unique_ptr<Node>& node, std::vector<const T*>& foundObjects)
{
	//std::cerr << "CheckNode()" << std::endl;

	Node* currentNode = node.get();
	bool collision = currentNode->boundingBox.Intersects(frustum);

	//std::cerr << "Depth: " << currentNode->depth << std::endl; 
  
	if (!collision)	// No collision found, node and potential children are not relevant
	{
		return;
	}

	if (currentNode->isLeaf) // The current node is a leaf node
	{
		for (int i = 0; i < currentNode->size; i++)
		{
			// Check if there is a collision between the frustum and the element's bounding volume
			collision = currentNode->elementBoxes[i].Intersects(frustum);

			if (collision)
			{
				// Check if the element is already in the return vector
				if (std::count(foundObjects.begin(), foundObjects.end(), currentNode->elements[i]) == 0)
				{
					// add if not
					foundObjects.push_back(currentNode->elements[i]);
				}
			}
		}
	}
	else
	{
		// Recursively run this function for each of the current node's children
		for (int i = 0; i < 4; i++)
		{
			CheckNode(frustum, currentNode->children[i], foundObjects);
		}
	}
}


// PUBLIC FUNCTIONS

template<typename T>
QuadTree<T>::QuadTree(float extent)
{
	initialize(extent);
}

template<typename T>
void QuadTree<T>::initialize(float extent)
{
	this->root = std::make_unique<Node>();	// Probably not necessary, but just to be sure
	Node* root = this->root.get();

	root->isLeaf = true;
	root->depth = 0;
	root->size = 0;
	root->boundingBox = DirectX::BoundingBox(DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(extent, extent, extent));
}


template<typename T>
void QuadTree<T>::AddElement(const T* elementAddress, const DirectX::BoundingBox& boundingBox)
{
	std::cerr << "AddElement() initializing recursion" << std::endl;
	AddToNode(elementAddress, boundingBox, this->root);
}

template<typename T>
std::vector<const T*> QuadTree<T>::CheckTree(const DirectX::BoundingFrustum& frustum)
{
	std::vector<const T* > toReturn;

	CheckNode(frustum, this->root, toReturn);

	return toReturn;
}