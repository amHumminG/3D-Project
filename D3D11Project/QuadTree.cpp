#include "QuadTree.h"


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
	bool collision = boundingBox->Intersects(currentNode->boundingBox);

	if (!collision)	// No collission found, node and potential children are not relevant
	{
		return;
	}

	if (currentNode->children == nullptr) // The current node is a leaf node
	{
		if (currentNode->elements.size() < MAX_ELEMENTS) // Check if the node is not full yet
		{
			// Add the object and volume to the node
			currentNode->elements.push_back(elementAddress);

			return;
		}
		else // The node is a leaf node, but there is no more room for elements
		{
			// Add child nodes to the node based on this node's covered volume
			std::vector<DirectX::BoundingBox> childBoxes = GetChildBoundingBoxes(currentNode->boundingBox);

			// Initialize the child nodes with their respective bounding volumes
			for (int i = 0; i < 4; i++)
			{
				currentNode->children[i] = std::make_unique<Node>();
				Node* childNode = currentNode->children[i].get();
				childNode->boundingBox = childBoxes[i];

				childNode->children[0] = nullptr;
				childNode->children[1] = nullptr;
				childNode->children[2] = nullptr;
				childNode->children[3] = nullptr;
			}

			// For each of the currently stored elements in this node, attempt to add them to the new child nodes
			for (const T* element : currentNode->elements)
			{
				AddToNode(element, element->GetBoundingBox(), this->root);
			}
		}
	}

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
	Node* currentNode = node.get();
	bool collision = frustum.Intersects(currentNode->boundingBox);

	if (!collision)	// No collision found, node and potential children are not relevant
	{
		return;
	}

	if (currentNode->children == nullptr) // The current node is a leaf node
	{
		for (const T* element : currentNode->elements)
		{
			// Check if there is a collision between the frustum and the element's bounding volume
			collision = frustum.Intersects(element.GetBoundingBox());

			if (collision)
			{
				// Check if the element is already in the return vector
				if (std::count(foundObjects.begin(), foundObjects.end(), element) == 0)
				{
					// add if not
					foundObjects.push_back(element);
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

	root->boundingBox = DirectX::BoundingBox(DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(extent, extent, extent));
	root->children[0] = nullptr;
	root->children[1] = nullptr;
	root->children[2] = nullptr;
	root->children[3] = nullptr;
	root->elements.clear();
}


template<typename T>
void QuadTree<T>::AddElement(const T* elementAddress, const DirectX::BoundingBox& boundingBox)
{
	AddToNode(elementAddress, boundingBox, root);
}

template<typename T>
std::vector<const T*> QuadTree<T>::CheckTree(const DirectX::BoundingFrustum& frustum)
{
	std::vector<const T* > toReturn;

	CheckNode(frustum, root, toReturn);

	return toReturn;
}



