package com.infomaximum.system;

public abstract class NativeReference implements AutoCloseable {

    protected long nativePointer = 0;

    public NativeReference(long nativePointer) {
        this.nativePointer = nativePointer;
    }

    public abstract void close() throws SystemException;
}
