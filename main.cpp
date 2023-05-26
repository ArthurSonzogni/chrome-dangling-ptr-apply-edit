#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Edit {
  std::string file;
  int line;
};

bool operator<(const Edit& a, const Edit& b) {
  // Files are sorted alphabetically
  if (a.file < b.file)
    return true;
  if (b.file < a.file)
    return false;

  // Lines are sorted descendently, so that edits can be applied in order.
  if (a.line > b.line)
    return true;
  if (b.line > a.line)
    return false;

  return false;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: ChromeDanglingPtrApplyEdit edit-file" << std::endl;
    return EXIT_FAILURE;
  }

  std::vector<Edit> edits;
  std::ifstream input(argv[1]);
  std::string line;
  while (std::getline(input, line)) {
    std::cout << "Reading: " << line << std::endl;
    auto separator = line.find(':');
    edits.push_back(Edit{
        line.substr(0, separator),
        std::stoi(line.substr(separator + 1)),
    });
  }

  std::sort(edits.begin(), edits.end());

  int i = 0;
  for (auto& it : edits) {
    std::cout << "Applying: " << it.file << ":" << it.line << std::endl;
    std::string content;
    {
      std::ifstream input(it.file);
      std::string line;
      int line_index = 0;
      while (std::getline(input, line)) {
        if (++line_index == it.line) {
          const int start_experimental_ash = line.find("ExperimentalAsh");
          if (start_experimental_ash != std::string::npos) {
            line.insert(start_experimental_ash, "DanglingUntriaged | ");
            std::cout << "Rewrote " << line << std::endl;
          } else {
            const int start_ptr = line.find("raw_ptr<");
            const int start_ref = line.find("raw_ref<");
            const int start =
                (start_ptr != std::string::npos) ? start_ptr : start_ref;
            if (start != std::string::npos) {
              int depth = 0;
              for (int i = start; i < line.size(); ++i) {
                if (line[i] == '<') {
                  depth++;
                  continue;
                }

                if (line[i] == '>') {
                  if (depth == 1) {
                    line.insert(i, ", DanglingUntriaged");
                    std::cout << "Rewrote " << line << std::endl;
                    break;
                  }
                  depth--;
                }
              }
            }
          }
        }
        content += line + '\n';
      }
    }
    {
       std::ofstream output(it.file);
       output << content;
    }
  }

  return EXIT_SUCCESS;
}
