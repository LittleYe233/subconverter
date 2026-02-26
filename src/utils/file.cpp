#include <string>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <mutex>
#include <cstdlib>
#include <climits>

#include "utils/string.h"
#include "handler/settings.h"

#ifdef _WIN32
#include <stdlib.h>
static std::string get_absolute_path(const std::string& path) {
    char abs_path[_MAX_PATH];
    if (_fullpath(abs_path, path.c_str(), _MAX_PATH) != NULL) {
        return std::string(abs_path);
    }
    return "";
}
#else
static std::string get_absolute_path(const std::string& path) {
    char abs_path[PATH_MAX];
    if (realpath(path.c_str(), abs_path) != NULL) {
        return std::string(abs_path);
    }
    return "";
}
#endif

/**
 * @brief Resolve `allowed_scopes` in global variable. Both relative
 * and absolute paths are supported. Relative paths are based on the current
 * folder of the executable. All paths are resolved to absolute paths.
 * 
 * @param scopes A list of allowed paths.
 */
void resolveAllowedScopes(std::vector<std::string>& scopes)
{
    std::vector<std::string> resolved_scopes;
    
    // Always add current working directory to allowed scopes
    std::string cwd = get_absolute_path(".");
    if (!cwd.empty()) {
        resolved_scopes.push_back(cwd);
    }

    for (const auto& scope : scopes) {
        std::string abs_scope = get_absolute_path(scope);
        if (!abs_scope.empty()) {
            resolved_scopes.push_back(abs_scope);
        }
    }

    scopes = resolved_scopes;
}

/**
 * @brief Determine if a given `path` is within the allowed scopes.
 *
 * Allowed scopes are defined as `global.allowedScopes` plus the current folder
 * of the executable as well as its children. All paths are resolved to 
 * absolute paths before comparison to handle symlinks and relative paths securely.
 * 
 * @param path The path to be checked.
 * @return true if the path is within allowed scopes.
 * @return false 
 */
bool isInScope(const std::string &path)
{
    if (path.find("..") != std::string::npos)
        return false;

    std::string abs_path = get_absolute_path(path);
    if (abs_path.empty()) {
        return false;
    }

    if (!global.enableAllowedScopes) {
        return true;
    }

    if (global.allowedScopes.empty()) {
        std::string cwd = get_absolute_path(".");
        if (!cwd.empty() && startsWith(abs_path, cwd)) {
             // Check boundary to avoid /tmp/foo matching /tmp/foobar
            if(abs_path.size() == cwd.size()) return true;
            char next_char = abs_path[cwd.size()];
            if(next_char == '/' || next_char == '\\') return true;
        }
        return false;
    }

    for (const auto& scope : global.allowedScopes)
    {
        if (startsWith(abs_path, scope))
        {
            if (abs_path.size() == scope.size()) return true;
            char last_scope_char = scope.back();
            if (last_scope_char == '/' || last_scope_char == '\\') return true;
            char next_char = abs_path[scope.size()];
            if (next_char == '/' || next_char == '\\') return true;
        }
    }

    return false;
}

// TODO: Add preprocessor option to disable (open web service safety)
std::string fileGet(const std::string &path, bool scope_limit)
{
    std::string content;

    if(scope_limit && !isInScope(path))
        return "";

    std::FILE *fp = std::fopen(path.c_str(), "rb");
    if(fp)
    {
        std::fseek(fp, 0, SEEK_END);
        long tot = std::ftell(fp);
        /*
        char *data = new char[tot + 1];
        data[tot] = '\0';
        std::rewind(fp);
        std::fread(&data[0], 1, tot, fp);
        std::fclose(fp);
        content.assign(data, tot);
        delete[] data;
        */
        content.resize(tot);
        std::rewind(fp);
        std::fread(&content[0], 1, tot, fp);
        std::fclose(fp);
    }

    /*
    std::stringstream sstream;
    std::ifstream infile;
    infile.open(path, std::ios::binary);
    if(infile)
    {
        sstream<<infile.rdbuf();
        infile.close();
        content = sstream.str();
    }
    */
    return content;
}

bool fileExist(const std::string &path, bool scope_limit)
{
    //using c++17 standard, but may cause problem on clang
    //return std::filesystem::exists(path);
    if(scope_limit && !isInScope(path))
        return false;
    struct stat st;
    return stat(path.data(), &st) == 0 && S_ISREG(st.st_mode);
}

bool fileCopy(const std::string &source, const std::string &dest)
{
    std::ifstream infile;
    std::ofstream outfile;
    infile.open(source, std::ios::binary);
    if(!infile)
        return false;
    outfile.open(dest, std::ios::binary);
    if(!outfile)
        return false;
    try
    {
        outfile<<infile.rdbuf();
    }
    catch (std::exception &e)
    {
        return false;
    }
    infile.close();
    outfile.close();
    return true;
}

int fileWrite(const std::string &path, const std::string &content, bool overwrite)
{
    /*
    std::fstream outfile;
    std::ios_base::openmode mode = overwrite ? std::ios_base::out : std::ios_base::app;
    mode |= std::ios_base::binary;
    outfile.open(path, mode);
    outfile << content;
    outfile.close();
    return 0;
    */
    const char *mode = overwrite ? "wb" : "ab";
    std::FILE *fp = std::fopen(path.c_str(), mode);
    std::fwrite(content.c_str(), 1, content.size(), fp);
    std::fclose(fp);
    return 0;
}
