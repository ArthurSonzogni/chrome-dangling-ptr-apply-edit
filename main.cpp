#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Edit {
  std::string file;
  int line;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: ChromeDanglingPtrApplyEdit edit-file" << std::endl;
    return EXIT_FAILURE;
  }

  std::vector<Edit> edits;
  std::ifstream input(argv[1]);
  std::string line;
  while (std::getline(input, line)) {
    auto separator = line.find(':');
    edits.push_back(Edit{
        line.substr(0, separator),
        std::stoi(line.substr(separator + 1)),
    });
  }

  int i = 0;
  for (auto& it : edits) {
    std::string content;
    {
      std::ifstream input(it.file);
      std::string line;
      int line_index = 0;
      while (std::getline(input, line)) {
        if (++line_index == it.line) {
          int start = line.find("raw_ptr<");
          if (start != std::string::npos) {
            int depth = 0;
            for (int i = start; i < line.size(); ++i) {
              std::cout << "i = " << i << " " << depth << std::endl;
              if (line[i] == '<') {
                depth++;
                continue;
              }

              if (line[i] == '>') {
                if (depth == 1) {
                  line.insert(i, ", DanglingUntriaged");
                  std::cout << "Rewrote " << line << std::endl;
                }
                depth--;
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
