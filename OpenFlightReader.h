#pragma once
#include <fstream>
class OpenFlightReader {
  struct SimpleFaceData {
    std::string id{};
    uint16_t color_name_index = 0;
    int16_t material_index = 0;
  };

public:
  OpenFlightReader(const std::string &filename);
  void Read();

private:
  void ParseRecords(std::ifstream &file);
  uint16_t ReadUInt16BE(std::ifstream &file);
  void ReadFace(std::ifstream &file, uint16_t length);
  std::string ReadId(std::ifstream &file, size_t size = 8);

  void UpdateIndent() { indent_str_ = std::string(indent_level_ * 4, ' '); }
  std::string filename_;
  int indent_level_ = 0;
  std::string indent_str_ = "";
};