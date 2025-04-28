/*!
 * @file main.cpp
 * @description
 * This program implements a single line of code count for C/C++ programs.
 * @author	Add your name here
 * @date	September, 9th 2024.
 * @remark On 2022-09-9 changed ...
 */
#include <cstdint>
#include <cstdlib>
#include <dirent.h>
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

/// The running options provided via CLI.
struct RunningOpt {
  std::vector<std::string> input_list;  //!< This might be a list of filenames or a directories.
  // TODO: add other running options here.
};

//== Aux functions

// trim from left
inline std::string ltrim(const std::string& s, const char* t = " \t\n\r\f\v") {
  std::string clone{ s };
  clone.erase(0, clone.find_first_not_of(t));
  return clone;
}
// trim from right
inline std::string rtrim(const std::string& s, const char* t = " \t\n\r\f\v") {
  // TODO
  return s;
}
// trim from left & right
inline std::string trim(const std::string& s, const char* t = " \t\n\r\f\v") {
  // TODO
  return s;
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
