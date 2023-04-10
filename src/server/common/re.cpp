//
// Created by ubuntu on 4/2/23.
//
#include "re.h"

#define RE_CASE_STRING(re) \
  case re:                 \
    return #re

const char *strRe(Re re) {
    switch (re) {
        RE_CASE_STRING(Success);
        RE_CASE_STRING(GenericError);
        RE_CASE_STRING(InvalidArgument);
        RE_CASE_STRING(NotImplement);
        RE_CASE_STRING(SqlSyntax);
        RE_CASE_STRING(BufferPool);
        RE_CASE_STRING(Record);
        RE_CASE_STRING(Internal);
        RE_CASE_STRING(Permission);
        RE_CASE_STRING(Abort);
        RE_CASE_STRING(Busy);
        RE_CASE_STRING(Locked);
        RE_CASE_STRING(NoMem);
        RE_CASE_STRING(ReadOnly);
        RE_CASE_STRING(Interrupt);
        RE_CASE_STRING(IoErr);
        RE_CASE_STRING(Corrupt);
        RE_CASE_STRING(NotFound);
        RE_CASE_STRING(Full);
        RE_CASE_STRING(CantOpen);
        RE_CASE_STRING(Protocol);
        RE_CASE_STRING(Empty);
        RE_CASE_STRING(Schema);
        RE_CASE_STRING(TooBig);
        RE_CASE_STRING(Constraint);
        RE_CASE_STRING(Mismatch);
        RE_CASE_STRING(Misuse);
        RE_CASE_STRING(NoLfs);
        RE_CASE_STRING(Auth);
        RE_CASE_STRING(Format);
        RE_CASE_STRING(Range);
        RE_CASE_STRING(NotADb);
        RE_CASE_STRING(LogBuf);
        RE_CASE_STRING(Notice);

        RE_CASE_STRING(BufferPoolExist);
        RE_CASE_STRING(BufferPoolFileErr);
        RE_CASE_STRING(BufferPoolInvalidName);
        RE_CASE_STRING(BufferPoolWindows);
        RE_CASE_STRING(BufferPoolClosed);
        RE_CASE_STRING(BufferPoolOpen);
        RE_CASE_STRING(BufferPoolNoBuf);
        RE_CASE_STRING(BufferPoolEof);
        RE_CASE_STRING(BufferPoolInvalidPageId);
        RE_CASE_STRING(BufferPoolNotInBuf);
        RE_CASE_STRING(BufferPoolPagePinned);
        RE_CASE_STRING(BufferPoolOpenTooManyFiles);
        RE_CASE_STRING(BufferPoolIllegalFileId);

        RE_CASE_STRING(RecordClosed);
        RE_CASE_STRING(RecordOpened);
        RE_CASE_STRING(RecordInvalidRecSize);
        RE_CASE_STRING(RecordInvalidRId);
        RE_CASE_STRING(RecordNoMoreRecInMem);
        RE_CASE_STRING(RecordOpen);
        RE_CASE_STRING(RecordNoMoreIdxInMem);
        RE_CASE_STRING(RecordInvalidKey);
        RE_CASE_STRING(RecordDuplicateKey);
        RE_CASE_STRING(RecordNoMem);
        RE_CASE_STRING(RecordScanClosed);
        RE_CASE_STRING(RecordScanOpened);
        RE_CASE_STRING(RecordEof);
        RE_CASE_STRING(RecordRecordNotExist);

        RE_CASE_STRING(SchemaDbExist);
        RE_CASE_STRING(SchemaDbNotExist);
        RE_CASE_STRING(SchemaDbNotOpened);
        RE_CASE_STRING(SchemaTableNotExist);
        RE_CASE_STRING(SchemaTableExist);
        RE_CASE_STRING(SchemaTableNameIllegal);
        RE_CASE_STRING(SchemaFieldNotExist);
        RE_CASE_STRING(SchemaFieldExist);
        RE_CASE_STRING(SchemaFieldNameIllegal);
        RE_CASE_STRING(SchemaFieldMissing);
        RE_CASE_STRING(SchemaFieldRedundant);
        RE_CASE_STRING(SchemaFieldTypeMismatch);
        RE_CASE_STRING(SchemaIndexNameRepeat);
        RE_CASE_STRING(SchemaIndexExist);
        RE_CASE_STRING(SchemaIndexNotExist);
        RE_CASE_STRING(SchemaIndexNameIllegal);

        RE_CASE_STRING(IoErrRead);
        RE_CASE_STRING(IoErrShortRead);
        RE_CASE_STRING(IoErrWrite);
        RE_CASE_STRING(IoErrFsync);
        RE_CASE_STRING(IoErrDirFSync);
        RE_CASE_STRING(IoErrTruncate);
        RE_CASE_STRING(IoErrFStat);
        RE_CASE_STRING(IoErrDelete);
        RE_CASE_STRING(IoErrBlocked);
        RE_CASE_STRING(IoErrAccess);
        RE_CASE_STRING(IoErrCheckReservedLock);
        RE_CASE_STRING(IoErrClose);
        RE_CASE_STRING(IoErrDirClose);
        RE_CASE_STRING(IoErrShmOpen);
        RE_CASE_STRING(IoErrShmSize);
        RE_CASE_STRING(IoErrShmLock);
        RE_CASE_STRING(IoErrShmMap);
        RE_CASE_STRING(IoErrSeek);
        RE_CASE_STRING(IoErrDeleteNoEnt);
        RE_CASE_STRING(IoErrMMap);
        RE_CASE_STRING(IoErrGetTempPath);
        RE_CASE_STRING(IoErrConvPath);
        RE_CASE_STRING(IoErrVNode);
        RE_CASE_STRING(IoErrBeginAtomic);
        RE_CASE_STRING(IoErrCommitAtomic);
        RE_CASE_STRING(IoErrRollbackAtomic);
        RE_CASE_STRING(IoErrData);
        RE_CASE_STRING(IoErrCorruptFs);
        RE_CASE_STRING(IoErrOpenTooManyFiles);

        RE_CASE_STRING(LockedLock);
        RE_CASE_STRING(LockedUnlock);
        RE_CASE_STRING(LockedSharedCache);
        RE_CASE_STRING(LockedVirt);
        RE_CASE_STRING(LockedNeedWait);
        RE_CASE_STRING(LockedResourceDeleted);

        RE_CASE_STRING(BusyRecovery);
        RE_CASE_STRING(BusySnapShot);
        RE_CASE_STRING(BusyTimeOut);

        RE_CASE_STRING(CantOpenNotEmptyDir);
        RE_CASE_STRING(CantOpenIsDir);
        RE_CASE_STRING(CantOpenFullPath);
        RE_CASE_STRING(CantOpenConvPath);
        RE_CASE_STRING(CantOpenDirtyWal);
        RE_CASE_STRING(CantOpenSymlink);

        RE_CASE_STRING(ReadOnlyRecovery);
        RE_CASE_STRING(ReadOnlyCantLock);
        RE_CASE_STRING(ReadonlyRollback);
        RE_CASE_STRING(ReadOnlyDbMoved);
        RE_CASE_STRING(ReadOnlyCantInit);
        RE_CASE_STRING(ReadOnlyDirectory);

        RE_CASE_STRING(AbortRollBack);

        RE_CASE_STRING(ConstraintCheck);
        RE_CASE_STRING(ConstraintCommitHook);
        RE_CASE_STRING(ConstraintForeignKey);
        RE_CASE_STRING(ConstraintFunction);
        RE_CASE_STRING(ConstraintNotNull);
        RE_CASE_STRING(ConstraintPrimaryKey);
        RE_CASE_STRING(ConstraintTrigger);
        RE_CASE_STRING(ConstraintUnique);
        RE_CASE_STRING(ConstraintVirt);
        RE_CASE_STRING(ConstraintRowid);
        RE_CASE_STRING(ConstraintPinned);

        RE_CASE_STRING(NoticeRecoverWal);
        RE_CASE_STRING(NoticeRecoverRollback);
        RE_CASE_STRING(NoticeAutoIndex);

        RE_CASE_STRING(FileExist);
        RE_CASE_STRING(FileNotExist);
        RE_CASE_STRING(FileName);
        RE_CASE_STRING(FileAssociated);
        RE_CASE_STRING(FileCreate);
        RE_CASE_STRING(FileOpen);
        RE_CASE_STRING(FileNotOpened);
        RE_CASE_STRING(FileClose);
        RE_CASE_STRING(FileRemove);
        RE_CASE_STRING(FileSeek);
        RE_CASE_STRING(FileRead);
        RE_CASE_STRING(FileWrite);

        RE_CASE_STRING(AuthUser);

        RE_CASE_STRING(LogBufFull);
        RE_CASE_STRING(LogBufEmpty);
        default: {
            return "UNKNOWN";
        }
    }
}