
// =================================================================================================
// This file is part of the CLTune project, which loosely follows the Google C++ styleguide and uses
// a tab-size of two spaces and a max-width of 100 characters per line.
//
// Author(s):
//   Cedric Nugteren <www.cedricnugteren.nl>
//
// This file tests public methods of the Tuner class.
//
// =================================================================================================

#include "catch.hpp"

#include "cltune.h"

// Settings
const size_t kPlatformID = 0;
const size_t kDeviceID = 0;
const size_t kInvalidPlatformID = 99;
const size_t kInvalidDeviceID = 99;

// =================================================================================================

SCENARIO("tuners can be created", "[Tuner]") {
  REQUIRE_NOTHROW(new cltune::Tuner(kPlatformID, kDeviceID));
}

SCENARIO("invalid tuners throw an exception", "[Tuner]") {
  REQUIRE_THROWS_AS(new cltune::Tuner(kPlatformID, kInvalidDeviceID), std::runtime_error);
  REQUIRE_THROWS_AS(new cltune::Tuner(kInvalidPlatformID, kDeviceID), std::runtime_error);
  REQUIRE_THROWS_AS(new cltune::Tuner(kInvalidPlatformID, kInvalidDeviceID), std::runtime_error);
}

// =================================================================================================

SCENARIO("kernels can be added", "[Tuner]") {
  GIVEN("An example tuner") {
    cltune::Tuner tuner(kPlatformID, kDeviceID);
    tuner.SuppressOutput();

    // Example data
    const auto kConfigGlobal = cltune::IntRange(128, 256);
    const auto kConfigLocal = cltune::IntRange(8, 1);
    const std::vector<std::pair<std::string,std::string>> kExampleKernels = {
      {"../samples/simple/simple_reference.opencl","matvec_reference"},
      {"../samples/simple/simple_unroll.opencl","matvec_unroll"}
    };
    const auto kExampleParameter = std::string{"TEST_PARAM"};
    const auto kExampleParameterValues = std::initializer_list<size_t>{6, 9, 1003};
    const auto kExampleRange = cltune::StringRange{kExampleParameter, kExampleParameter};

    auto counter = size_t{0};
    WHEN("new kernels are added") {

      for (; counter<kExampleKernels.size(); ++counter) {
        auto example_kernel = kExampleKernels[counter];
        auto id = tuner.AddKernel({example_kernel.first}, example_kernel.second,
                                  kConfigGlobal, kConfigLocal);

        THEN("their IDs are monotonously increasing #" + std::to_string(counter)) {
          REQUIRE(counter == id);
        }

        AND_THEN("their parameters can be specified, but duplicates cannot #" + std::to_string(counter)) {
          tuner.AddParameter(id, kExampleParameter, kExampleParameterValues);
          REQUIRE_THROWS_AS(tuner.AddParameter(id, kExampleParameter, kExampleParameterValues),
                            std::runtime_error);
        }

        AND_THEN("string-ranges can be set #" + std::to_string(counter)) {
          tuner.MulGlobalSize(id, kExampleRange);
          tuner.DivGlobalSize(id, kExampleRange);
          tuner.MulLocalSize(id, kExampleRange);
          tuner.DivLocalSize(id, kExampleRange);
        }
      }
    }

    WHEN("parameters for invalid kernels are added") {
      THEN("an exception is thrown") {
        REQUIRE_THROWS_AS(tuner.AddParameter(counter, kExampleParameter, kExampleParameterValues),
                          std::runtime_error);
      }
    }

    WHEN("string-ranges for invalid kernels are set") {
      THEN("an exception is thrown") {
        REQUIRE_THROWS_AS(tuner.MulGlobalSize(counter, kExampleRange), std::runtime_error);
        REQUIRE_THROWS_AS(tuner.DivGlobalSize(counter, kExampleRange), std::runtime_error);
        REQUIRE_THROWS_AS(tuner.MulLocalSize(counter, kExampleRange), std::runtime_error);
        REQUIRE_THROWS_AS(tuner.DivLocalSize(counter, kExampleRange), std::runtime_error);
      }
    }

  }
}

// =================================================================================================
