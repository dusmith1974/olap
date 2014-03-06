#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

typedef std::vector<std::string> MsgVec;
typedef std::map<int, std::string> CompetitorMap;

int main() {
  std::string line;
  std::ifstream file;

  MsgVec msgs;
  CompetitorMap competitors;

  file.open("competitors.txt");
  while (std::getline(file, line)) {
    boost::char_separator<char> sep(",");
    boost::tokenizer<boost::char_separator<char>> tokens(line, sep);

    // TODO(ds) Spirit to parse line.
    int competitor_num = 0;
    for (const auto& token : tokens) {
      try {
        competitor_num = boost::lexical_cast<int>(token);
      } catch (boost::bad_lexical_cast&) {
        competitors[competitor_num] = token;
      }
    }
  }

  int grid_pos = 1;
  for (const auto& competitor : competitors) {
    std::stringstream ss;
    ss << "competitor," << competitor.first << "," << competitor.second << "\n";
    msgs.push_back(ss.str());

    ss.str("");
    ss.clear();
    ss << "grid-pos," << grid_pos << "," << competitor.second << "\n";
    msgs.push_back(ss.str());

    ++grid_pos;
  }

  std::copy(msgs.begin(), msgs.end(),
      std::ostream_iterator<std::string>(std::cout));
}
