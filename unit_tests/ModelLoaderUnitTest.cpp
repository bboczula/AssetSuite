#include "CppUnitTest.h"
#include "../source/wavefront/ModelLoader.h"
//#include "../model_loader/ModelLoader.cpp"
#include <vector>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ModelLoaderUnitTest
{
	TEST_CLASS(ProcessLineVertex)
	{
	public:
		TEST_METHOD(BasicVertexLine1)
		{
			std::string line("v 10.927240 2.957242 2.370587");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)0, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
		TEST_METHOD(BasicVertexLine2)
		{
			std::string line("v          10.927240 2.957242 2.370587");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)0, modelLoader->GenNumOfFaces());

			auto vertex = modelLoader->GetVertex(0);
			Assert::AreEqual((float)10.927240, vertex.x);
			Assert::AreEqual((float)2.957242, vertex.y);
			Assert::AreEqual((float)2.370587, vertex.z);
			delete modelLoader;
		}
		TEST_METHOD(BasicVertexLine3)
		{
			std::string line("v        10.927240  2.957242   2.370587 ");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)0, modelLoader->GenNumOfFaces());

			auto vertex = modelLoader->GetVertex(0);
			Assert::AreEqual((float)10.927240, vertex.x);
			Assert::AreEqual((float)2.957242, vertex.y);
			Assert::AreEqual((float)2.370587, vertex.z);
			delete modelLoader;
		}
		TEST_METHOD(BasicVertexLine4)
		{
			std::string line("v 10 2 2");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)0, modelLoader->GenNumOfFaces());

			auto vertex = modelLoader->GetVertex(0);
			Assert::AreEqual((float)10, vertex.x);
			Assert::AreEqual((float)2, vertex.y);
			Assert::AreEqual((float)2, vertex.z);
			delete modelLoader;
		}
		TEST_METHOD(BasicVertexLine5)
		{
			// Found in Sibenik model
			std::string line("v  -0.011057 -15.300000 -3.420000");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)0, modelLoader->GenNumOfFaces());

			auto vertex = modelLoader->GetVertex(0);
			Assert::AreEqual((float)-0.011057, vertex.x);
			Assert::AreEqual((float)-15.3, vertex.y);
			Assert::AreEqual((float)-3.42, vertex.z);
			delete modelLoader;
		}
	};

	TEST_CLASS(ProcessLineFace)
	{
	public:
		TEST_METHOD(BasicFaceLine1)
		{
			// Found in Blender export without texture data
			// Here we have two separators, but no texture UV data
			std::string line("f 1//1 3//1 4//1");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)1, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
		TEST_METHOD(BasicFaceLine2)
		{
			std::string line("f     1//1     3//1     4//1     ");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)1, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
		TEST_METHOD(BasicFaceLine3)
		{
			// Found in Casual Effects in Sponza model
			// Here you have only one separator for each face vertex
			// Additionally, you have four face elements instead of three
			std::string line("f 1/1 2/2 3/3 4/4");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)2, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
		TEST_METHOD(BasicFaceLine4)
		{
			// Found in Blender export
			std::string line("f 2/1/1 3/2/1 1/3/1");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)1, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
		TEST_METHOD(BasicFaceLine5)
		{
			// Found in Blender export
			std::string line("f 190/190 191/191 192/192 193/193 ");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)2, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
		TEST_METHOD(BasicFaceLine6)
		{
			// Found in Blender export
			std::string line("f 804/804 805/805 194/194");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)1, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
		TEST_METHOD(BasicFaceLine7)
		{
			// This is how Blender exports without UVs and normals
			std::string line("f 1 2 4 3");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)2, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
		TEST_METHOD(BasicFaceLine8)
		{
			// Found in Sibenik model
			std::string line("f 3490/1 3488/2 3487/3");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)1, modelLoader->GenNumOfFaces());
			Assert::AreEqual((unsigned int)3490 - 1, modelLoader->GetFace(0).vertexIndex[0]);
			Assert::AreEqual((unsigned int)3488 - 1, modelLoader->GetFace(0).vertexIndex[1]);
			Assert::AreEqual((unsigned int)3487 - 1, modelLoader->GetFace(0).vertexIndex[2]);
			Assert::AreEqual((unsigned int)1 - 1, modelLoader->GetFace(0).textureIndex[0]);
			Assert::AreEqual((unsigned int)2 - 1, modelLoader->GetFace(0).textureIndex[1]);
			Assert::AreEqual((unsigned int)3 - 1, modelLoader->GetFace(0).textureIndex[2]);
			delete modelLoader;
		}
		TEST_METHOD(BasicFaceLine9)
		{
			// Found in Sibenik model
			std::string line("f 1/1 2/2 3/3 4/4");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());
			Assert::AreEqual((unsigned int)2, modelLoader->GenNumOfFaces());
			Assert::AreEqual((unsigned int)1 - 1, modelLoader->GetFace(0).vertexIndex[0]);
			Assert::AreEqual((unsigned int)2 - 1, modelLoader->GetFace(0).vertexIndex[1]);
			Assert::AreEqual((unsigned int)3 - 1, modelLoader->GetFace(0).vertexIndex[2]);
			Assert::AreEqual((unsigned int)1 - 1, modelLoader->GetFace(0).textureIndex[0]);
			Assert::AreEqual((unsigned int)2 - 1, modelLoader->GetFace(0).textureIndex[1]);
			Assert::AreEqual((unsigned int)3 - 1, modelLoader->GetFace(0).textureIndex[2]);
			delete modelLoader;
		}
	};

	TEST_CLASS(ProcessLineGroup)
	{
	public:
		TEST_METHOD(BasicGroupLine1)
		{
			std::vector<std::string> lines;
			lines.push_back("g arcs_floor");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)0, modelLoader->GetGroupOffset("arcs_floor"));
		}

		TEST_METHOD(BasicGroupLine2)
		{
			std::vector<std::string> lines;
			lines.push_back("v -1.000000 0.000000 1.000000");
			lines.push_back("v 1.000000 0.000000 1.000000");
			lines.push_back("v -1.000000 0.000000 -1.000000");
			lines.push_back("v 1.000000 0.000000 -1.000000");
			lines.push_back("f 1 2 4 3");
			lines.push_back("g arcs_floor");
			lines.push_back("f 5 6 8 7");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)1, modelLoader->GetNumOfGroups());
			Assert::AreEqual((unsigned int)2, modelLoader->GetGroupOffset("arcs_floor"));
		}
		TEST_METHOD(BasicGroupLine3)
		{
			std::vector<std::string> lines;
			lines.push_back("v -1.000000 0.000000 1.000000");
			lines.push_back("v 1.000000 0.000000 1.000000");
			lines.push_back("v -1.000000 0.000000 -1.000000");
			lines.push_back("v 1.000000 0.000000 -1.000000");
			lines.push_back("f 1 2 4 3");
			lines.push_back("g arcs_floor");
			lines.push_back("f 5 6 8 7");
			lines.push_back("g arcs_main");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)2, modelLoader->GetNumOfGroups());
			Assert::AreEqual((unsigned int)4, modelLoader->GetGroupOffset("arcs_main"));
		}
		TEST_METHOD(BasicGroupLine4)
		{
			std::vector<std::string> lines;
			lines.push_back("g arcs_floor");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual(true, modelLoader->HasGroup("arcs_floor"));
			Assert::AreEqual(false, modelLoader->HasGroup("Arcs_floor"));
		}
	};

	TEST_CLASS(ProcessLineTexture)
	{
	public:
		TEST_METHOD(BasicTextureLine1)
		{
			// Found in the Sponza orginal model
			std::string line("vt 0.755376 0.571389 0.000000");
			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			modelLoader->ProcessLine((char*)line.c_str());

			auto textureCoord = modelLoader->GetTextureCoord(0);
			Assert::AreEqual((float)0.755376f, textureCoord.x);
			Assert::AreEqual((float)0.571389f, textureCoord.y);
			delete modelLoader;
		}
	};

	TEST_CLASS(Triangulation)
	{
	public:
		TEST_METHOD(SingleTriangle)
		{
			std::vector<std::string> lines;
			lines.push_back("v -1.000000 0.000000 1.000000");
			lines.push_back("v 1.000000 0.000000 1.000000");
			lines.push_back("v -1.000000 0.000000 -1.000000");
			lines.push_back("v 1.000000 0.000000 -1.000000");
			lines.push_back("f 1 2 4 3");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)2, modelLoader->GenNumOfFaces());

			auto face1 = modelLoader->GetFace(0);
			Assert::AreEqual((unsigned int)0, face1.vertexIndex[0]);
			Assert::AreEqual((unsigned int)1, face1.vertexIndex[1]);
			Assert::AreEqual((unsigned int)3, face1.vertexIndex[2]);

			auto face2 = modelLoader->GetFace(1);
			Assert::AreEqual((unsigned int)0, face2.vertexIndex[0]);
			Assert::AreEqual((unsigned int)3, face2.vertexIndex[1]);
			Assert::AreEqual((unsigned int)2, face2.vertexIndex[2]);
			delete modelLoader;
		}
	};

	TEST_CLASS(Primitive)
	{
	public:
		TEST_METHOD(SingleTriangle)
		{
			std::vector<std::string> lines;
			lines.push_back("v 2.000000 -2.000000 -2.000000");
			lines.push_back("v 2.000000 2.000000 -2.000000");
			lines.push_back("v 2.000000 -2.000000 2.000000");
			lines.push_back("v 2.000000 2.000000 2.000000");
			lines.push_back("f 1/1 2/2 4/3");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)1, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}

		TEST_METHOD(TwoTriangles)
		{
			std::vector<std::string> lines;
			lines.push_back("v 2.000000 -2.000000 -2.000000");
			lines.push_back("v 2.000000 2.000000 -2.000000");
			lines.push_back("v 2.000000 -2.000000 2.000000");
			lines.push_back("v 2.000000 2.000000 2.000000");
			lines.push_back("f 1/1 2/2 4/3 3/4");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)2, modelLoader->GenNumOfFaces());
			delete modelLoader;
		}
	};

	TEST_CLASS(ModelLoaderUnitTest)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			std::vector<std::string> testData;
			testData.push_back("v  10.927240 2.957242 2.370587");
			testData.push_back("v  10.927240 3.238548 2.282929");
			testData.push_back("v  10.927240 3.149897 2.049175");
			testData.push_back("v  10.927240 2.897413 2.127851");
			testData.push_back("v  11.587240 3.149897 2.049175");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < testData.size(); i++)
			{
				modelLoader->ProcessLine((char*)testData[i].c_str());
			}

			Assert::AreEqual((unsigned int)0, modelLoader->GenNumOfFaces());

			delete modelLoader;
		}
	};
	TEST_CLASS(GroupSize)
	{
		TEST_METHOD(BasicGroupSize1)
		{
			std::vector<std::string> lines;
			lines.push_back("v -1.000000 0.000000 1.000000");
			lines.push_back("v 1.000000 0.000000 1.000000");
			lines.push_back("v -1.000000 0.000000 -1.000000");
			lines.push_back("v 1.000000 0.000000 -1.000000");
			lines.push_back("g arcs_floor");
			lines.push_back("f 1 2 4 3");
			lines.push_back("f 5 6 8 7");
			lines.push_back("g arcs_main");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)4, modelLoader->GetGroupSize("arcs_floor"));
			Assert::AreEqual((unsigned int)0, modelLoader->GetGroupSize("arcs_main"));
		}
		TEST_METHOD(BasicGroupSize2)
		{
			std::vector<std::string> lines;
			lines.push_back("v -1.000000 0.000000 1.000000");
			lines.push_back("v 1.000000 0.000000 1.000000");
			lines.push_back("v -1.000000 0.000000 -1.000000");
			lines.push_back("v 1.000000 0.000000 -1.000000");
			lines.push_back("g arcs_floor");
			lines.push_back("f 1 2 4 3");
			lines.push_back("f 5 6 8 7");
			lines.push_back("g arcs_main");
			lines.push_back("f 1 2 4 3");
			lines.push_back("f 5 6 8 7");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)4, modelLoader->GetGroupSize("arcs_floor"));
			Assert::AreEqual((unsigned int)4, modelLoader->GetGroupSize("arcs_main"));
		}
		TEST_METHOD(BasicGroupSize3)
		{
			std::vector<std::string> lines;
			lines.push_back("v -1.000000 0.000000 1.000000");
			lines.push_back("v 1.000000 0.000000 1.000000");
			lines.push_back("v -1.000000 0.000000 -1.000000");
			lines.push_back("v 1.000000 0.000000 -1.000000");
			lines.push_back("g arcs_floor");
			lines.push_back("f 1 2 4");
			lines.push_back("f 5 6 8");
			lines.push_back("g arcs_main");
			lines.push_back("f 1 2 4");
			lines.push_back("f 5 6 8");

			AssetSuite::ModelLoader* modelLoader = new AssetSuite::ModelLoader;
			for (int i = 0; i < lines.size(); i++)
			{
				modelLoader->ProcessLine((char*)lines[i].c_str());
			}

			Assert::AreEqual((unsigned int)2, modelLoader->GetGroupSize("arcs_floor"));
			Assert::AreEqual((unsigned int)2, modelLoader->GetGroupSize("arcs_main"));
		}
	};
}
