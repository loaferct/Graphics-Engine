#include"Model.h"

Model::Model(const char* file)
{
	// Make a JSON object
	std::string text = get_file_contents(file);
	JSON = json::parse(text);

	// Get the binary data
	Model::file = file;
	data = getData();

	// Traverse all nodes
	traverseNode(0);
}

void Model::Draw(Shader& shader, Camera& camera)
{
	// Go over all meshes and draw each one
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i].Mesh::Draw(shader, camera, matricesMeshes[i]);
	}
}

void Model::loadMesh(unsigned int indMesh)
{
	// Get all accessor indices
	unsigned int posAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["POSITION"];
	unsigned int normalAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["NORMAL"];
	unsigned int texAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
	unsigned int indAccInd = JSON["meshes"][indMesh]["primitives"][0]["indices"];

	// Use accessor indices to get all vertices components
	std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
	std::vector<glm::vec3> positions = groupFloatsVec3(posVec);
	std::vector<float> normalVec = getFloats(JSON["accessors"][normalAccInd]);
	std::vector<glm::vec3> normals = groupFloatsVec3(normalVec);
	std::vector<float> texVec = getFloats(JSON["accessors"][texAccInd]);
	std::vector<glm::vec2> texUVs = groupFloatsVec2(texVec);

	// Combine all the vertex components and also get the indices and textures
	std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);
	std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);
	std::vector<Texture> textures = getTextures();

	glm::vec3 minVertex = findMinVertex(vertices); 	//calculates the minimum vertex so that we can do calculation of the voxel soze etc
	glm::vec3 maxVertex = findMaxVertex(vertices);

	glm::vec3 size = maxVertex - minVertex;

	float voxelSize = 0.01f;

	// Number of voxels in each dimension
	std::vector<Vertex> voxelVertices;
    std::vector<GLuint> voxelIndices;

	int voxX = ceil(size.x / voxelSize);
	int voxY = ceil(size.y / voxelSize);
	int voxZ = ceil(size.z / voxelSize);

	std::cout<<"x"<<voxX<<"y"<<voxY<<"z"<<voxZ;
	for (int x = 0; x < voxX; x++)
	for (int y = 0; y < voxY; y++)

	for (int z = 0; z < voxZ; z++)
	{
		glm::vec3 voxelPos = minVertex + glm::vec3(x, y, z) * voxelSize;

		if (triangleIntersectsVoxel(vertices, indices, voxelPos, voxelSize))
		{
			addCube(voxelPos, voxelVertices, voxelIndices, voxelSize);
			
		}
	}



	meshes.push_back(Mesh(voxelVertices, voxelIndices, textures));
}

void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
	// Current node
	json node = JSON["nodes"][nextNode];

	// Get translation if it exists
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	if (node.find("translation") != node.end())
	{
		float transValues[3];
		for (unsigned int i = 0; i < node["translation"].size(); i++)
			transValues[i] = (node["translation"][i]);
		translation = glm::make_vec3(transValues);
	}
	// Get quaternion if it exists
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	if (node.find("rotation") != node.end())
	{
		float rotValues[4] =
		{
			node["rotation"][3],
			node["rotation"][0],
			node["rotation"][1],
			node["rotation"][2]
		};
		rotation = glm::make_quat(rotValues);
	}
	// Get scale if it exists
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	if (node.find("scale") != node.end())
	{
		float scaleValues[3];
		for (unsigned int i = 0; i < node["scale"].size(); i++)
			scaleValues[i] = (node["scale"][i]);
		scale = glm::make_vec3(scaleValues);
	}
	// Get matrix if it exists
	glm::mat4 matNode = glm::mat4(1.0f);
	if (node.find("matrix") != node.end())
	{
		float matValues[16];
		for (unsigned int i = 0; i < node["matrix"].size(); i++)
			matValues[i] = (node["matrix"][i]);
		matNode = glm::make_mat4(matValues);
	}

	// Initialize matrices
	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	// Use translation, rotation, and scale to change the initialized matrices
	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	// Multiply all matrices together
	glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

	// Check if the node contains a mesh and if it does load it
	if (node.find("mesh") != node.end())
	{
		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		loadMesh(node["mesh"]);
	}

	// Check if the node has children, and if it does, apply this function to them with the matNextNode
	if (node.find("children") != node.end())
	{
		for (unsigned int i = 0; i < node["children"].size(); i++)
			traverseNode(node["children"][i], matNextNode);
	}
}

std::vector<unsigned char> Model::getData()
{
	// Create a place to store the raw text, and get the uri of the .bin file
	std::string bytesText;
	std::string uri = JSON["buffers"][0]["uri"];

	// Store raw text data into bytesText
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
	bytesText = get_file_contents((fileDirectory + uri).c_str());

	// Transform the raw text data into bytes and put them in a vector
	std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
	return data;
}

std::vector<float> Model::getFloats(json accessor)
{
	std::vector<float> floatVec;

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 1);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	std::string type = accessor["type"];

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Interpret the type and store it into numPerVert
	unsigned int numPerVert;
	if (type == "SCALAR") numPerVert = 1;
	else if (type == "VEC2") numPerVert = 2;
	else if (type == "VEC3") numPerVert = 3;
	else if (type == "VEC4") numPerVert = 4;
	else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

	// Go over all the bytes in the data at the correct place using the properties from above
	unsigned int beginningOfData = byteOffset + accByteOffset;
	unsigned int lengthOfData = count * 4 * numPerVert;
	for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i += 4)
	{
		unsigned char bytes[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
		float value;
		std::memcpy(&value, bytes, sizeof(float));
		floatVec.push_back(value);
	}

	return floatVec;
}

std::vector<GLuint> Model::getIndices(json accessor)
{
	std::vector<GLuint> indices;

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor["componentType"];

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Get indices with regards to their type: unsigned int, unsigned short, or short
	unsigned int beginningOfData = byteOffset + accByteOffset;
	if (componentType == 5125)
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 4; i += 4)
		{
			unsigned char bytes[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
			unsigned int value;
			std::memcpy(&value, bytes, sizeof(unsigned int));
			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5123)
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i += 2)
		{
			unsigned char bytes[] = { data[i], data[i + 1] };
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short));
			indices.push_back((GLuint)value);
		}
	}
	else if (componentType == 5122)
	{
		for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i += 2)
		{
			unsigned char bytes[] = { data[i], data[i + 1] };
			short value;
			std::memcpy(&value, bytes, sizeof(short));
			indices.push_back((GLuint)value);
		}
	}

	return indices;
}

std::vector<Texture> Model::getTextures()
{
	std::vector<Texture> textures;

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	// Go over all images
	for (unsigned int i = 0; i < JSON["images"].size(); i++)
	{
		// uri of current texture
		std::string texPath = JSON["images"][i]["uri"];

		// Check if the texture has already been loaded
		bool skip = false;
		for (unsigned int j = 0; j < loadedTexName.size(); j++)
		{
			if (loadedTexName[j] == texPath)
			{
				textures.push_back(loadedTex[j]);
				skip = true;
				break;
			}
		}

		// If the texture has been loaded, skip this
		if (!skip)
		{
			// Load diffuse texture
			if (texPath.find("baseColor") != std::string::npos)
			{
				Texture diffuse = Texture((fileDirectory + texPath).c_str(), "diffuse", loadedTex.size());
				textures.push_back(diffuse);
				loadedTex.push_back(diffuse);
				loadedTexName.push_back(texPath);
			}
			// Load specular texture
			else if (texPath.find("metallicRoughness") != std::string::npos)
			{
				Texture specular = Texture((fileDirectory + texPath).c_str(), "specular", loadedTex.size());
				textures.push_back(specular);
				loadedTex.push_back(specular);
				loadedTexName.push_back(texPath);
			}
		}
	}

	return textures;
}

std::vector<Vertex> Model::assembleVertices
(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	std::vector<glm::vec2> texUVs
)
{
	std::vector<Vertex> vertices;
	for (int i = 0; i < positions.size(); i++)
	{
		vertices.push_back
		(
			Vertex
			{
				positions[i],
				normals[i],
				glm::vec3(1.0f, 1.0f, 1.0f),
				texUVs[i]
			}
		);
	}
	return vertices;
}

std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec)
{
	const unsigned int floatsPerVector = 2;
	
	std::vector<glm::vec2> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
	{
		vectors.push_back(glm::vec2(0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++)
		{
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}
std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec)
{
	const unsigned int floatsPerVector = 3;

	std::vector<glm::vec3> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
	{
		vectors.push_back(glm::vec3(0, 0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++)
		{
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}
std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec)
{
	const unsigned int floatsPerVector = 4;

	std::vector<glm::vec4> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
	{
		vectors.push_back(glm::vec4(0, 0, 0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++)
		{
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}

glm::vec3 findMinVertex(const std::vector<Vertex> vertices)
{

    glm::vec3 minV(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    );

    for (const auto& v : vertices)
    {
        if (v.position.x < minV.x) minV.x = v.position.x;
        if (v.position.y < minV.y) minV.y = v.position.y;
        if (v.position.z < minV.z) minV.z = v.position.z;
    }

    return minV;
}

glm::vec3 findMaxVertex(const std::vector<Vertex> vertices)
{

    glm::vec3 maxV(
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min()
    );

    for (const auto& v : vertices)
    {
        if (v.position.x > maxV.x) maxV.x = v.position.x;
        if (v.position.y > maxV.y) maxV.y = v.position.y;
        if (v.position.z > maxV.z) maxV.z = v.position.z;
    }

    return maxV;
}


bool triangleIntersectsVoxel(
    const std::vector<Vertex>& vertices,
    const std::vector<GLuint>& indices,
    const glm::vec3& voxelPos,
    float voxelSize
) {
    glm::vec3 voxelMax = voxelPos + glm::vec3(voxelSize);

    for (size_t i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0 = vertices[indices[i + 0]].position;
        glm::vec3 v1 = vertices[indices[i + 1]].position;
        glm::vec3 v2 = vertices[indices[i + 2]].position;

        glm::vec3 triMin(
            std::min({v0.x, v1.x, v2.x}),
            std::min({v0.y, v1.y, v2.y}),
            std::min({v0.z, v1.z, v2.z})
        );

        glm::vec3 triMax(
            std::max({v0.x, v1.x, v2.x}),
            std::max({v0.y, v1.y, v2.y}),
            std::max({v0.z, v1.z, v2.z})
        );

        // AABB overlap test
        if ((triMax.x >= voxelPos.x && triMin.x <= voxelMax.x) &&
            (triMax.y >= voxelPos.y && triMin.y <= voxelMax.y) &&
            (triMax.z >= voxelPos.z && triMin.z <= voxelMax.z)) {
            return true;
        }
    }
    return false;
}

void addCube(
    const glm::vec3& voxelPos,
    std::vector<Vertex>& voxelVertices,
    std::vector<GLuint>& voxelIndices,
    float voxelSize
) {
    glm::vec3 corners[8] = {
        voxelPos,
        voxelPos + glm::vec3(voxelSize, 0, 0),
        voxelPos + glm::vec3(voxelSize, voxelSize, 0),
        voxelPos + glm::vec3(0, voxelSize, 0),
        voxelPos + glm::vec3(0, 0, voxelSize),
        voxelPos + glm::vec3(voxelSize, 0, voxelSize),
        voxelPos + glm::vec3(voxelSize, voxelSize, voxelSize),
        voxelPos + glm::vec3(0, voxelSize, voxelSize)
    };
    GLuint cubeInds[36] = {
        0,1,2, 2,3,0, // front
        1,5,6, 6,2,1, // right
        5,4,7, 7,6,5, // back
        4,0,3, 3,7,4, // left
        3,2,6, 6,7,3, // top
        4,5,1, 1,0,4  // bottom
    };
    GLuint baseIndex = voxelVertices.size();
    for (int i = 0; i < 8; i++){
        voxelVertices.push_back(Vertex{ corners[i], glm::vec3(0,0,1), glm::vec3(1,1,1), glm::vec2(0,0) });
	}

    for (int i = 0; i < 36; i++){
        voxelIndices.push_back(baseIndex + cubeInds[i]);
	}
}