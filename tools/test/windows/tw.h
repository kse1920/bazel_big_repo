// Copyright 2018 The Bazel Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BAZEL_TOOLS_TEST_WINDOWS_TW_H_
#define BAZEL_TOOLS_TEST_WINDOWS_TW_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace bazel {

namespace windows {
class AutoHandle;
}  // namespace windows

namespace tools {
namespace test_wrapper {

// Info about a file/directory in the results of TestOnly_GetFileListRelativeTo.
class FileInfo {
 public:
  // C'tor for a directory.
  FileInfo(const std::wstring& rel_path)
      : rel_path_(rel_path), size_(0), is_dir_(true) {}

  // C'tor for a file.
  // Marked "explicit" because `size` is just a `int`.
  explicit FileInfo(const std::wstring& rel_path, int size)
      : rel_path_(rel_path), size_(size), is_dir_(false) {}

  inline const std::wstring& RelativePath() const { return rel_path_; }

  inline int Size() const { return size_; }

  inline bool IsDirectory() const { return is_dir_; }

 private:
  // The file's path, relative to the traversal root.
  std::wstring rel_path_;

  // The file's size, in bytes.
  //
  // Unfortunately this field has to be `int`, so it can only describe files up
  // to 2 GiB in size. The reason is, devtools_ijar::Stat::total_size is
  // declared as `int`, which is what we ultimately store the file size in,
  // therefore this field is also `int`.
  int size_;

  // Whether this is a directory (true) or a regular file (false).
  bool is_dir_;
};

// Zip entry paths for devtools_ijar::ZipBuilder.
// The function signatures mirror the signatures of ZipBuilder's functions.
class ZipEntryPaths {
 public:
  // Initialize the strings in this object.
  // `root` must be an absolute mixed-style path (Windows path with "/"
  // separators).
  // `files` must be relative, Unix-style paths.
  void Create(const std::string& root, const std::vector<std::string>& files);

  // Returns the number of paths in `AbsPathPtrs` and `EntryPathPtrs`.
  size_t Size() const { return size_; }

  // Returns a mutable array of const pointers to const char data.
  // Each pointer points to an absolute path: the file to archive.
  // The pointers are owned by this object and become invalid when the object is
  // destroyed.
  // Each entry corresponds to the entry at the same index in `EntryPathPtrs`.
  char const* const* AbsPathPtrs() const { return abs_path_ptrs_.get(); }

  // Returns a mutable array of const pointers to const char data.
  // Each pointer points to a relative path: an entry in the zip file.
  // The pointers are owned by this object and become invalid when the object is
  // destroyed.
  // Each entry corresponds to the entry at the same index in `AbsPathPtrs`.
  char const* const* EntryPathPtrs() const { return entry_path_ptrs_.get(); }

 private:
  size_t size_;
  std::unique_ptr<char[]> abs_paths_;
  std::unique_ptr<char*[]> abs_path_ptrs_;
  std::unique_ptr<char*[]> entry_path_ptrs_;
};

// Streams data from an input to two outputs.
// Inspired by tee(1) in the GNU coreutils.
class Tee {
 public:
  virtual ~Tee() {}

 protected:
  Tee() {}
  Tee(const Tee&) = delete;
  Tee& operator=(const Tee&) = delete;
};

// Buffered input stream (based on a HANDLE) with peek-ahead support.
class IFStream {
 public:
  virtual ~IFStream() {}

  // Gets the current byte under the read cursor.
  // Returns true upon success, returns false if there's no more data to read.
  virtual bool Get(uint8_t* result) const = 0;

  // Advances the read cursor one byte ahead. May fetch data from the underlying
  // HANDLE.
  // Returns true if the cursor could be moved. Returns false if EOF was reached
  // or if there was an I/O error.
  virtual bool Advance() = 0;

  // Peeks at the next byte after the read cursor. Returns true if there's at
  // least one more byte in the stream.
  bool Peek1(uint8_t* result) const { return PeekN(1, result); }

  // Peeks at the next two bytes after the read cursor. Returns true if there
  // are at least two more byte in the stream.
  bool Peek2(uint8_t* result) const { return PeekN(2, result); }

  // Peeks at the next three bytes after the read cursor. Returns true if there
  // are at least three more byte in the stream.
  bool Peek3(uint8_t* result) const { return PeekN(3, result); }

 protected:
  IFStream() {}
  IFStream(const IFStream&) = delete;
  IFStream& operator=(const IFStream&) = delete;

  // Peeks ahead N bytes, writing them to 'result'. Returns true if successful.
  // The result does not include the byte currently under the read cursor.
  virtual bool PeekN(DWORD n, uint8_t* result) const = 0;
};

// The main function of the test wrapper.
int TestWrapperMain(int argc, wchar_t** argv);

// The main function of the test XML writer.
int XmlWriterMain(int argc, wchar_t** argv);

// The "testing" namespace contains functions that should only be used by tests.
namespace testing {

// Retrieves an environment variable.
bool TestOnly_GetEnv(const wchar_t* name, std::wstring* result);

// Lists all files under `abs_root`, with paths relative to `abs_root`.
// Limits the directory depth to `depth_limit` many directories below
// `abs_root`.
// A negative depth means unlimited depth. 0 depth means searching only
// `abs_root`, while a positive depth limit allows matches in up to that many
// subdirectories.
bool TestOnly_GetFileListRelativeTo(const std::wstring& abs_root,
                                    std::vector<FileInfo>* result,
                                    int depth_limit = -1);

// Converts a list of files to ZIP file entry paths.a
bool TestOnly_ToZipEntryPaths(
    const std::wstring& abs_root,
    const std::vector<bazel::tools::test_wrapper::FileInfo>& files,
    ZipEntryPaths* result);

// Archives `files` into a zip file at `abs_zip` (absolute path to the zip).
bool TestOnly_CreateZip(const std::wstring& abs_root,
                        const std::vector<FileInfo>& files,
                        const std::wstring& abs_zip);

// Returns the MIME type of a file. The file does not need to exist.
std::string TestOnly_GetMimeType(const std::string& filename);

// Returns the contents of the Undeclared Outputs manifest.
bool TestOnly_CreateUndeclaredOutputsManifest(
    const std::vector<FileInfo>& files, std::string* result);

bool TestOnly_CreateUndeclaredOutputsAnnotations(
    const std::wstring& abs_root, const std::wstring& abs_output);

bool TestOnly_AsMixedPath(const std::wstring& path, std::string* result);

// Creates a Tee object. See the Tee class declaration for more info.
bool TestOnly_CreateTee(bazel::windows::AutoHandle* input,
                        bazel::windows::AutoHandle* output1,
                        bazel::windows::AutoHandle* output2,
                        std::unique_ptr<Tee>* result);

bool TestOnly_CdataEncode(const uint8_t* buffer, const DWORD size,
                          std::basic_ostream<char>* out_stm);

IFStream* TestOnly_CreateIFStream(bazel::windows::AutoHandle* handle,
                                  DWORD page_size);

}  // namespace testing

}  // namespace test_wrapper
}  // namespace tools
}  // namespace bazel

#endif  // BAZEL_TOOLS_TEST_WINDOWS_TW_H_
