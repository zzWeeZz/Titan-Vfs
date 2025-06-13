#ifndef VIRTUALFILESYSTEM_HPP
#define VIRTUALFILESYSTEM_HPP

#include "IFileSystem.h"
#include "IFile.h"
#include "EASTL/sort.h"

namespace Titan::Vfs
{

using HVirtualFileSystem = std::shared_ptr<class VirtualFileSystem>;
    
class VirtualFileSystem final
{
public:
    typedef eastl::list<HFileSystem> TFileSystemList;
    typedef eastl::unordered_map<eastl::string, TFileSystemList> TFileSystemMap;

private:
    VirtualFileSystem()
    {
    }
public:

    ~VirtualFileSystem()
    {
        for (const auto& fs : m_FileSystems) {
            for (const auto& f : fs.second) {
                f->Shutdown();
            }
        }
    }

    static HVirtualFileSystem Create()
    {
        return HVirtualFileSystem(new VirtualFileSystem());
    }
    
    /*
     * Register new filesystem. Alias is a base prefix to file access.
     * For ex. registered filesystem has base path '/home/media', but registered
     * with alias '/', so it possible to access files with path '/filename'
     * instead of '/home/media/filename
     */
    void AddFileSystem(eastl::string alias, HFileSystem filesystem)
    {
        if (!filesystem) {
            return;
        }

        if (!StringUtils::EndsWith(alias, "/")) {
            alias += "/";
        }
        
        eastl::function<void()> fn = [&]() {
            m_FileSystems[alias].push_back(filesystem);
            if (eastl::find(m_SortedAlias.begin(), m_SortedAlias.end(), alias) == m_SortedAlias.end()) {
                m_SortedAlias.push_back(alias);
            }
            eastl::sort(m_SortedAlias.begin(), m_SortedAlias.end(), [](const eastl::string& a1, const eastl::string& a2) {
                return a1.length() > a2.length();
            });
        };
        
        if constexpr (g_MtSupportEnabled) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            fn();
        } else {
            fn();
        }
    }
    
    /*
     * Remove registered filesystem
     */
    void RemoveFileSystem(eastl::string alias, HFileSystem filesystem)
    {
        if (!StringUtils::EndsWith(alias, "/")) {
            alias += "/";
        }

        eastl::function<void()> fn = [&]() {
            auto it = m_FileSystems.find(alias);
            if (it != m_FileSystems.end()) {
                it->second.remove(filesystem);
                if (it->second.empty()) {
                    m_FileSystems.erase(it);
                    m_SortedAlias.erase(std::remove(m_SortedAlias.begin(), m_SortedAlias.end(), alias), m_SortedAlias.end());
                }
            }
        };

        if constexpr (g_MtSupportEnabled) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            fn();
        } else {
            fn();
        }
    }

    /*
     * Check if filesystem with 'alias' added
     */
    bool HasFileSystem(eastl::string alias, HFileSystem fileSystem) const
    {
        if (!StringUtils::EndsWith(alias, "/")) {
            alias += "/";
        }

        eastl::function<bool()> fn = [&]() -> bool {
            auto it = m_FileSystems.find(alias);
            if (it != m_FileSystems.end()) {
                return (std::find(it->second.begin(), it->second.end(), fileSystem) != it->second.end());
            }
            return false;
        };

        if constexpr (g_MtSupportEnabled) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return fn();
        } else {
            return fn();
        }
    }

    /*
     * Unregister all filesystems with 'alias'
     */
    void UnregisterAlias(eastl::string alias)
    {
        if (!StringUtils::EndsWith(alias, "/")) {
            alias += "/";
        }

        eastl::function<void()> fn = [&]() {
            m_FileSystems.erase(alias);
            m_SortedAlias.erase(eastl::remove(m_SortedAlias.begin(), m_SortedAlias.end(), alias), m_SortedAlias.end());
        };

        if constexpr (g_MtSupportEnabled) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            fn();
        } else {
            fn();
        }
    }
    
    /*
     * Check if there any filesystem with 'alias' registered
     */
    bool IsAliasRegistered(eastl::string alias) const
    {
        if (!StringUtils::EndsWith(alias, "/")) {
            alias += "/";
        }

        if constexpr (g_MtSupportEnabled) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return (m_FileSystems.find(alias) != m_FileSystems.end());
        } else {
            return (m_FileSystems.find(alias) != m_FileSystems.end());
        }
    }
    
    /*
     * Get all added filesystems with 'alias'
     */
    const TFileSystemList& GetFilesystems(eastl::string alias)
    {
        if constexpr (g_MtSupportEnabled) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return GetFilesystemsST(alias);
        } else {
            return GetFilesystemsST(alias);
        }
    }
    
    /*
     * Iterate over all registered filesystems and find first ocurrences of file
     */
    HFile OpenFile(const FileInfo& filePath, IFile::FileMode mode)
    {
        eastl::function<HFile()> fn = [&]() -> HFile {
            for (const eastl::string& alias : m_SortedAlias) {
                if (!StringUtils::StartsWith(filePath.AbsolutePath(), alias)) {
                    continue;
                }

                // Strip alias from file path
                eastl::string relativePath = filePath.AbsolutePath().substr(alias.length());
                
                // Enumerate reverse to get filesystems in order of registration
                const TFileSystemList& filesystems = GetFilesystemsST(alias);
                if (filesystems.empty()) {
                    continue;
                }
                
                for (auto it = filesystems.rbegin(); it != filesystems.rend(); ++it) {
                    // Is it last filesystem
                    HFileSystem fs = *it;
                    bool isMain = (fs == filesystems.front());

                    // If file exists in filesystem we try to open it. 
                    // In case file not exists and we are in first filesystem we try to create new file if mode allows it
                    FileInfo realPath(fs->BasePath(), relativePath, false);
                    if (fs->IsFileExists(realPath) || isMain) {
                        HFile file = fs->OpenFile(realPath, mode);
                        if (file) {
                            return file;
                        }
                    }
                }
            }
            
            return nullptr;
        };

        if constexpr (g_MtSupportEnabled) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return fn();
        } else {
            return fn();
        }
    }

    eastl::string AbsolutePath(eastl::string_view relativePath)
    {
        eastl::function<eastl::string()> fn = [&]() -> eastl::string {
            eastl::string strRelativePath = eastl::string(relativePath);
            for (const eastl::string& alias : m_SortedAlias) {
                if (!StringUtils::StartsWith(strRelativePath, alias)) {
                    continue;
                }

                // Strip alias from file path
                eastl::string strippedRelativePath = strRelativePath.substr(alias.length());

                // Enumerate reverse to get filesystems in order of registration
                const TFileSystemList& filesystems = GetFilesystemsST(alias);
                if (filesystems.empty()) {
                    continue;
                }

                for (auto it = filesystems.rbegin(); it != filesystems.rend(); ++it) {
                    // Is it last filesystem
                    HFileSystem fs = *it;
                    bool isMain = (fs == filesystems.front());

                    // If file exists in filesystem we try to open it. 
                    // In case file not exists and we are in first filesystem we try to create new file if mode allows it
                    FileInfo realPath(fs->BasePath(), strippedRelativePath, false);
                    if (fs->IsFileExists(realPath) || isMain) {
                        return realPath.AbsolutePath();
                    }
                }
            }

            return "";
            };

        if constexpr (g_MtSupportEnabled) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return fn();
        }
        else {
            return fn();
        }
    }

private:
    inline const TFileSystemList& GetFilesystemsST(eastl::string alias)
    {
        if (!StringUtils::EndsWith(alias, "/")) {
            alias += "/";
        }

        auto it = m_FileSystems.find(alias);
        if (it != m_FileSystems.end()) {
            return it->second;
        }
        
        static TFileSystemList empty;
        return empty;
    }

    
private:
    TFileSystemMap m_FileSystems;
    eastl::vector<eastl::string> m_SortedAlias;
    mutable std::mutex m_Mutex;
};
    
}; // namespace vfspp

#endif // VIRTUALFILESYSTEM_HPP
