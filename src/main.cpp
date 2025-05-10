/*!
 * @file main.cpp
 * @description
 * This program implements a single line of code count for C/C++ programs.
 * @author	Marcel Setubal Costa & Olive Oliveira Medeiros
 * @date	September, 9th 2024.
 * @remark On 2022-09-9 changed ...
 */
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

//== Enumerations

/// This enumeration lists all the supported languages.
enum lang_type_e : std::uint8_t {
  C = 0,  //!< C language
  CPP,    //!< C++ language
  H,      //!< C/C++ header
  HPP,    //!< C++ header
  UNDEF,  //!< Undefined type.
};

//== Class/Struct declaration

/// Integer type for counting lines.
using count_t = unsigned long;
/// Stores the file information we are collecting.
class FileInfo {
public:
  std::string filename;  //!< the filename.
  lang_type_e type;      //!< the language type.
  count_t n_blank;       //!< # of blank lines in the file.
  count_t n_comments;    //!< # of comment lines.
  count_t n_loc;         //!< # lines of code.
  count_t n_lines;       //!< # of lines.

  /// Ctro.
  FileInfo(std::string fn = "",
           lang_type_e t = UNDEF,
           count_t nb = 0,
           count_t nc = 0,
           count_t nl = 0,
           count_t ni = 0)
      : filename{ std::move(fn) }, type{ t }, n_blank{ nb }, n_comments{ nc }, n_loc{ nl },
        n_lines{ ni } {
    /* empty*/
  }
};
using FileList = std::vector<FileInfo>;
/// The running options provided via CLI.
struct RunningOpt {
  std::vector<std::string> input_list;  //!< This might be a list of filenames or a directories.
  bool recursive{ false };
  bool should_order{ false };
  std::pair<bool, char> ordering_method;  // first = true if -s, false if -S;
};

//== Aux functions
/// Prints the help message and exits the program.
/// If `message` is not empty, it is printed as an error before the help.
void usage(const std::string& message = "") {
  if (!message.empty()) {
    std::cerr << "[ERROR] " << message << "\n\n";
  }

  std::cerr
    << "Welcome to sloc cpp, version 1.0, (c) DIMAp/UFRN.\n\n"
    << "NAME\n"
    << "  sloc - single line of code counter.\n\n"
    << "SYNOPSIS\n"
    << "  sloc [-h | --help] [-r] [(-s | -S) f|t|c|b|s|a] <file | directory>\n\n"
    << "EXAMPLES\n"
    << "  sloc main.cpp sloc.cpp\n"
    << "     Counts loc, comments, blanks of the source files 'main.cpp' and 'sloc.cpp'\n\n"
    << "  sloc source\n"
    << "     Counts loc, comments, blanks of all C/C++ source files inside 'source'\n\n"
    << "  sloc -r -s c source\n"
    << "     Counts loc, comments, blanks of all C/C++ source files recursively inside 'source'\n"
    << "     and sort the result in ascending order by # of comment lines.\n\n"
    << "DESCRIPTION\n"
    << "  Sloc counts the individual number **lines of code** (LOC), comments, and blank\n"
    << "  lines found in a list of files or directories passed as the last argument\n"
    << "  (after options).\n"
    << "  After the counting process is concluded the program prints out to the standard\n"
    << "  output a table summarizing the information gathered, by each source file and/or\n"
    << "  directory provided.\n"
    << "  It is possible to inform which fields sloc should use to sort the data by, as\n"
    << "  well as if the data should be presented in ascending/descending numeric order.\n\n"
    << "OPTIONS:\n"
    << "  -h/--help\n"
    << "            Display this information.\n\n"
    << "  -r\n"
    << "            Look for files recursively in the directory provided.\n\n"
    << "  -s f|t|c|d|b|s|a\n"
    << "            Sort table in ASCENDING order by (f)ilename, (t) filetype,\n"
    << "            (c)omments, (d)oc comments, (b)lank lines, (s)loc, or (a)ll.\n"
    << "            Default is to show files in ordem of appearance.\n\n"
    << "  -S f|t|c|d|b|s|a\n"
    << "            Sort table in DESCENDING order by (f)ilename, (t) filetype,\n"
    << "            (c)omments, (d)oc comments, (b)lank lines, (s)loc, or (a)ll.\n"
    << "            Default is to show files in ordem of appearance.\n";

  std::exit(message.empty() ? EXIT_SUCCESS : EXIT_FAILURE);
}

void validate_arguments(int argc, char* argv[], RunningOpt& run_options) {
  int c;
  int option_index{ 0 };

  static struct option long_options[] = { { "help", no_argument, 0, 'h' }, { 0, 0, 0, 0 } };

  while ((c = getopt_long(argc, argv, "hrs:S:", long_options, &option_index)) != -1) {
    switch (c) {
    case 'h':
      usage("");
      break;
    case 'r':
      run_options.recursive = true;
      break;
    case 's':
    case 'S':
      run_options.should_order = true;
      run_options.ordering_method.first = (c == 's');
      if (optarg == nullptr or strlen(optarg) != 1 or strchr("ftcdbsa", optarg[0]) == nullptr) {
        usage("Invalid character value for sorting -s -S");
        break;
      }
      run_options.ordering_method.second = optarg[0];
      break;
    default:
      usage("Invalid option");
      break;
    }
  }
  for (int i = optind; i < argc; ++i) {
    run_options.input_list.emplace_back(argv[i]);
  }
  if (run_options.input_list.empty())
    usage("Please, provide a source file or directory");
}

bool ends_with(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size()
         && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::optional<lang_type_e> id_lang_type(const std::string& filename) {
  if (ends_with(filename, ".c"))
    return C;
  if (ends_with(filename, ".cpp"))
    return CPP;
  if (ends_with(filename, ".h"))
    return H;
  if (ends_with(filename, ".hpp"))
    return HPP;
  return std::nullopt;
}

std::string to_lower(const std::string& str) {
  std::string result = str;
  std::transform(
    result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
  return result;
}

/// Retrieves a list of supported file names, based on the directory/file provided
FileList create_list_of_src_files(const std::vector<std::string>& src_list, bool recursive_search) {
  FileList file_list;
  // Traverse source list
  for (const auto& item : src_list) {
    // If it's directory, let us collect file names
    if (std::filesystem::is_directory(item) and recursive_search) {
      // Iterates over all the entries
      for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ item }) {
        // Get language type based on the file extension.
        auto lang_type = id_lang_type(to_lower(dir_entry.path().string()));
        if (lang_type.has_value()) {
          file_list.emplace_back(dir_entry.path(), lang_type.value());
        }
      }
    } else if (std::filesystem::is_directory(item)) {
      // Non recursive directory
      for (auto const& dir_entry : std::filesystem::directory_iterator{ item }) {
        // Get language type
        auto lang_type = id_lang_type(to_lower(dir_entry.path().string()));
        if (lang_type.has_value()) {
          file_list.emplace_back(dir_entry.path(), lang_type.value());
        }
      }
    } else if (std::filesystem::is_regular_file(item)) {
      auto lang_type = id_lang_type(to_lower(item));
      if (lang_type.has_value()) {
        file_list.emplace_back(item, lang_type.value());
      }
    }
  }
  return file_list;
}

// trim from left
inline std::string ltrim(const std::string& s, const char* t = " \t\n\r\f\v") {
  std::string clone{ s };
  clone.erase(0, clone.find_first_not_of(t));
  return clone;
}
// trim from right
inline std::string rtrim(const std::string& s, const char* t = " \t\n\r\f\v") {
  std::string clone{ s };
  clone.erase(clone.find_last_not_of(t) + 1);
  return clone;
  return s;
}
// trim from left & right
inline std::string trim(const std::string& s, const char* t = " \t\n\r\f\v") {
  return rtrim(ltrim(s, t), t);
}

struct CountFields {
  int locs{ 0 };
  int comments{ 0 };
  int docblocks{ 0 };
  int blanks{ 0 };
};

class CodeParser {
private:
  bool inside_comment{ false };
  bool inside_docblock{ false };
  bool inside_literal{ false };

public:
  CountFields parse_line(const std::string& line);
};

CountFields CodeParser::parse_line(const std::string& line) {
  CountFields count_data;
  std::string trimmed = trim(line);  // right trim + left trim
  auto len = trimmed.size();

  if (len == 0) {  // blank line
    count_data.blanks = 1;
    return count_data;
  }

  bool inside_comment_block = false;
  bool inside_docblock = false;
  bool inside_single_line_comment = false;
  bool inside_literal = false;
  bool inside_char = false;

  for (size_t i = 0; i < len;) {
    // grab 3 characters
    if (i + 2 < len) {
      std::string sub = trimmed.substr(i, 3);

      if (sub == "/*") {
        inside_comment_block = true;
        count_data.comments = 1;
        i += 3;
        continue;
      }

      if (sub == "/**" || sub == "/*!") {
        inside_docblock = true;
        count_data.docblocks = 1;
        i += 3;
        continue;
      }

      if (sub == "///" || sub == "//!") {
        inside_single_line_comment = true;
        count_data.comments = 1;
        break;  // because it's a single line comment
      }
    }

    // grab 2 characters
    if (i + 1 < len) {
      std::string sub = trimmed.substr(i, 2);

      if (sub == "//") {
        inside_single_line_comment = true;
        count_data.comments = 1;
        break;
      }
    }

    // grab 1 character
    if (i < len) {
      if (trimmed[i] == '"' && !inside_comment_block && !inside_char) {
        inside_literal = !inside_literal;
      } else if (trimmed[i] == '\'' && !inside_comment_block && !inside_literal) {
        inside_char = !inside_char;
      }

      // are we inside a comment block?
      if (inside_comment_block) {
        if (i + 1 < len && trimmed[i] == '*' && trimmed[i + 1] == '/') {
          inside_comment_block = false;  // close the comment block
          i += 2;                        // getting out of block
          continue;
        }
      }

      // if it's none other state, then it's a code line
      if (!inside_comment_block && !inside_docblock && !inside_literal && !inside_char
          && !isspace(trimmed[i])) {
        count_data.locs = 1;
      }

      i++;
    }
  }

  if (inside_single_line_comment or inside_docblock) {
    count_data.comments = 1;
    count_data.locs = 0;  // no code in this line
  }

  return count_data;
}
//== Main entry

int main(int argc, char* argv[]) {

  std::vector<FileInfo> db;

  FileInfo fc;
  fc.filename = "test.cpp";
  fc.type = CPP;
  fc.n_comments = 4;
  fc.n_blank = 5;
  fc.n_loc = 15;
  fc.n_lines = 20;

  db.push_back(fc);

  db.emplace_back("test.cpp", CPP, 3, 10, 15, 18);

  return EXIT_SUCCESS;
}
