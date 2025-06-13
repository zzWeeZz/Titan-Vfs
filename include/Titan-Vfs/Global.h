#ifndef GLOBAL_H
#define GLOBAL_H

#include <mutex>

#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/vector.h>
#include <EASTL/list.h>

#include <EASTL/algorithm.h>

#include <EASTL/unordered_map.h>

#include <fstream>
#include <iostream>
#include <filesystem>

namespace Titan::Vfs
{
#ifdef TITAN_VFS_MT_SUPPORT
    constexpr bool g_MtSupportEnabled = true;
#else
    constexpr bool g_MtSupportEnabled = false;
#endif
}

#endif // GLOBAL_H
