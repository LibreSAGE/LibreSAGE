#include "qfilewrapper.h"
#include <QDebug>

QFileWrapper::QFileWrapper(char const *filename)
{
    Set_Name(filename);
}

QFileWrapper::~QFileWrapper(void)
{
}

int QFileWrapper::Read(void *buffer, int size)
{
    if (!File.isOpen()) return 0;
    return File.read(static_cast<char *>(buffer), size);
}

int QFileWrapper::Seek(int pos, int dir)
{
    if (!File.isOpen()) return -1;

    qint64 newPos;
    switch (dir) {
        case SEEK_SET:
            newPos = pos;
            break;
        case SEEK_CUR:
            newPos = File.pos() + pos;
            break;
        case SEEK_END:
            newPos = File.size() + pos;
            break;
        default:
            return -1; // Invalid direction
    }

    if (newPos < 0) {
        newPos = 0; // Prevent seeking before the beginning of the file
    } else if (newPos > File.size()) {
        newPos = File.size(); // Prevent seeking beyond the end of the file
    }

    File.seek(newPos);
    return static_cast<int>(File.pos());
}

int QFileWrapper::Size(void)
{
    if (!File.isOpen()) return -1;
    return static_cast<int>(File.size());
}

int QFileWrapper::Write(void const * /*buffer*/, int /*size*/)
{
    return 0; // Writing is not supported in this wrapper
}

void QFileWrapper::Error(int error, int canretry, char const *filename)
{
    qDebug() << "Error:" << error << "on file:" << filename << "Can retry:" << canretry;
}