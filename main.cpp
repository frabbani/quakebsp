
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "lodepng.h"
#include "vector.h"

template <typename T>
struct BinarySearch {
  static int indexOf(const std::vector<T>& vec, const T& value) {
    int l = 0;
    int r = static_cast<int>(vec.size() - 1);
    while (l <= r) {
      int m = l + (r - l) / 2;  // prevent (unlikely) overflow
      if (vec[m] == value) return m;
      if (vec[m] < value)
        l = m + 1;
      else
        r = m - 1;
    }
    return -1;
    //  auto it = std::lower_bound(vec.begin(), vec.end(), value);
    // if (it != vec.end() && *it == value) {
    //   return static_cast<int>(std::distance(vec.begin(), it));
    // }
    // return -1;
  }
};

struct FileData {
  std::vector<Uint8> data;
  FileData(std::string_view filename) {
    FILE* file = std::fopen(filename.data(), "rb");
    if (!file) {
      std::perror("Failed to open file");
      std::exit(EXIT_FAILURE);
    }
    std::fseek(file, 0, SEEK_END);
    std::size_t size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    data.resize(size);
    if (std::fread(data.data(), 1, size, file) != size) {
      std::perror("Failed to read file");
      std::exit(EXIT_FAILURE);
    }
    std::fclose(file);
  }
};

void fileWrite(std::string_view filename, std::string_view str) {
  FILE* file = std::fopen(filename.data(), "w");
  if (!file) {
    std::perror("Failed to open file for writing");
    std::exit(EXIT_FAILURE);
  }
  if (std::fprintf(file, "%s", str.data()) < 0) {
    std::perror("Failed to write file");
    std::exit(EXIT_FAILURE);
  }
  std::fclose(file);
}

void exportStr(std::string_view filename, std::string_view content) {
  FILE* file = std::fopen(filename.data(), "w");
  if (!file) {
    std::perror("Failed to open file for writing");
    std::exit(EXIT_FAILURE);
  }
  if (std::fwrite(content.data(), 1, content.size(), file) != content.size()) {
    std::perror("Failed to write file");
    std::exit(EXIT_FAILURE);
  }
  std::fclose(file);
}

namespace bsp {
#pragma pack(push, 1)
// Quake BSP Reader
struct Entry {
  Uint32 offset;
  Uint32 size;
};

struct Header {
  Uint32 version;
  Entry entities;
  Entry planes;
  Entry miptex;
  Entry vertices;
  Entry visilist;
  Entry nodes;
  Entry texinfo;
  Entry faces;
  Entry lightmaps;
  Entry clipnodes;
  Entry leaves;
  Entry facesList;
  Entry edges;
  Entry edgesList;
  Entry models;
};

/*
plane types:
    0: Axial plane, in X
    1: Axial plane, in Y
    2: Axial plane, in Z
    3: Non axial plane, roughly toward X
    4: Non axial plane, roughly toward Y
    5: Non axial plane, roughly toward Z
*/

struct Plane {
  Vector3 normal;
  float dist;
  int type;
};

struct BoundBox {
  Vector3 min;
  Vector3 max;
};

struct Model {
  BoundBox bound;
  Vector3 origin;
  Uint32 nodeIds[4];
  Uint32 numLeaves;
  Uint32 faceId;
  Uint32 faceNum;
};

struct MipHeader {
  Uint32 numTex;
  Uint32 offsets[];
};

struct MipTex {
  char name[16];
  Uint32 width;
  Uint32 height;
  Uint32 offsets[4];
};

struct Surface {
  Vector3 vectorS;
  float distS;
  Vector3 vectorT;
  float distT;
  Uint32 textureId;
  Uint32 animated;
};

struct Edge {
  Uint16 vertexIds[2];
};

struct Face {
  Uint16 planeId;
  Uint16 side;
  Sint32 ledgeId;
  Uint16 ledgeNum;
  Uint16 textureId;
  Uint8 typeLight;
  Uint8 baseLight;
  Uint8 lights[2];
  Sint32 lightmap;
};

struct Lmap {
  Sint32 faceId;
  float mins[2];
  float maxs[2];
  int bmins[2];
  int bmaxs[2];
  float tmins[2];
  int size[2];
  RGBA* data;
  int w;
  int h;
  int atlasX;
  int atlasY;

  void init(int faceId) {
    this->faceId = faceId;
    mins[0] = mins[1] = FLT_MAX;
    maxs[0] = maxs[1] = -FLT_MAX;
  }
  void addST(float s, float t) {
    mins[0] = std::min(mins[0], s);
    mins[1] = std::min(mins[1], t);
    maxs[0] = std::max(maxs[0], s);
    maxs[1] = std::max(maxs[1], t);
  }
  void calc() {
    int exts[2];
    for (int i = 0; i < 2; ++i) {
      bmins[i] = static_cast<int>(std::floor(mins[i]) / 16.0f);
      bmaxs[i] = static_cast<int>(std::ceil(maxs[i]) / 16.0f);
      tmins[i] = static_cast<float>(bmins[i]) * 16.0f;
      exts[i] = (bmaxs[i] - bmins[i]) * 16;
    }
    w = exts[0] / 16 + 1;
    h = exts[1] / 16 + 1;
  }
  Vector2 getUV(float s, float t) {
    float u = (s - tmins[0]) / 16.0f + 0.5f;
    float v = (t - tmins[1]) / 16.0f + 0.5f;
    return {u, v};
  }
  int operator<(const Lmap& rhs) const {
    if (h != rhs.h) {
      return rhs.h - h;
    }
    if (w != rhs.w) {
      return rhs.w - w;
    }
    return 0;
  }
};

std::unique_ptr<std::vector<RGB>> palette = nullptr;

void initPalette() {
  FileData fd("palette.lmp");
  if (fd.data.empty()) {
    std::perror("Failed to read palette");
    std::exit(EXIT_FAILURE);
  }
  if (fd.data.size() != (256 * 3)) {
    std::perror("Invalid palette size");
    std::exit(EXIT_FAILURE);
  }
  palette = std::make_unique<std::vector<RGB>>(256);
  for (size_t i = 0; i < 256; ++i) {
    (*palette)[i].r = fd.data[i * 3];
    (*palette)[i].g = fd.data[i * 3 + 1];
    (*palette)[i].b = fd.data[i * 3 + 2];
  }
}

RGB getColor(size_t index) {
  if (!palette || index >= palette->size()) {
    return {255, 0, 255};
  }
  return (*palette)[index];
}

#pragma pack(pop)
}  // namespace bsp

namespace obj {
struct FaceData {
  uint matIdx;
  std::vector<Vector3> vertices;
  std::vector<Vector2> uvs;
};

std::string exportOBJ(std::string_view objName,
                      const std::vector<FaceData>& facesData,
                      const std::vector<std::string>& matNames) {
  std::set<Vector3> vertexSet;
  std::set<Vector2> uvSet;

  std::vector<Vector3> vertices;
  std::vector<Vector2> uvs;

  for (const auto& face : facesData) {
    for (const auto& vertex : face.vertices) {
      vertexSet.insert(vertex);
    }
    for (const auto& uv : face.uvs) {
      uvSet.insert(uv);
    }
  }

  auto to_str = [](int v, int vt) {
    return std::to_string(v + 1) + "/" + std::to_string(vt + 1);
  };

  vertices.assign(vertexSet.begin(), vertexSet.end());
  uvs.assign(uvSet.begin(), uvSet.end());

  std::map<std::string, std::vector<std::vector<std::pair<int, int>>>> matFaces;
  for (auto& face : facesData) {
    std::vector<std::pair<int, int>> faceVerts;
    for (size_t i = 0; i < face.vertices.size(); ++i) {
      int v = BinarySearch<Vector3>::indexOf(vertices, face.vertices[i]);
      int vt = BinarySearch<Vector2>::indexOf(uvs, face.uvs[i]);
      faceVerts.emplace_back(v, vt);
    }
    matFaces[matNames[face.matIdx]].push_back(std::move(faceVerts));
  }

  std::stringstream ss;
  ss << "# " + std::to_string(vertices.size()) + " vertices\n";
  ss << "# " + std::to_string(uvs.size()) + " texture coordinates\n";
  ss << "mtllib " << objName << ".mtl\n";
  for (auto& v : vertices) {
    ss << "v " << v.x << " " << v.y << " " << v.z << "\n";
  }
  for (auto& uv : uvs) {
    ss << "vt " << uv.x << " " << uv.y << "\n";
  }

  for (auto it = matFaces.begin(); it != matFaces.end(); ++it) {
    const std::string& name = it->first;
    const std::vector<std::vector<std::pair<int, int>>>& faceList = it->second;
    ss << "usemtl " << name << "\n";
    for (const auto& verts : faceList) {
      ss << "f ";  // << to_str(face.first, face.second) << "\n"
      for (size_t i = 0; i < verts.size(); ++i) {
        ss << to_str(verts[i].first, verts[i].second) << " ";
      }
      ss << "\n";
    }
  }
  // Continue with the rest of the exportOBJ function
  return ss.str();  // Return the accumulated string
}

}  // namespace obj

int main(int argc, char* argv[]) {
  printf("Hello, World!\n");

  std::string mapName("2fort5r");
  FileData bspFile("assets/" + mapName + ".bsp");

  bsp::initPalette();
  Uint64 offset = reinterpret_cast<Uint64>(bspFile.data.data());
  bsp::Header* header = reinterpret_cast<bsp::Header*>(offset);
  bsp::Plane* planes =
      reinterpret_cast<bsp::Plane*>(offset + header->planes.offset);
  bsp::MipHeader* mipheaders =
      reinterpret_cast<bsp::MipHeader*>(offset + header->miptex.offset);
  Vector3* vertices =
      reinterpret_cast<Vector3*>(offset + header->vertices.offset);
  bsp::Surface* surfaces =
      reinterpret_cast<bsp::Surface*>(offset + header->texinfo.offset);
  bsp::Face* faces =
      reinterpret_cast<bsp::Face*>(offset + header->faces.offset);
  Sint32* faceIndices =
      reinterpret_cast<Sint32*>(offset + header->facesList.offset);
  bsp::Edge* edges =
      reinterpret_cast<bsp::Edge*>(offset + header->edges.offset);
  Sint32* edgeList =
      reinterpret_cast<Sint32*>(offset + header->edgesList.offset);
  bsp::Model* models =
      reinterpret_cast<bsp::Model*>(offset + header->models.offset);

  struct Texinfo {
    std::string name;
    int w, h;
    std::vector<RGBA> data;
  };

  std::stringstream ss;
  std::vector<std::string> matNames;
  std::vector<std::pair<float, float>> matSizes;

  for (int i = 0, unnamed = 0; i < mipheaders->numTex; ++i) {
    bsp::MipTex* miptex = reinterpret_cast<bsp::MipTex*>(
        offset + header->miptex.offset + mipheaders->offsets[i]);
    if (miptex->name[0] == '*') {
      miptex->name[0] =
          '_';  // Replace '*' with '_' to avoid issues with file names
    } else if (miptex->name[0] == '\0') {
      sprintf(miptex->name, "unnamed_%d", unnamed);
      unnamed++;
    }
    printf("texture: %16s (%d x %d)\n", miptex->name, miptex->width,
           miptex->height);
    Texinfo tex;
    int size = miptex->width * miptex->height;
    tex.name = miptex->name;
    tex.w = miptex->width;
    tex.h = miptex->height;
    tex.data.resize(size);
    Uint8* mipData =
        reinterpret_cast<Uint8*>(offset + header->miptex.offset +
                                 mipheaders->offsets[i] + miptex->offsets[0]);
    for (int i = 0; i < size; i++) {
      RGB col = bsp::getColor(mipData[i]);
      tex.data[i].r = col.b;
      tex.data[i].g = col.g;
      tex.data[i].b = col.r;
      tex.data[i].a = 255;
    }

    std::string path = "export/textures/" + tex.name + ".png";
    unsigned error = lodepng_encode32_file(
        path.c_str(), (Uint8*)tex.data.data(), miptex->width, miptex->height);
    if (error) {
      printf("lodepng error %u: %s\n", error, lodepng_error_text(error));
    } else {
      ss << "newmtl " << tex.name << std::endl;
      ss << "Ka 1.000 1.000 1.000" << std::endl;
      ss << "Kd 1.000 1.000 1.000" << std::endl;
      ss << "Ks 0.000 0.000 0.000" << std::endl;
      ss << "Tr 1" << std::endl;
      ss << "Ns 0" << std::endl;
      ss << "map_Kd textures/" << tex.name << ".png" << std::endl;
      matNames.push_back(tex.name);
      matSizes.push_back(
          {static_cast<float>(tex.w), static_cast<float>(tex.h)});
    }
  }
  exportStr("export/" + mapName + ".mtl", ss.str());

  uint numFaces = header->faces.size / sizeof(bsp::Face);
  uint numModels = header->models.size / sizeof(bsp::Model);

  for (uint i = 0; i < 1; ++i) {
    bsp::Model& model = models[i];
    std::vector<obj::FaceData> facesData;

    for (uint j = 0; j < model.faceNum; ++j) {
      bsp::Face& face = faces[model.faceId + j];
      //   printf("face %u:\n", j);
      //   printf(" - ledgeId: %d, ledgeNum: %d, textureId: %d, vertices:\n",
      //           face.ledgeId, face.ledgeNum, face.textureId);
      std::vector<int> vs;
      for (int k = 0; k < face.ledgeNum; ++k) {
        int edgeIdx = edgeList[face.ledgeId + k];
        bsp::Edge& edge = edges[std::abs(edgeIdx)];
        vs.push_back(edgeIdx < 0 ? edge.vertexIds[1] : edge.vertexIds[0]);
      }
      obj::FaceData faceData;
      auto& surface = surfaces[face.textureId];
      faceData.matIdx = surface.textureId;
      auto texSize = matSizes[faceData.matIdx];
      float invW = 1.0f / texSize.first;
      float invH = 1.0f / texSize.second;
      for (auto v : vs) {
        Vector3 vtx = vertices[v];
        faceData.vertices.push_back(0.01f * vtx);
        float s = (surface.vectorS.dot(vtx) + surface.distS) * invW;
        float t = (surface.vectorT.dot(vtx) + surface.distT) * invH;
        Vector2 uv(s, t);
        faceData.uvs.push_back(uv);
      }
      std::reverse(faceData.vertices.begin(), faceData.vertices.end());
      std::reverse(faceData.uvs.begin(), faceData.uvs.end());
      facesData.push_back(faceData);
    }
    auto obj = obj::exportOBJ(mapName, facesData, matNames);
    if (0 == i) {
      exportStr("export/" + mapName + ".obj", obj);
    } else {
      exportStr("export/" + mapName + "_" + std::to_string(i) + ".obj", obj);
    }
  }

  printf("# of faces..: %u\n", numFaces);
  printf("# of models.: %u\n", numModels);
  printf("Goodbye!\n");

  return 0;
}