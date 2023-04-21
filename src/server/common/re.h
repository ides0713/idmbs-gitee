//
// Created by ubuntu on 4/2/23.
//
#pragma once

#define RE_SIMPLE_STR(re) #re

enum ReBufferPool {
    BpExist = 1,
    BpFileErr,
    BpInvalidName,
    BpWindows,
    BpClosed,
    BpOpen,
    BpNoBuf,
    BpEof,
    BpInvalidPageId,
    BpNotInBuf,
    BpPagePinned,
    BpOpenTooManyFiles,
    BpIllegalFileId,
};

enum ReRecord {
    RdClosed = 1,
    RdOpened,
    RdInvalidRecSize,
    RdInvalidRId,
    RdNoMoreRecInMem,
    RdOpen,
    RdNoMoreIdxInMem,
    RdInvalidKey,
    RdDuplicateKey,
    RdNoMem,
    RdScanClosed,
    RdScanOpened,
    RdEof,
    RdNotExist,
};

enum ReSchema {
    DbExist = 1,
    DbNotExist,
    DbNotOpened,
    TableNotExist,
    TableExist,
    TableNameIllegal,
    FieldNotExist,
    FieldExist,
    FieldNameIllegal,
    FieldMissing,
    FieldRedundant,
    FieldTypeMismatch,
    IndexNameRepeat,
    IndexExist,
    IndexNotExist,
    IndexNameIllegal,
};

enum ReSql {
    SqlSelect = 1
};

enum ReIoError {
    Read = 1,
    ShortRead,
    Write,
    Fsync,
    DirFSync,
    Truncate,
    Fstat,
    IoDelete,
    Blocked,
    Access,
    CheckReservedLock,
    Close,
    DirClose,
    ShmOpen,
    ShmSize,
    ShmLock,
    ShmMap,
    Seek,
    DeleteNoEnt,
    MMap,
    GetTempPath,
    IoConvPath,
    VNode,
    BeginAtomic,
    CommitAtomic,
    RollBackAtomic,
    Data,
    CorruptFs,
    OpenTooManyFiles,
};

enum ReLock {
    Lock = 1,
    Unlock,
    SharedCache,
    LVirt,
    NeedWait,
    ResourceDeleted,
};

enum ReBusy {
    BRecovery = 1,
    Snapshot,
    Timeout,
};

enum ReCantOpen {
    NotEmpDir = 1,
    Isdir,
    FullPath,
    CoConvPath,
    DirtyWal,
    Symlink,
};

enum ReCorrupt {
    CorruptVirt = 1,
    CorruptSequence,
    CorruptIndex
};

enum ReReadOnly {
    RoRecovery = 1,
    CantLock,
    RoRollBack,
    DbMoved,
    CantInit,
    Directory,
};

enum ReAbort {
    ARollBack = 1,
};

enum ReConstraint {
    Check = 1,
    CommitHook,
    Foreignkey,
    Function,
    NotNull,
    PrimaryKey,
    Trigger,
    Unique,
    CVirt,
    RowId,
    Pinned,
};

enum ReNotice {
    RecoverWal = 1,
    RecoverRollBack,
    AutoIndex,
};

enum ReAuth {
    User = 1,
};

enum ReFile {
    FExist = 1,
    FNotExist,
    FName,
    FBound,
    FCreate,
    FOpen,
    FNotOpened,
    FClose,
    FRemove,
    FSeek,
    FRead,
    FWrite,
};

enum ReLogBuf {
    LbFull = 1,
    LbEmpty,
};

enum Re {
    Success = 0, /* Successful result */
    /* beginning-of-error-codes */
    GenericError,    /* Generic error */
    InvalidArgument, /* Invalid argument */
    NotImplement,    /* not implement yet */
    SqlSyntax,       /* SQL Syntax error */
    BufferPool,      /* Buffer pool error*/
    Record,          /* Record error */
    Internal,        /* Internal logic error in SQL */
    Permission,      /* Access permission denied */
    Abort,           /* Callback routine requested an abort */
    Busy,            /* The database file is locked */
    Locked,          /* A table in the database is locked */
    NoMem,           /* A malloc() failed */
    ReadOnly,        /* Attempt to write a readonly database */
    Interrupt,       /* Operation terminated by interrupt()*/
    IoErr,           /* Some kind of disk I/O error occurred */
    Corrupt,         /* The database disk image is malformed */
    NotFound,        /* Unknown opcode in file control() */
    Full,            /* Insertion failed because database is full */
    CantOpen,        /* Unable to open the database file */
    Protocol,        /* Database lock protocol error */
    Empty,           /* Internal use only */
    Schema,          /* The database schema error */
    TooBig,          /* String or BLOB exceeds size limit */
    Constraint,      /* Abort due to constraint violation */
    Mismatch,        /* Data type mismatch */
    Misuse,          /* Library used incorrectly */
    NoLfs,           /* Uses OS features not supported on host */
    Auth,            /* Authorization denied */
    Format,          /* Not used */
    Range,           /* 2nd parameter to bind out of range */
    NotADb,          /* File opened that is not a database file */
    FileError,       /* File error */
    LogBuf,          /* clog buffer error */
    Notice = 100,    /* Notifications from log() */

    /* buffer pool part */
    BufferPoolExist = (BufferPool | (ReBufferPool::BpExist << 8)),
    BufferPoolFileErr = (BufferPool | (ReBufferPool::BpFileErr << 8)),
    BufferPoolInvalidName = (BufferPool | (ReBufferPool::BpInvalidName << 8)),
    BufferPoolWindows = (BufferPool | (ReBufferPool::BpWindows << 8)),
    BufferPoolClosed = (BufferPool | (ReBufferPool::BpClosed << 8)),
    BufferPoolOpen = (BufferPool | (ReBufferPool::BpOpen << 8)),
    BufferPoolNoBuf = (BufferPool | (ReBufferPool::BpNoBuf << 8)),
    BufferPoolEof = (BufferPool | (ReBufferPool::BpEof << 8)),
    BufferPoolInvalidPageId = (BufferPool | (ReBufferPool::BpInvalidPageId << 8)),
    BufferPoolNotInBuf = (BufferPool | (ReBufferPool::BpNotInBuf << 8)),
    BufferPoolPagePinned = (BufferPool | (ReBufferPool::BpPagePinned << 8)),
    BufferPoolOpenTooManyFiles = (BufferPool | (ReBufferPool::BpOpenTooManyFiles << 8)),
    BufferPoolIllegalFileId = (BufferPool | (ReBufferPool::BpIllegalFileId << 8)),

    /* record part */
    RecordClosed = (Record | (ReRecord::RdClosed << 8)),
    RecordOpened = (Record | (ReRecord::RdOpened << 8)),
    RecordInvalidRecSize = (Record | (ReRecord::RdInvalidRecSize << 8)),
    RecordInvalidRId = (Record | (ReRecord::RdInvalidRId << 8)),
    RecordNoMoreRecInMem = (Record | (ReRecord::RdNoMoreRecInMem << 8)),
    RecordOpen = (Record | (ReRecord::RdOpen << 8)),
    RecordNoMoreIdxInMem = (Record | (ReRecord::RdNoMoreIdxInMem << 8)),
    RecordInvalidKey = (Record | (ReRecord::RdInvalidKey << 8)),
    RecordDuplicateKey = (Record | (ReRecord::RdDuplicateKey << 8)),
    RecordNoMem = (Record | (ReRecord::RdNoMem << 8)),
    RecordScanClosed = (Record | (ReRecord::RdScanClosed << 8)),
    RecordScanOpened = (Record | (ReRecord::RdScanOpened << 8)),
    RecordEof = (Record | (ReRecord::RdEof << 8)),
    RecordRecordNotExist = (Record | (ReRecord::RdNotExist << 8)),

    /* schema part */
    SchemaDbExist = (Schema | (ReSchema::DbExist << 8)),
    SchemaDbNotExist = (Schema | (ReSchema::DbNotExist << 8)),
    SchemaDbNotOpened = (Schema | (ReSchema::DbNotOpened << 8)),
    SchemaTableNotExist = (Schema | (ReSchema::TableNotExist << 8)),
    SchemaTableExist = (Schema | (ReSchema::TableExist << 8)),
    SchemaTableNameIllegal = (Schema | (ReSchema::TableNameIllegal << 8)),
    SchemaFieldNotExist = (Schema | (ReSchema::FieldNotExist << 8)),
    SchemaFieldExist = (Schema | (ReSchema::FieldExist << 8)),
    SchemaFieldNameIllegal = (Schema | (ReSchema::FieldNameIllegal << 8)),
    SchemaFieldMissing = (Schema | (ReSchema::FieldMissing << 8)),
    SchemaFieldRedundant = (Schema | (ReSchema::FieldRedundant << 8)),
    SchemaFieldTypeMismatch = (Schema | (ReSchema::FieldTypeMismatch << 8)),
    SchemaIndexNameRepeat = (Schema | (ReSchema::IndexNameRepeat << 8)),
    SchemaIndexExist = (Schema | (ReSchema::IndexExist << 8)),
    SchemaIndexNotExist = (Schema | (ReSchema::IndexNotExist << 8)),
    SchemaIndexNameIllegal = (Schema | (ReSchema::IndexNameIllegal << 8)),

    /*io error part*/
    IoErrRead = (IoErr | (ReIoError::Read << 8)),
    IoErrShortRead = (IoErr | (ReIoError::ShortRead << 8)),
    IoErrWrite = (IoErr | (ReIoError::Write << 8)),
    IoErrFsync = (IoErr | (ReIoError::Fsync << 8)),
    IoErrDirFSync = (IoErr | (ReIoError::DirFSync << 8)),
    IoErrTruncate = (IoErr | (ReIoError::Truncate << 8)),
    IoErrFStat = (IoErr | (ReIoError::Fstat << 8)),
    IoErrDelete = (IoErr | (ReIoError::IoDelete << 8)),
    IoErrBlocked = (IoErr | (ReIoError::Blocked << 8)),
    IoErrAccess = (IoErr | (ReIoError::Access << 8)),
    IoErrCheckReservedLock = (IoErr | (ReIoError::CheckReservedLock << 8)),
    IoErrClose = (IoErr | (ReIoError::Close << 8)),
    IoErrDirClose = (IoErr | (ReIoError::DirClose << 8)),
    IoErrShmOpen = (IoErr | (ReIoError::ShmOpen << 8)),
    IoErrShmSize = (IoErr | (ReIoError::ShmSize << 8)),
    IoErrShmLock = (IoErr | (ReIoError::ShmLock << 8)),
    IoErrShmMap = (IoErr | (ReIoError::ShmMap << 8)),
    IoErrSeek = (IoErr | (ReIoError::Seek << 8)),
    IoErrDeleteNoEnt = (IoErr | (ReIoError::DeleteNoEnt << 8)),
    IoErrMMap = (IoErr | (ReIoError::MMap << 8)),
    IoErrGetTempPath = (IoErr | (ReIoError::GetTempPath << 8)),
    IoErrConvPath = (IoErr | (ReIoError::IoConvPath << 8)),
    IoErrVNode = (IoErr | (ReIoError::VNode << 8)),
    IoErrBeginAtomic = (IoErr | (ReIoError::BeginAtomic << 8)),
    IoErrCommitAtomic = (IoErr | (ReIoError::CommitAtomic << 8)),
    IoErrRollbackAtomic = (IoErr | (ReIoError::RollBackAtomic << 8)),
    IoErrData = (IoErr | (ReIoError::Data << 8)),
    IoErrCorruptFs = (IoErr | (ReIoError::CorruptFs << 8)),
    IoErrOpenTooManyFiles = (IoErr | ReIoError::OpenTooManyFiles << 8),

    /* Lock part*/
    LockedLock = (Locked | (ReLock::Lock << 8)),
    LockedUnlock = (Locked | (ReLock::Unlock << 8)),
    LockedSharedCache = (Locked | (ReLock::SharedCache << 8)),
    LockedVirt = (Locked | (ReLock::LVirt << 8)),
    LockedNeedWait = (Locked | (ReLock::NeedWait << 8)),
    LockedResourceDeleted = (Locked | (ReLock::ResourceDeleted << 8)),

    /* busy part */
    BusyRecovery = (Busy | (ReBusy::BRecovery << 8)),
    BusySnapShot = (Busy | (ReBusy::Snapshot << 8)),
    BusyTimeOut = (Busy | (ReBusy::Timeout << 8)),

    /* Can't open part */
    CantOpenNotEmptyDir = (CantOpen | (ReCantOpen::NotEmpDir << 8)),
    CantOpenIsDir = (CantOpen | (ReCantOpen::Isdir << 8)),
    CantOpenFullPath = (CantOpen | (ReCantOpen::FullPath << 8)),
    CantOpenConvPath = (CantOpen | (ReCantOpen::CoConvPath << 8)),
    CantOpenDirtyWal = (CantOpen | (ReCantOpen::DirtyWal << 8)),
    CantOpenSymlink = (CantOpen | (ReCantOpen::Symlink << 8)),

    /* corrupt part */// compile error
    // CorruptVirt = (CORRUPT | (RECorrupt::CorruptVirt << 8)),
    // CorruptSequence = (CORRUPT | (RECorrupt::CorruptSequence << 8)),
    // CorruptIndex = (CORRUPT | (RECorrupt::CorruptIndex << 8)),

    /*readonly part*/
    ReadOnlyRecovery = (ReadOnly | (ReReadOnly::RoRecovery << 8)),
    ReadOnlyCantLock = (ReadOnly | (ReReadOnly::CantLock << 8)),
    ReadonlyRollback = (ReadOnly | (ReReadOnly::RoRollBack << 8)),
    ReadOnlyDbMoved = (ReadOnly | (ReReadOnly::DbMoved << 8)),
    ReadOnlyCantInit = (ReadOnly | (ReReadOnly::CantInit << 8)),
    ReadOnlyDirectory = (ReadOnly | (ReReadOnly::Directory << 8)),

    AbortRollBack = (Abort | (ReAbort::ARollBack << 8)),

    /* constraint part */
    ConstraintCheck = (Constraint | (ReConstraint::Check << 8)),
    ConstraintCommitHook = (Constraint | (ReConstraint::CommitHook << 8)),
    ConstraintForeignKey = (Constraint | (ReConstraint::Foreignkey << 8)),
    ConstraintFunction = (Constraint | (ReConstraint::Function << 8)),
    ConstraintNotNull = (Constraint | (ReConstraint::NotNull << 8)),
    ConstraintPrimaryKey = (Constraint | (ReConstraint::PrimaryKey << 8)),
    ConstraintTrigger = (Constraint | (ReConstraint::Trigger << 8)),
    ConstraintUnique = (Constraint | (ReConstraint::Unique << 8)),
    ConstraintVirt = (Constraint | (ReConstraint::CVirt << 8)),
    ConstraintRowid = (Constraint | (ReConstraint::RowId << 8)),
    ConstraintPinned = (Constraint | (ReConstraint::Pinned << 8)),

    /* notice part */
    NoticeRecoverWal = (Notice | (ReNotice::RecoverWal << 8)),
    NoticeRecoverRollback = (Notice | (ReNotice::RecoverRollBack << 8)),
    NoticeAutoIndex = (Notice | (ReNotice::AutoIndex << 8)),

    /* file part */
    FileExist = (FileError | (ReFile::FExist << 8)),
    FileNotExist = (FileError | (ReFile::FNotExist << 8)),
    FileName = (FileError | (ReFile::FName << 8)),
    FileAssociated = (FileError | (ReFile::FBound << 8)),
    FileCreate = (FileError | (ReFile::FCreate << 8)),
    FileOpen = (FileError | (ReFile::FOpen << 8)),
    FileNotOpened = (FileError | (ReFile::FNotOpened << 8)),
    FileClose = (FileError | (ReFile::FClose << 8)),
    FileRemove = (FileError | (ReFile::FRemove << 8)),
    FileSeek = (FileError | (ReFile::FSeek << 8)),
    FileRead = (FileError | (ReFile::FRead << 8)),
    FileWrite = (FileError | (ReFile::FWrite << 8)),

    /* auth part*/
    AuthUser = (Auth | (ReAuth::User << 8)),

    /* clog buffer part */
    LogBufFull = (LogBuf | (ReLogBuf::LbFull << 8)),
    LogBufEmpty = (LogBuf | (ReLogBuf::LbEmpty << 8)),
};

const char *StrRe(Re re);
