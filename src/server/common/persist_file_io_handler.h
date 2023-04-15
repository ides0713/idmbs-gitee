#pragma once

#include "re.h"
#include <cstring>
#include <string>
#include <cstdio>
#include "../../common/common_defs.h"
#include <filesystem>

class PersistFileIoHandler {
public:
    PersistFileIoHandler();

    ~PersistFileIoHandler();

    /** 创建一个名称为指定文件名的文件，并将该文件绑定到当前对象 */
    Re createFile(const char *file_path);

    Re createFile(std::filesystem::path file_path);

    /** 根据文件名打开一个文件并绑定到当前对象，若文件名为空则打开当前文件 */
    Re openFile(const char *file_name);

    /** 关闭当前文件 */
    Re closeFile();

    /**删除当前文件 */
    Re removeFile();

    /** 在当前文件描述符的位置写入一段数据，并返回实际写入的数据大小out_size */
    Re writeFile(int size, const char *data, size_t *out_size);

    /** 在指定位置写入一段数据，并返回实际写入的数据大小out_size */
    Re writeAt(long offset, int size, const char *data, size_t *out_size);

    /** 在文件末尾写入一段数据，并返回实际写入的数据大小out_size */
    Re append(int size, const char *data, size_t *out_size);

    /** 在当前文件描述符的位置读取一段数据，并返回实际读取的数据大小out_size */
    Re readFile(int size, char *data, size_t *out_size);

    /** 在指定位置读取一段数据，并返回实际读取的数据大小out_size */
    Re readAt(long offset, int size, char *data, size_t *out_size);

    /** 将文件描述符移动到指定位置 */
    Re seek(long offset);

private:
    std::string file_path_;
    FILE *file_;
private:
    bool isAssociated() { return (!file_path_.empty() and file_ != nullptr); }
};
