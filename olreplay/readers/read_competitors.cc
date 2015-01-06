// Copyright 2014 Duncan Smith
// https://github.com/dusmith1974/olap
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Implements ReadCompetitors.

#include "olreplay_pch.h"  // NOLINT

#include "readers/read_competitors.h"

#include <fstream>  // NOLINT
#include <string>  // NOLINT

#include "olcore/messages/competitor.h"

namespace olap {
void ReadBestSectors(CompetitorMap* competitors) {
  if (!competitors) return;

  std::ifstream file;
  std::string filename("BestSectors.txt");
  file.open(filename);
  if (!file.is_open())
    throw std::runtime_error("Could not open " + filename);

  int num = 0;
  cpp_dec_float_3 lap_time;
  cpp_dec_float_3 sector_1, sector_2, sector_3;
  cpp_dec_float_3 sector_1_pc, sector_2_pc, sector_3_pc;

  std::string str;
  while (std::getline(file, str)) {
    std::istringstream iss(str);
    iss >> num;
    iss >> sector_1; iss >> sector_2; iss >> sector_3;

    sector_1_pc = sector_1 / (sector_1 + sector_2 + sector_3);
    sector_2_pc = sector_2 / (sector_1 + sector_2 + sector_3);
    sector_3_pc = sector_3 / (sector_1 + sector_2 + sector_3);
    sector_3_pc += 1 - (sector_1_pc + sector_2_pc + sector_3_pc);
    (*competitors)[num].set_sector_1_percent(sector_1_pc);
    (*competitors)[num].set_sector_2_percent(sector_2_pc);
    (*competitors)[num].set_sector_3_percent(sector_3_pc);

    // TODO(ds) NDEBUG release
    assert(1 == sector_1_pc + sector_2_pc + sector_3_pc);
  }
}

void ReadCompetitors(CompetitorMap* competitors) {
  if (!competitors) return;

  std::ifstream file;
  std::string filename("competitors.txt");

  file.open(filename);
  if (!file.is_open())
    throw std::runtime_error("Could not open " + filename);

  int grid_pos = 0;
  std::string line;
  while (std::getline(file, line)) {
    auto competitor = boost::lexical_cast<Competitor>(line);
    competitor.set_grid_pos(++grid_pos);
    (*competitors)[competitor.num()] = competitor;
  }

  ReadBestSectors(competitors);
}
}  // namespace olap
