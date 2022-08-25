package com.infomaximum.system.service;

import com.infomaximum.system.LibraryLoader;
import com.infomaximum.system.NativeReference;
import com.infomaximum.system.SystemException;

public class ServiceEntry extends NativeReference {

    public enum Event
    {
        STARTING(0),
        STOPPING(1);

        private int value;

        Event(int value) {
            this.value = value;
        }

        int getValue() {
            return value;
        }
    }

    @FunctionalInterface
    public interface Handler {

        void apply() throws SystemException;
    }

    private ServiceEntry(long nativePointer) {
        super(nativePointer);
    }

    public ServiceEntry(String serviceName) throws SystemException {
        this(createEntry(serviceName));
    }

    public void registerEventHandler(Event event, Handler handler) throws SystemException {
        registerEventHandler(nativePointer, event.getValue(), handler);
    }

    public void registerEventHandler(int nativeEventCode, Handler handler) throws SystemException {
        registerNativeEventHandler(nativePointer, nativeEventCode, handler);
    }

    public void setExitCode(int exitCode) throws SystemException {
        setExitCode(nativePointer, exitCode);
    }

    public void execute() throws SystemException {
        execute(nativePointer);
    }

    public void exit()  throws SystemException {
        exit(nativePointer);
    }

    @Override
    public void close() throws SystemException {
        destroy(nativePointer);
    }

    private static long createEntry(String serviceName) throws SystemException {
        LibraryLoader.getInstance().load();
        return create(serviceName);
    }

    protected native static long create(final String serviceName) throws SystemException;
    protected native static void registerEventHandler(final long nativePointer, final int event, final Handler handler) throws SystemException;
    protected native static void registerNativeEventHandler(final long nativePointer, final int nativeEvent, final Handler handler) throws SystemException;
    protected native static void setExitCode(final long nativePointer, final int exitCode) throws SystemException;
    protected native static void execute(final long nativePointer) throws SystemException;
    protected native static void exit(final long nativePointer) throws SystemException;
    protected native static void destroy(final long nativePointer) throws SystemException;
}
