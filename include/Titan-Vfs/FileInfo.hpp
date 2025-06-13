#ifndef FILEINFO_HPP
#define FILEINFO_HPP

#include "Global.h"
#include "StringUtils.hpp"

namespace fs = std::filesystem;

namespace Titan::Vfs
{
    
class FileInfo final
{
public:
    FileInfo(const eastl::string& filePath)
    {
        Configure("", filePath, false);
    }

    FileInfo(const eastl::string& basePath, const eastl::string& fileName, bool isDir)
    {
        Configure(basePath, fileName, isDir);
    }

    FileInfo(const fs::path& path, bool isDir)
        : m_Path(path)
        , m_IsDir(isDir)
    {
    }

    FileInfo() = delete;

    ~FileInfo()
    {

    }
    
    /*
     * Get file name with extension
     */
    inline eastl::string Name() const
    {
        return m_Path.filename().string().c_str();
    }
    
    /*
     * Get file name without extension
     */
    inline eastl::string BaseName() const
    {
        return m_Path.stem().string().c_str();
    }
    
    /*
     * Get file extension
     */
    inline eastl::string Extension() const
    {
        return m_Path.extension().string().c_str();
    }
    
    /*
     * Get absolute file path
     */
    inline eastl::string AbsolutePath() const
    {
        return m_Path.string().c_str();
    }
    
    /*
     * Is a directory
     */
    inline bool IsDir() const
    {
        return m_IsDir;
    }

    inline const fs::path& Path() const
    {
        return m_Path;
    }
    
    /*
     *
     */
    inline bool IsValid() const
    {
        return !m_Path.string().empty();
    }

private:
    void Configure(const eastl::string& basePath, const eastl::string& fileName, bool isDir)
    {
        m_Path = (fs::path(basePath.c_str()) / fs::path(fileName.c_str())).generic_string();
        m_IsDir = isDir;
    }
    
private:
    fs::path m_Path;
    bool m_IsDir;
};
    
inline bool operator ==(const FileInfo& fi1, const FileInfo& fi2)
{
    return fi1.AbsolutePath() == fi2.AbsolutePath();
}
    
inline bool operator <(const FileInfo& fi1, const FileInfo& fi2)
{
    return fi1.AbsolutePath() < fi2.AbsolutePath();
}
    
}; // namespace vfspp

#endif // FILEINFO_HPP
