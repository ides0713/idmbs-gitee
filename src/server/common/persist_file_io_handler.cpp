#include "persist_file_io_handler.h"

#include <bits/chrono.h>                                // for filesystem
#include <errno.h>                                      // for errno
#include <cstring>                                      // for strerror

#include "/home/ubuntu/idbms/src/common/common_defs.h"  // for DebugPrint

PersistFileIoHandler::PersistFileIoHandler() : file_(nullptr) {
}
PersistFileIoHandler::~PersistFileIoHandler() {
    CloseFile();
}
Re PersistFileIoHandler::CreateFile(const char *file_path) {
    namespace fs = std::filesystem;
    if (IsAssociated()) {
        DebugPrint("PersistFileIoHandler:failed to create file %s,because handler is associated with file %s now\n",
                   file_path, file_path_.c_str());
        return Re::FileAssociated;
    }
    fs::path file_path_path(file_path);
    if (fs::exists(file_path_path)) {
        DebugPrint("PersistFileIoHandler:failed to create file %s,because file already exists\n", file_path);
        return Re::FileExist;
    } else {
        FILE *f = fopen(file_path, "w+");
        if (f == nullptr) {
            DebugPrint("PersistFileIoHandler:failed to create %s, due to %s.\n", file_path, strerror(errno));
            return Re::FileCreate;
        }
        file_path_ = file_path_path;
        file_ = f;
        DebugPrint("PersistFileIoHandler:successfully create %s\n", file_path);
    }
    return Re::Success;
}
Re PersistFileIoHandler::CreateFile(std::filesystem::path file_path) {
    return CreateFile(file_path.c_str());
}
Re PersistFileIoHandler::OpenFile(const char *file_name) {
    namespace fs = std::filesystem;
    if (IsAssociated()) {
        DebugPrint("PersistFileIoHandler:failed to open file %s,because handler is associated with file %s\n",
                   file_name, file_path_.c_str());
        return Re::FileAssociated;
    }
    fs::path file_path_path(file_name);
    if (!fs::exists(file_path_path)) {
        DebugPrint("PersistFileIoHandler:failed to open file %s,because file is not exist\n", file_name);
        return Re::FileNotExist;
    }
    FILE *f = fopen(file_name, "r+");
    if (f == nullptr) {
        DebugPrint("PersistFileIoHandler:failed to open file %s, because %s.\n", file_name, strerror(errno));
        return Re::FileOpen;
    }
    file_path_ = file_path_path;
    file_ = f;
    DebugPrint("PersistFileIoHandler:successfully open file %s.\n", file_name);
    return Re::Success;
}
Re PersistFileIoHandler::CloseFile() {
    std::string file_str(file_path_.c_str());
    file_path_.clear();
    if (file_ == nullptr) {
        return Re::Success;
    }
    if (fclose(file_) != 0) {
        DebugPrint("PersistFileIoHandler:failed to close file%d:%s,error %s\n", fileno(file_), file_str.c_str(),
                   strerror(errno));
        return Re::FileClose;
    }
    file_ = nullptr;
    DebugPrint("PersistFileIoHandler:successfully close file %d:%s.\n", fileno(file_), file_str.c_str());
    return Re::Success;
}
Re PersistFileIoHandler::RemoveFile() {
    namespace fs = std::filesystem;
    if (!IsAssociated()) {
        DebugPrint("PersistFileIoHandler:remove file failed,because no file is associated with handler\n");
        return Re::FileAssociated;
    }
    if (!fs::exists(file_path_)) {
        DebugPrint("PersistFileIoHandler:remove file failed,because associated file is not exist now\n");
        return Re::FileNotExist;
    }
    if (!fs::remove(file_path_)) {
        DebugPrint("PersistFileIoHandler:remove file failed\n");
        return Re::FileRemove;
    }
    return CloseFile();
}
Re PersistFileIoHandler::WriteFile(int size, const char *data, size_t *out_size) {
    if (!IsAssociated()) {
        DebugPrint("PersistFileIoHandler:write file failed,because no file is associated with handler\n");
        return Re::FileAssociated;
    }
    size_t write_size = fwrite(data, 1, size, file_);
    if (write_size != size) {
        DebugPrint("PersistFileIoHandler:write file %s failed.\n", file_path_.c_str());
        return Re::FileWrite;
    }
    if (out_size != nullptr)
        *out_size = write_size;
    return Re::Success;
}
Re PersistFileIoHandler::WriteAt(long offset, int size, const char *data, size_t *out_size) {
    if (!IsAssociated()) {
        DebugPrint("PersistFileIoHandler:write file at %ld failed,because no file is associated with handler\n",
                   offset);
        return Re::FileAssociated;
    }
    if (fflush(file_) != 0) {
        DebugPrint("PersistFileIoHandler:write file %s at %ld failed,because flush failed %s\n", file_path_.c_str(),
                   offset, strerror(errno));
        return Re::FileError;
    }
    if (fseek(file_, offset, SEEK_SET) != 0) {
        DebugPrint("PersistFileIoHandler:write file %s at %ld failed,because seek failed\n", file_path_.c_str(),
                   offset);
        return Re::FileSeek;
    }
    size_t write_size = fwrite(data, 1, size, file_);
    if (write_size != size) {
        DebugPrint("PersistFileIoHandler:write file %s at %ld failed.\n", file_path_.c_str(), offset);
        return Re::FileWrite;
    }
    if (out_size != nullptr)
        *out_size = write_size;
    return Re::Success;
}
Re PersistFileIoHandler::Append(int size, const char *data, size_t *out_size) {
    if (!IsAssociated()) {
        DebugPrint("PersistFileIoHandler:append file failed,because no file is associated with handler\n");
        return Re::FileAssociated;
    }
    if (fflush(file_) != 0) {
        DebugPrint("PersistFileIoHandler:append file %s failed,because flush failed %s\n", file_path_.c_str(),
                   strerror(errno));
        return Re::FileError;
    }
    if (fseek(file_, 0, SEEK_END) != 0) {
        DebugPrint("PersistFileIoHandler:append file %s failed,because seek failed\n", file_path_.c_str());
        return Re::FileSeek;
    }
    size_t write_size = fwrite(data, 1, size, file_);
    if (write_size != size) {
        DebugPrint("PersistFileIoHandler:append file %s failed.\n", file_path_.c_str());
        return Re::FileWrite;
    }
    if (out_size != nullptr)
        *out_size = write_size;
    return Re::Success;
}
Re PersistFileIoHandler::ReadFile(int size, char *data, size_t *out_size) {
    if (!IsAssociated()) {
        DebugPrint("PersistFileIoHandler:read file failed,because no file is associated with handler\n");
        return Re::FileAssociated;
    }
    if (fflush(file_) != 0) {
        DebugPrint("PersistFileIoHandler:read file %s failed,because flush failed %s\n", file_path_.c_str(),
                   strerror(errno));
        return Re::FileError;
    }
    size_t read_size = fread(data, 1, size, file_);
    if (read_size != size) {
        DebugPrint("PersistFileIoHandler:read file %s failed.\n", file_path_.c_str());
        return Re::FileRead;
    }
    if (out_size != nullptr)
        *out_size = read_size;
    return Re::Success;
}
Re PersistFileIoHandler::ReadAt(long offset, int size, char *data, size_t *out_size) {
    if (!IsAssociated()) {
        DebugPrint("PersistFileIoHandler:read file with offset %ld failed,because no file is associated with handler\n",
                   offset);
        return Re::FileAssociated;
    }
    if (fflush(file_) != 0) {
        DebugPrint("PersistFileIoHandler:read file %s with offset %ld failed,because flush failed %s\n",
                   file_path_.c_str(), offset, strerror(errno));
        return Re::FileError;
    }
    if (fseek(file_, offset, SEEK_SET) != 0) {
        DebugPrint("PersistFileIoHandler:read file %s with offset %ld failed,because seek failed\n", file_path_.c_str(),
                   offset);
        return Re::FileSeek;
    }
    size_t read_size = fread(data, 1, size, file_);
    if (read_size != size) {
        DebugPrint("PersistFileIoHandler:read file %s with offset %ld failed.\n", file_path_.c_str(), offset);
        return Re::FileRead;
    }
    if (out_size != nullptr)
        *out_size = read_size;
    return RecordEof;
}
Re PersistFileIoHandler::Seek(long offset) {
    if (!IsAssociated()) {
        DebugPrint("PersistFileIoHandler:seek file %s with offset %ld failed,because no file is associated with "
                   "handler\n");
        return Re::FileAssociated;
    }
    if (fseek(file_, offset, SEEK_SET) != 0) {
        DebugPrint("PersistFileIoHandler:seek file %s with offset %ld failed.\n", file_path_.c_str(), offset);
        return Re::FileSeek;
    }
    return Re::Success;
}