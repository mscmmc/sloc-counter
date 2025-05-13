/*!
 * @file main.cpp
 * @description
 * This program implements a single line of code count for C/C++ programs.
 * @author	Marcel Setubal Costa & Olive Oliveira Medeiros
 * @date	May, 12th 2025.
 */
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <fstream>
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
  count_t n_doc;         //!< # of documentation lines
  count_t n_loc;         //!< # lines of code.
  count_t n_lines;       //!< # of lines.

  /// Ctro.
  FileInfo(std::string fn = "",
           lang_type_e t = UNDEF,
           count_t nb = 0,
           count_t nc = 0,
           count_t nl = 0,
           count_t nd = 0,
           count_t ni = 0)
      : filename{ std::move(fn) }, type{ t }, n_blank{ nb }, n_comments{ nc }, n_loc{ nl },
        n_doc{ nd }, n_lines{ ni } {}
};

using FileList = std::vector<FileInfo>;

/// The running options provided via CLI.
struct RunningOpt {
  std::vector<std::string> input_list;  //!< This might be a list of filenames or a directories.
  bool recursive{ false };
  bool should_order{ false };
  std::pair<bool, char> ordering_method;  // first = true if -s, false if -S;
};

/// Class to parse each line, store the current state and the results.
class CodeParser {
private:
  int blank_lines = 0;
  int code_lines = 0;
  int comment_lines = 0;
  int doc_comment_lines = 0;

  bool in_block_comment = false;
  bool in_doc_block_comment = false;

public:
  void parse_line(const std::string& line) {
    std::string trimmed = line;
    trimmed.erase(std::remove(trimmed.begin(), trimmed.end(), ' '), trimmed.end());
    if (trimmed.empty()) {
      blank_lines++;
      return;
    }

    size_t i = 0;
    bool inside_string = false;
    bool inside_char = false;

    while (i < line.length()) {
      if (in_doc_block_comment) {
        doc_comment_lines++;
        if (line.find("*/", i) != std::string::npos) {
          in_doc_block_comment = false;
        }
        return;
      }
      if (in_block_comment) {
        comment_lines++;
        if (line.find("*/", i) != std::string::npos) {
          in_block_comment = false;
        }
        return;
      }

      if (!inside_string && line[i] == '\'') {
        inside_char = !inside_char;
        i++;
        continue;
      }

      if (!inside_char && line[i] == '"') {
        inside_string = !inside_string;
        i++;
        continue;
      }

      if (inside_string || inside_char) {
        i++;
        continue;
      }

      if (line.compare(i, 3, "///") == 0
          || line.compare(i, 3, "//!") == 0) {  // doxyxgen documentation blocks
        doc_comment_lines++;
        return;
      }
      if (line.compare(i, 2, "//") == 0) {  // line of comment
        comment_lines++;
        return;
      }
      if (line.compare(i, 3, "/**") == 0
          || line.compare(i, 3, "/*!") == 0) {  // doxygen documentation blocks
        doc_comment_lines++;
        if (line.find("*/", i + 3) == std::string::npos) {
          in_doc_block_comment = true;
        }
        return;
      }
      if (line.compare(i, 2, "/*") == 0) {  // line of comment
        comment_lines++;
        if (line.find("*/", i + 2) == std::string::npos) {
          in_block_comment = true;
        }
        return;
      }

      i++;
    }

    code_lines++;  // If it isn't a comment, documentation or blank line, then it's a single line of
                   // code.
  }

  int get_blank_lines() const { return blank_lines; }
  int get_code_lines() const { return code_lines; }
  int get_comment_lines() const { return comment_lines; }
  int get_doc_comment_lines() const { return doc_comment_lines; }
};

//== Aux functions
/**
 * @brief Prints the help message and exits the program.
 * @brief If 'message' is not empty, it is printed as an error before the help.
 * @param string message: the error message passed as an argument, or empty if help is called for.
 */
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
    << "            Default is to show files in order of appearance.\n\n"
    << "  -S f|t|c|d|b|s|a\n"
    << "            Sort table in DESCENDING order by (f)ilename, (t) filetype,\n"
    << "            (c)omments, (d)oc comments, (b)lank lines, (s)loc, or (a)ll.\n"
    << "            Default is to show files in order of appearance.\n";

  std::exit(message.empty() ? EXIT_SUCCESS : EXIT_FAILURE);
}

/**
 * @brief Validates the running options passed by command line.
 *
 * @param argc, argv: command line options.
 * @param run_options: struct with the options passed by command line.
 */
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

/**
 * @brief Checks if a string ends with a specific suffix.
 *
 * @param str: The input string to check.
 * @param suffix: The suffix to look for at the end of the string.
 * @return true if the string ends with the specific suffix, false otherwise.
 */
bool ends_with(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size()
         && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * @brief Identifies the programming language based on a filename's extension.
 *
 * @param filename: The name of the file to be checked (with extension).
 * @return An optional containing the corresponding language type enum, or std::nullopt if
 * unrecognized.
 */
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

/**
 * @brief Converts a language type enum to its corresponding string representation, to be printed
 * later.
 *
 * @param type: The language type enum value
 * @return a string representing the name of the language.
 */
std::string lang_type_to_string(lang_type_e type) {
  switch (type) {
  case C:
    return "C";
  case CPP:
    return "C++";
  case H:
    return "C Header";
  case HPP:
    return "C++ Header";
  case UNDEF:
    return "Unknown";
  default:
    return "Invalid";
  }
}

/**
 * @brief Converts a string to lowercase.
 *
 * @param str: The input string.
 * @return a new string where all characters are converted to lowercase.
 */
std::string to_lower(const std::string& str) {
  std::string result = str;
  std::transform(
    result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
  return result;
}

/**
 * @brief Retrieves a list of supported source files from a given list of paths.
 *
 * This function traverses the provided list of paths. If a path is a directory,
 * it will recursively or non-recursively collect file names depending on the
 * recursive_search flag, collecting the supported files.
 *
 * @param src_list: A list of file or directory paths to search through.
 * @param recursive_search: If true, searches directories recursively.
 * @return a list of FileInfo objects representing the supported source files.
 */
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

/**
 * @brief Trims whitespace to the left of a string.
 *
 * @param s: The string to trim.
 * @param t: A C-string containing characters to trim (whitespace characters).
 * @return a new string, left-trimmed.
 */
inline std::string ltrim(const std::string& s, const char* t = " \t\n\r\f\v") {
  std::string clone{ s };
  clone.erase(0, clone.find_first_not_of(t));
  return clone;
}

/**
 * @brief Trims whitespace to the right of a string.
 *
 * @param s: The string to trim.
 * @param t: A C-string containing characters to trim (whitespace characters).
 * @return a new string, right-trimmed.
 */
inline std::string rtrim(const std::string& s, const char* t = " \t\n\r\f\v") {
  std::string clone{ s };
  clone.erase(clone.find_last_not_of(t) + 1);
  return clone;
}

/**
 * @brief Trims whitespace on both sides of a string
 *
 * @param s The string to trim.
 * @param t A C-string containing characters to trim (whitespace characters).
 * @return a new string, trimmed from both sides.
 */
inline std::string trim(const std::string& s, const char* t = " \t\n\r\f\v") {
  return rtrim(ltrim(s, t), t);
}

/**
 * @brief Sorts a list of source files based on a specified criterion and order, passed by the user.
 *
 * The function supports sorting by:
 * - 'f' : filename
 * - 't' : language type
 * - 'c' : number of comments
 * - 'd' : number of documentation comments
 * - 'b' : number of blank lines
 * - 's' : number of lines of code
 * - 'a' : total number of lines
 *
 * @param files: The list of files to sort.
 * @param method A pair where:
 *        - first: true for ascending, false for descending
 *        - second: character representing the sorting criterion
 */
void sort_files(FileList& files, const std::pair<bool, char>& method) {
  auto [ascending, criteria] = method;
  auto comp = [&](const FileInfo& a, const FileInfo& b) -> bool {
    switch (criteria) {
    case 'f':
      return ascending ? a.filename < b.filename : a.filename > b.filename;
    case 't':
      return ascending ? a.type < b.type : a.type > b.type;
    case 'c':
      return ascending ? a.n_comments < b.n_comments : a.n_comments > b.n_comments;
    case 'd':
      return ascending ? a.n_doc < b.n_doc : a.n_doc > b.n_doc;
    case 'b':
      return ascending ? a.n_blank < b.n_blank : a.n_blank > b.n_blank;
    case 's':
      return ascending ? a.n_loc < b.n_loc : a.n_loc > b.n_loc;
    case 'a':
      return ascending ? a.n_lines < b.n_lines : a.n_lines > b.n_lines;
    default:
      return true;
    }
  };
  std::sort(files.begin(), files.end(), comp);
}

/**
 * @brief Extracts the base name (filename only) from a full file path.
 *
 * This is important for the formatting of the output table to be presented.
 *
 * @param path: The full path to the file.
 * @return the filename without directory components.
 */
std::string basename(const std::string& path) { return path.substr(path.find_last_of("/\\") + 1); }

/**
 * @brief Prints a formatted table with information about each file.
 *
 * Displays the following columns for each file:
 * - Filename
 * - Language
 * - Number of comments (and percentage)
 * - Number of documentation comments (and percentage)
 * - Number of blank lines (and percentage)
 * - Number of lines of code (and percentage)
 * - Total number of lines
 *
 * @param files: The list of files to display.
 */
void print_table(const FileList& files) {
  std::cout << "Files processed: " << files.size() << '\n';
  std::cout << std::string(114, '-') << '\n';

  std::cout << std::left << std::setw(20) << "Filename" << std::setw(12) << "Language"
            << std::setw(15) << "Comments" << std::setw(17) << "Doc Comments" << std::setw(12)
            << "Blank" << std::setw(12) << "Code" << std::setw(12) << "# of lines" << '\n';

  std::cout << std::string(114, '-') << '\n';

  for (const auto& f : files) {
    int total = f.n_blank + f.n_comments + f.n_doc + f.n_loc;

    auto percent = [&](int count) -> std::string {
      if (total == 0)
        return "0.0%";
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(1) << (100.0 * count / total) << '%';
      return oss.str();
    };

    std::ostringstream comments;
    comments << f.n_comments << " (" << percent(f.n_comments) << ")";

    std::ostringstream doc;
    doc << f.n_doc << " (" << percent(f.n_doc) << ")";

    std::ostringstream blank;
    blank << f.n_blank << " (" << percent(f.n_blank) << ")";

    std::ostringstream code;
    code << f.n_loc << " (" << percent(f.n_loc) << ")";

    std::cout << std::left << std::setw(20) << basename(f.filename) << std::setw(12)
              << lang_type_to_string(f.type) << std::setw(15) << comments.str() << std::setw(17)
              << doc.str() << std::setw(12) << blank.str() << std::setw(12) << code.str()
              << std::setw(12) << total << '\n';
  }

  std::cout << std::string(114, '-') << '\n';
}

//== Main entry

int main(int argc, char* argv[]) {
  RunningOpt run_options;
  validate_arguments(argc, argv, run_options);

  // creates the file list for processing
  FileList files = create_list_of_src_files(run_options.input_list, run_options.recursive);

  // parser
  for (auto& file : files) {
    std::ifstream in(file.filename);
    if (!in.is_open()) {
      usage("Could not open file");
      continue;
    }

    CodeParser parser;
    std::string line;
    while (std::getline(in, line)) {
      parser.parse_line(line);
    }

    file.n_blank = parser.get_blank_lines();
    file.n_comments = parser.get_comment_lines() + parser.get_doc_comment_lines();
    file.n_loc = parser.get_code_lines();
    file.n_lines = file.n_blank + file.n_loc + file.n_comments;
  }

  if (run_options.should_order) {
    sort_files(files, run_options.ordering_method);
  }

  print_table(files);

  return 0;
}
