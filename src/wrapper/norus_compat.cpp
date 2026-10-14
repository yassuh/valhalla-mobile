// Stand-ins for the mjolnir code that the Norus engine reaches from the routing libraries.
//
// Mobile builds leave out mjolnir, valhalla's tile-building library (ENABLE_DATA_TOOLS is off),
// because it pulls in OpenSSL, SQLite, and the OSM parsers.
// The Norus engine uses it from two places that the server needs and the phone links in:
//
// - baldr::EdgeInfo::GetCustomAttributes splits the custom attribute text with
//   mjolnir::GetTagTokens. The phone needs this to read the organization of an edge,
//   so it is a copy of the real function from src/valhalla/src/mjolnir/util.cc.
// - thor's traffic action writes live and predicted speeds into tiles with GraphTileBuilder.
//   That is a server action. The wrapper never calls the actor's traffic action
//   (it exposes route, trace_route, trace_attributes, height, and matrix),
//   so it never runs on a phone, and these stand-ins throw rather than write anything.
//   Android's linker drops the unreachable code before resolving symbols; Apple's does not.
//
// Remove this file once the engine no longer needs mjolnir for these.

#include <valhalla/mjolnir/graphtilebuilder.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace valhalla {
namespace mjolnir {

std::vector<std::string> GetTagTokens(const std::string& tag_value, const std::string& delim_str) {
  std::vector<std::string> tokens;
  if (delim_str.empty()) {
    tokens.emplace_back(tag_value);
    return tokens;
  }
  size_t start = 0, pos;
  while ((pos = tag_value.find(delim_str, start)) != std::string::npos) {
    tokens.emplace_back(tag_value, start, pos - start);
    start = pos + delim_str.size();
  }
  // an empty token after the last delimiter is not emitted
  if (start < tag_value.size()) {
    tokens.emplace_back(tag_value, start);
  }
  return tokens;
}

namespace {

[[noreturn]] void ThrowTileWritesUnsupported() {
  throw std::runtime_error("Writing routing tiles is not supported on mobile.");
}

} // namespace

GraphTileBuilder::GraphTileBuilder(const std::string&,
                                   const baldr::GraphId&,
                                   const bool,
                                   bool) {
  ThrowTileWritesUnsupported();
}

baldr::DirectedEdge& GraphTileBuilder::directededge(const size_t) {
  ThrowTileWritesUnsupported();
}

void GraphTileBuilder::AddPredictedSpeed(const uint32_t,
                                         const std::array<int16_t, baldr::kCoefficientCount>&,
                                         const size_t) {
  ThrowTileWritesUnsupported();
}

void GraphTileBuilder::UpdatePredictedSpeeds(const std::vector<baldr::DirectedEdge>&) {
  ThrowTileWritesUnsupported();
}

} // namespace mjolnir
} // namespace valhalla
