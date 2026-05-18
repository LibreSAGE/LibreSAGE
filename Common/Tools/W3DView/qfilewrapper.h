#pragma once
#include "wwfile.h"
#include <QFile>

// A wrapper for QFile to be used as a FileClass in the WW3D engine.
// This allows us to read files from Qt's resource system, which is useful for loading assets that are compiled into the application
class QFileWrapper : public FileClass
{
public:
    QFileWrapper(char const *filename);
    virtual ~QFileWrapper(void);

    virtual char const *File_Name(void) const { return File.fileName().toUtf8().constData(); }
    virtual char const *Set_Name(char const *filename) { File.setFileName(filename); return File_Name(); }
    virtual int Create(void) { return false; }
    virtual int Delete(void) { return File.remove(); }
    virtual bool Is_Available(int /*forced=false*/) { return File.exists(); }
    virtual bool Is_Open(void) const { return File.isOpen(); }

    virtual int Open(char const * /*fname*/, int /*rights=READ*/) { return File.open(QIODevice::ReadOnly); }
    virtual int Open(int /*rights=READ*/) { return File.open(QIODevice::ReadOnly); }

    virtual int Read(void *buffer, int size);
    virtual int Seek(int pos, int dir = SEEK_CUR);
    virtual int Size(void);
    virtual int Write(void const * /*buffer*/, int /*size*/);
    virtual void Close(void) { File.close(); }
    virtual void Error(int error, int canretry = false, char const *filename = NULL);

    virtual unsigned char *Peek_Data(void) const { return NULL; }

protected:
    QFile File;
};