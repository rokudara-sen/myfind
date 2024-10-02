#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

void searchForFile(const std::string& searchpath, const std::string& filename, bool recursive, bool case_insensitive, std::vector<std::string>& found_files);
bool matchFilename(const std::string& file1, const std::string& file2, bool case_insensitive);

int main(int argc, char *argv[]) {
    int opt;
    bool recursive = false;
    bool case_insensitive = false;
    std::vector<std::string> filenames;
    std::string searchpath;

    while ((opt = getopt(argc, argv, "Ri")) != -1) {
        if (opt == 'R') {
            recursive = true;
        } else if (opt == 'i') {
            case_insensitive = true;
        }
    }

    if (optind >= argc) {
        std::cerr << "Usage: " << argv[0] << " [-R] [-i] searchpath filename1 [filename2] ..." << std::endl;
        return 1;
    }
    searchpath = argv[optind++];
    
    if (optind >= argc) {
        std::cerr << "Error: No filenames specified." << std::endl;
        return 1;
    }
    for (int i = optind; i < argc; ++i) {
        filenames.push_back(argv[i]);
    }

    std::vector<pid_t> child_pids;
    for (const auto& filename : filenames) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        } else if (pid == 0) {
            std::vector<std::string> found_files;
            searchForFile(searchpath, filename, recursive, case_insensitive, found_files);
            pid_t mypid = getpid();
            for (const auto& found_file : found_files) {
                std::cout << mypid << ": " << filename << ": " << found_file << std::endl;
            }
            exit(0);
        } else {
            child_pids.push_back(pid);
        }
    }

    int status;
    for (pid_t pid : child_pids) {
        waitpid(pid, &status, 0);
    }

    return 0;
}

void searchForFile(const std::string& searchpath, const std::string& filename, bool recursive, bool case_insensitive, std::vector<std::string>& found_files) {
    namespace fs = std::filesystem;
    try {
        fs::path search_dir(searchpath);
        if (!fs::exists(search_dir) || !fs::is_directory(search_dir)) {
            std::cerr << "Error: Invalid search path '" << searchpath << "'" << std::endl;
            return;
        }

        if (recursive) {
            for (const auto& entry : fs::recursive_directory_iterator(search_dir)) {
                if (fs::is_regular_file(entry.status())) {
                    std::string entry_filename = entry.path().filename().string();
                    if (matchFilename(entry_filename, filename, case_insensitive)) {
                        found_files.push_back(fs::absolute(entry.path()).string());
                    }
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(search_dir)) {
                if (fs::is_regular_file(entry.status())) {
                    std::string entry_filename = entry.path().filename().string();
                    if (matchFilename(entry_filename, filename, case_insensitive)) {
                        found_files.push_back(fs::absolute(entry.path()).string());
                    }
                }
            }
        }
    } catch (...) {
        
    }
}

bool matchFilename(const std::string& file1, const std::string& file2, bool case_insensitive) {
    if (case_insensitive) {
        std::string f1 = file1;
        std::string f2 = file2;
        std::transform(f1.begin(), f1.end(), f1.begin(), ::tolower);
        std::transform(f2.begin(), f2.end(), f2.begin(), ::tolower);
        return f1 == f2;
    } else {
        return file1 == file2;
    }
}

