#include "OpenFlightReader.h"
#include <array>
#include <fstream>
#include <functional>
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
  root_ = std::make_unique<Node>("", "root");
  node_stack_.push_back(root_.get());

  while (file) {
    std::array<uint8_t, HeaderSize> header{};
    file.read(reinterpret_cast<char *>(header.data()), HeaderSize);

    if (file.gcount() != HeaderSize)
      break;

    uint16_t opcode = (header[0] << 8) | header[1];
    uint16_t length = (header[2] << 8) | header[3];

    switch (opcode) {
    case NodeType::Header: { // db
      std::string id = ReadId(file);
      auto node = std::make_unique<Node>(id, "db");
      Node *node_ptr = node.get();
      root_->children.push_back(std::move(node));
      node_stack_.push_back(node_ptr);
      file.seekg(length - HeaderSize - RecordIdSize, std::ios::cur);
      break;
    }
    case NodeType::Group: { // group
      std::string id = ReadId(file);
      if (!node_stack_.empty()) {
        auto node = std::make_unique<Node>(id, "group");
        node_stack_.back()->children.push_back(std::move(node));
      }
      file.seekg(length - HeaderSize - RecordIdSize, std::ios::cur);
      break;
    }
    case NodeType::Object: { // object
      std::string id = ReadId(file);
      if (!node_stack_.empty()) {
        auto node = std::make_unique<Node>(id, "object");
        node_stack_.back()->children.push_back(std::move(node));
      }
      file.seekg(length - HeaderSize - RecordIdSize, std::ios::cur);
      break;
    }
    case NodeType::Face: { // face
      ReadFace(file, length);
      break;
    }
    case NodeType::Push: { // push
      if (!node_stack_.empty() && !node_stack_.back()->children.empty()) {
        Node *last_child = node_stack_.back()->children.back().get();
        node_stack_.push_back(last_child);
      }
      file.seekg(length - HeaderSize, std::ios::cur);
      break;
    }
    case NodeType::Pop: { // pop
      if (node_stack_.size() > 1) {
        node_stack_.pop_back();
      }
      file.seekg(length - HeaderSize, std::ios::cur);
      break;
    }
    default:
      file.seekg(length - HeaderSize, std::ios::cur);
      break;
    }
  }
}

void OpenFlightReader::ReadFace(std::ifstream &file, uint16_t length) {
  if (node_stack_.empty())
    return;
  auto id = ReadId(file, 7);

  file.seekg(8, std::ios::cur);
  auto color_name_index = ReadUInt16BE<uint16_t>(file);

  file.seekg(RecordIdSize, std::ios::cur);
  auto material_index = ReadUInt16BE<int16_t>(file);

  size_t bytesRead = HeaderSize + 7 + 8 + 2 + 8 + 2;

  if (length > bytesRead) {
    file.seekg(length - bytesRead, std::ios::cur);
  }
  auto currentPos = file.tellg();
  std::array<uint8_t, HeaderSize> nextHeader{};
  file.read(reinterpret_cast<char *>(nextHeader.data()), HeaderSize);

  if (file.gcount() == HeaderSize) {
    uint16_t nextOpcode = (nextHeader[0] << 8) | nextHeader[1];
    uint16_t nextLength = (nextHeader[2] << 8) | nextHeader[3];
    if (nextOpcode == NodeType::LongId) {
      std::string longId = ReadId(file, nextLength - HeaderSize);
      id = longId;
    } else {
      file.seekg(currentPos);
    }
  } else {
    file.seekg(currentPos);
  }

  auto face_node = std::make_unique<Node>(id, "face");
  face_node->color_name_index = color_name_index;
  face_node->material_index = material_index;
  node_stack_.back()->children.push_back(std::move(face_node));
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

template <typename T> T OpenFlightReader::ReadUInt16BE(std::ifstream &file) {
  uint8_t buf[2];
  file.read(reinterpret_cast<char *>(buf), sizeof(buf));
  return static_cast<T>((buf[0] << 8) | buf[1]);
}

void OpenFlightReader::PrintStructure() const {
  if (!root_)
    return;

  std::function<void(const Node *, int)> print_node = [&](const Node *node,
                                                          int depth) {
    std::string indent(depth * 2, ' ');

    if (node->type == "face") {
      std::cout << indent << "Face: '" << node->id
                << "', Color Index: " << node->color_name_index
                << ", Material Index: " << node->material_index << std::endl;
    } else {
      std::cout << indent << node->type << ": " << node->id << std::endl;
    }

    for (const auto &child : node->children) {
      print_node(child.get(), depth + 1);
    }
  };

  for (const auto &child : root_->children) {
    print_node(child.get(), 0);
  }
}
