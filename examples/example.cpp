#include "Titan-Vfs/VFS.h"

using namespace Titan;
using namespace std::string_view_literals;

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line) {
    return malloc(size);
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line) {
    return malloc(size);
}

void PrintFile(const eastl::string& msg, Vfs::HFile file)
{
    if (file && file->IsOpened()) {
        char data[256];
        memset(data, 0, sizeof(data));
        file->Read(reinterpret_cast<uint8_t*>(data), 256);
        
        printf("%s\n%s\n", msg.c_str(), data);
    }
}

int main()
{
	const Vfs::HVirtualFileSystem vfs = Vfs::VirtualFileSystem::Create();
    const Vfs::HFileSystem rootFS = Vfs::NativeFileSystem::Create("../test-data/files");
    const Vfs::HFileSystem memFS = Vfs::MemoryFileSystem::Create();
    const Vfs::HFileSystem zipFS = Vfs::ZipFileSystem::Create("../test-data/test.zip");

    rootFS->Initialize();
    memFS->Initialize();
    zipFS->Initialize();

    vfs->AddFileSystem("/", rootFS);
    vfs->AddFileSystem("/memory", memFS);
    vfs->AddFileSystem("/zip", zipFS);

    printf("Native filesystem test:\n");

	auto test = vfs->AbsolutePath("/test.txt");
    Vfs::HFile file = vfs->OpenFile(Vfs::FileInfo("/test.txt"), Vfs::IFile::FileMode::ReadWrite);
    if (file && file->IsOpened()) {
        char data[] = "The quick brown fox jumps over the lazy dog\n";
        file->Write(reinterpret_cast<uint8_t*>(data), sizeof(data));
        file->Close();
    }

    Vfs::HFile file2 = vfs->OpenFile(Vfs::FileInfo("/test.txt"), Vfs::IFile::FileMode::Read);
    if (file2 && file2->IsOpened()) {
        PrintFile("File /test.txt:", file2);
    }

    printf("Memory filesystem test:\n");

    Vfs::HFile memFile = vfs->OpenFile(Vfs::FileInfo("/memory/file.txt"), Vfs::IFile::FileMode::ReadWrite);
    if (memFile && memFile->IsOpened()) {
        char data[] = "The quick brown fox jumps over the lazy dog\n";
	    memFile->Write(reinterpret_cast<uint8_t*>(data), sizeof(data));
	    memFile->Close();
    }
    
    Vfs::HFile memFile2 = vfs->OpenFile(Vfs::FileInfo("/memory/file.txt"), Vfs::IFile::FileMode::Read);
    if (memFile2 && memFile2->IsOpened()) {
        PrintFile("File /memory/file.txt:", memFile2);
    }

    printf("Zip filesystem test:\n");

    Vfs::IFileSystem::TFileList files = zipFS->FileList();
    for (auto& file : files) {
		printf("Zip file entry: %s\n", file.first.c_str());
	}

    Vfs::HFile zipFile = vfs->OpenFile(Vfs::FileInfo("/zip/file.txt"), Vfs::IFile::FileMode::Read);
    if (zipFile && zipFile->IsOpened()) {
        PrintFile("File /zip/file.txt:", zipFile);
    }

    printf("DLC filesystem test:\n");
    
    Vfs::HFileSystem dlc1FS = Vfs::NativeFileSystem::Create("../test-data/dlc1");
    Vfs::HFileSystem dlc2FS = Vfs::NativeFileSystem::Create("../test-data/dlc2");

    dlc1FS->Initialize();
    dlc2FS->Initialize();

    vfs->AddFileSystem("/dlc", dlc1FS);
       
    Vfs::HFile dlcFile = vfs->OpenFile(Vfs::FileInfo("/dlc/file.txt"), Vfs::IFile::FileMode::Read);
    if (dlcFile && dlcFile->IsOpened()) {
        PrintFile("File /dlc/file.txt that exists in dlc1:", dlcFile);
        dlcFile->Close();
    }
    
    vfs->AddFileSystem("/dlc", dlc2FS);

    dlcFile = vfs->OpenFile(Vfs::FileInfo("/dlc/file.txt"), Vfs::IFile::FileMode::Read);
    if (dlcFile && dlcFile->IsOpened()) {
        PrintFile("File /dlc/file.txt patched by dlc2:", dlcFile);
        dlcFile->Close();
    }

    Vfs::HFile dlcFile1 = vfs->OpenFile(Vfs::FileInfo("/dlc/file1.txt"), Vfs::IFile::FileMode::Read);
    if (dlcFile1 && dlcFile1->IsOpened()) {
        PrintFile("File /dlc/file1.txt that exists only in dlc1:", dlcFile1);
        dlcFile1->Close();
    }

    Vfs::HFile dlcFile2 = vfs->OpenFile(Vfs::FileInfo("/dlc/file2.txt"), Vfs::IFile::FileMode::Read);
    if (dlcFile2 && dlcFile2->IsOpened()) {
        PrintFile("File /dlc/file2.txt that exists only in dlc2:", dlcFile2);
        dlcFile2->Close();
    }

	return 0;
}

