#include "FunctionalTestFixture.h"

#include "Scenarios.h"

#include <Autotests/ClientSDK/Modules/ClientShip.h>

#include <yaml-cpp/yaml.h>
#include <sstream>

namespace autotests
{

class CelestialScannerTests : public FunctionalTestFixture
{
protected:
  // overrides from FunctionalTestFixture interface
  bool initialWorldState(YAML::Node& state) {
    std::string data[] = {
      "Blueprints:",
      "  Zond:",
      "    radius: 0.1",
      "    weight: 10 ",
      "    modules:",
      "      scanner:",
      "        type:                   CelestialScanner",
      "        max_scanning_radius_km: 100000",
      "        processing_time_us:     10",
      "Players:",
      "  test:",
      "    password: test",
      "    ships:",
      "      Zond:",
      "        position: { x: 0, y: 0}",
      "        velocity: { x: 0, y: 0}",
      "World:",
      "  Asteroids:",
      "    - { position:  { x: 100000, y: 0},",
      "        velocity:  { x: 0,    y: 0},",
      "        radius:    100,",
      "        silicates:  80,",
      "        mettals:    15,",
      "        ice:         5 }",
      "    - { position:  { x: 0, y: 100000},",
      "        velocity:  { x: 0, y: 0},",
      "        radius:    100,",
      "        silicates:  50,",
      "        mettals:     3,",
      "        ice:        40 }",
      // Creating two identical asteroids clouds:
      "  AsteroidsClouds:",
      "    - { pattern:        736628372,",
      "        center:         { x: 100000, y: 0 },",
      "        area_radius_km: 30,",
      "        total:          10 }",
      "    - { pattern:        736628372,",
      "        center:         { x: 0, y: 100000 },",
      "        area_radius_km: 30,",
      "        total:          10 }"
    };
    std::stringstream ss;
    for (std::string const& line : data)
      ss << line << "\n";
    state = YAML::Load(ss.str());
    return true;
  }
};

} // namespace autotests