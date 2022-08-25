package com.infomaximum.system.service;

import com.infomaximum.system.LibraryLoader;
import com.infomaximum.system.NativeReference;
import com.infomaximum.system.SystemException;

public class Service extends NativeReference {

    public static class Status
    {
        public final boolean isRunning;
        public final int exitCode;

        public Status(boolean isRunning, int exitCode) {
            this.isRunning = isRunning;
            this.exitCode = exitCode;
        }
    }

    public enum AccountType
    {
        LocalSystem(0),
        LocalService(1),
        VirtualAccount(2);

        private int value;

        AccountType(int value) {
            this.value = value;
        }

        int getValue() {
            return value;
        }
    }

    public enum OpenAccessRights
    {
        AllAccess(0),
        ReadWrite(1),
        ExecuteOnly(2),
        ReadOnly(3);

        private int value;

        OpenAccessRights(int value) {
            this.value = value;
        }

        int getValue() {
            return value;
        }
    }

    private Service(long nativePointer) {
        super(nativePointer);
    }

    public static Service create(String name, String executablePath, AccountType accountType) throws SystemException {
        LibraryLoader.getInstance().load();
        return new Service(create(name, executablePath, accountType.getValue()));
    }

    public static Service open(String name, OpenAccessRights accessRights) throws SystemException {
        LibraryLoader.getInstance().load();
        return new Service(open(name, accessRights.getValue()));
    }

    public static boolean exists(String name) throws SystemException {
        LibraryLoader.getInstance().load();
        return existsService(name);
    }

    public static void remove(String name) throws SystemException {
        LibraryLoader.getInstance().load();
        removeService(name);
    }

    public void start() throws SystemException {
        start(nativePointer);
    }

    public void startAsync() throws SystemException {
        startAsync(nativePointer);
    }

    public void stop() throws SystemException {
        stop(nativePointer);
    }

    public Status getStatus() throws SystemException {
        boolean[] isRunning = new boolean[1];
        int[] exitCode = new int[1];
        getStatus(nativePointer, isRunning, exitCode);
        return new Status(isRunning[0], exitCode[0]);
    }

    public void setAutoRestart(boolean autoRestart, String postFailCommand) throws SystemException {
        setAutoRestart(nativePointer, autoRestart, postFailCommand);
    }

    public String getDescription() throws SystemException {
        return getDescription(nativePointer);
    }

    public void setDescription(String description) throws SystemException {
        setDescription(nativePointer, description);
    }

    public void sendCode(int userCode) throws SystemException {
        sendCode(nativePointer, userCode);
    }

    public long getPid() throws SystemException {
        return getPid(nativePointer);
    }

    public String getExecutablePath() throws SystemException {
        return getExecutablePath(nativePointer);
    }

    @Override
    public void close() throws SystemException {
        destroy(nativePointer);
    }

    protected native static long create(final String name, final String executablePath, final int accountType) throws SystemException;
    protected native static long open(final String name, final int accessRights) throws SystemException;
    protected native static boolean existsService(final String name) throws SystemException;
    protected native static void removeService(final String name) throws SystemException;

    protected native static void start(final long nativePointer) throws SystemException;
    protected native static void startAsync(final long nativePointer) throws SystemException;
    protected native static void stop(final long nativePointer) throws SystemException;
    protected native static void getStatus(final long nativePointer, boolean[] isRunning, int[] exitCode) throws SystemException;
    protected native static void setAutoRestart(final long nativePointer, final boolean autoRestart, final String postFailCommand) throws SystemException;
    protected native static String getDescription(final long nativePointer) throws SystemException;
    protected native static void setDescription(final long nativePointer, final String description) throws SystemException;
    protected native static void sendCode(final long nativePointer, final int userCode) throws SystemException;
    protected native static long getPid(final long nativePointer) throws SystemException;
    protected native static String getExecutablePath(final long nativePointer) throws SystemException;
    protected native static void destroy(final long nativePointer) throws SystemException;
}
