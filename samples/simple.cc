
// =================================================================================================
// This file is part of the CLTune project, which loosely follows the Google C++ styleguide and uses
// a tab-size of two spaces and a max-width of 100 characters per line.
//
// Author: cedric.nugteren@surfsara.nl (Cedric Nugteren)
//
// This file demonstrates the usage of CLTune with a simple matrix-vector multiplication example.
//
// -------------------------------------------------------------------------------------------------
//
// Copyright 2014 SURFsara
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//  http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// =================================================================================================

#include <iostream>
#include <sstream>
#include <vector>

// Includes the OpenCL tuner library
#include "tuner/tuner.h"

// =================================================================================================

// Basic example showing how to tune OpenCL kernels. We take a matrix-vector multiplication as an
// example. Provided is reference code, a version with manual unrolling, and one with tiling in the
// local memory.
int main() {

  // Matrix size
  constexpr auto kSizeM = 2048;
  constexpr auto kSizeN = 4096;

  // Creates data structures
  std::vector<float> mat_a(kSizeN*kSizeM); // Assumes matrix A is transposed
  std::vector<float> vec_x(kSizeN);
  std::vector<float> vec_y(kSizeM);

  // Populates input data structures
  srand(time(nullptr));
  for (auto &item: mat_a) { item = (float)rand() / (float)RAND_MAX; }
  for (auto &item: vec_x) { item = (float)rand() / (float)RAND_MAX; }
  for (auto &item: vec_y) { item = 0.0; }

  // Initializes the tuner (platform 0, device 1)
  cltune::Tuner tuner(0, 0);

  // Adds a kernel which supports unrolling through the UNROLL parameter. Note that the kernel
  // itself needs to implement the UNROLL parameter and (in this case) only accepts a limited
  // amount of values.
  auto id = tuner.AddKernel("../samples/simple_unroll.opencl", "matvec_unroll", {kSizeM}, {128});
  tuner.AddParameter(id, "UNROLL", {1, 2, 4});

  // Adds another kernel and its parameters. This kernel caches the input vector X into local
  // memory to save global memory accesses. Note that the kernel's workgroup size is determined by
  // the tile size parameter TS.
  id = tuner.AddKernel("../samples/simple_tiled.opencl", "matvec_tiled", {kSizeM}, {1});
  tuner.AddParameter(id, "TS", {32, 64, 128, 256, 512});
  tuner.MulLocalSize(id, {"TS"});

  // Sets the tuner's golden reference function. This kernel contains the reference code to which
  // the output is compared. Supplying such a function is not required, but it is necessarily for
  // correctness checks to be enabled.
  tuner.SetReference("../samples/simple_reference.opencl", "matvec_reference", {kSizeM}, {128});

  // Sets the function's arguments. Note that all kernels have to accept (but not necessarily use)
  // all input arguments.
  tuner.AddArgumentScalar(kSizeM);
  tuner.AddArgumentScalar(kSizeN);
  tuner.AddArgumentInput(mat_a);
  tuner.AddArgumentInput(vec_x);
  tuner.AddArgumentOutput(vec_y);

  // Starts the tuner
  tuner.Tune();

  // Prints the results to screen
  tuner.PrintToScreen();
  return 0;
}

// =================================================================================================