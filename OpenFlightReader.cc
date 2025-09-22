#include "OpenFlightReader.h"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

constexpr size_t HeaderSize = 4;
constexpr size_t RecordIdSize = 8;

OpenFlightReader::OpenFlightReader(const std::string &filename)
    : filename_(filename) {}

void OpenFlightReader::Read() {
  std::ifstream file(filename_, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Error opening file: " + filename_);
  }

  ParseRecords(file);
}

void OpenFlightReader::ParseRecords(std::ifstream &file) {
  while (file) {
    std::array<uint8_t, HeaderSize> header{};
    file.read(reinterpret_cast<char *>(header.data()), HeaderSize);

    if (file.gcount() != HeaderSize)
      break;

    uint16_t opcode = (header[0] << 8) | header[1];
    uint16_t length = (header[2] << 8) | header[3];
    switch (opcode) {
    case 1:
    case 2:
    case 4: {
      std::string id = ReadId(file);
      std::cout << indent_str_ << "ID: " << id << std::endl;
      auto bytesRead = HeaderSize + RecordIdSize;
      if (length < bytesRead) {
        throw std::runtime_error("Corrupted file" + filename_);
      }
      file.seekg(length - bytesRead, std::ios::cur);
      break;
    }
    case 5: {

      ReadFace(file, length);
      break;
    }
    case 10: {
      indent_level_++;
      UpdateIndent();
      file.seekg(length - HeaderSize, std::ios::cur);
      break;
    }
    case 11: {
      if (indent_level_ > 0) {
        indent_level_--;
        UpdateIndent();
      }
      file.seekg(length - HeaderSize, std::ios::cur);
      break;
    }
    default:
      if (length < HeaderSize) {
        throw std::runtime_error("Corrupted record (invalid length)");
      }
      file.seekg(length - HeaderSize, std::ios::cur);
      break;
    }
  }
}

uint16_t OpenFlightReader::ReadUInt16BE(std::ifstream &file) {
  uint8_t buf[2];
  file.read(reinterpret_cast<char *>(buf), sizeof(buf));
  return static_cast<uint16_t>((buf[0] << 8) | buf[1]);
}

void OpenFlightReader::ReadFace(std::ifstream &file, uint16_t length) {
  SimpleFaceData face;

  face.id = ReadId(file, 7);

  file.seekg(8, std::ios::cur);
  face.color_name_index = ReadUInt16BE(file);

  file.seekg(RecordIdSize, std::ios::cur);
  face.material_index = ReadUInt16BE(file);

  size_t bytesRead = HeaderSize + 7 + 8 + 2 + 8 + 2;

  if (length > bytesRead) {
    file.seekg(length - bytesRead, std::ios::cur);
  }

  std::array<uint8_t, HeaderSize> nextHeader{};
  auto currentPos = file.tellg();
  file.read(reinterpret_cast<char *>(nextHeader.data()), HeaderSize);

  if (file.gcount() == HeaderSize) {
    uint16_t nextOpcode = (nextHeader[0] << 8) | nextHeader[1];
    uint16_t nextLength = (nextHeader[2] << 8) | nextHeader[3];

    if (nextOpcode == 33) {
      std::string longId = ReadId(file, nextLength - HeaderSize);
      face.id = longId;
    } else {
      file.seekg(currentPos);
    }
  } else {
    file.seekg(currentPos);
  }

  std::cout << indent_str_ << "ID: '" << face.id
            << "', Color Name Index: " << face.color_name_index
            << ", Material Index: " << face.material_index << std::endl;
}

std::string OpenFlightReader::ReadId(std::ifstream &file, size_t size) {
  std::vector<char> buf(size);
  file.read(buf.data(), buf.size());

  std::string id(buf.data(), size - 1);
  if (auto pos = id.find('\0'); pos != std::string::npos) {
    id.resize(pos);
  }

  return id;
}
