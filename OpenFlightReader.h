#pragma once
#include <fstream>
#include <memory>
#include <vector>
struct Node {
  std::string id;
  std::string type; // "db", "group", "object", "face"
  uint16_t color_name_index = 0;
  int16_t material_index = 0;
  std::vector<std::unique_ptr<Node>> children;

  Node(std::string id, std::string type) : id(id), type(type) {}
};

enum NodeType {
  Header = 1,
  Group = 2,
  Object = 4,
  Face = 5,
  Push = 10,
  Pop = 11,
  LongId = 33
};
class OpenFlightReader {
public:
  OpenFlightReader(const std::string &filename);
  void PrintStructure() const;

  void Read();

private:
  void ParseRecords(std::ifstream &file);
  template <typename T> T ReadUInt16BE(std::ifstream &file);
  void ReadFace(std::ifstream &file, uint16_t length);
  std::string ReadId(std::ifstream &file, size_t size = 8);

  std::string filename_;
  std::unique_ptr<Node> root_;
  std::vector<Node *> node_stack_;
};