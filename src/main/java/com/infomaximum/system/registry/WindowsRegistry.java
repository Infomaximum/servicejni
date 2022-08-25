package com.infomaximum.system.registry;

import com.infomaximum.system.*;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;
import java.util.Arrays;

public class WindowsRegistry extends NativeReference {

    public enum HKEY {
        CurrentUser(0x80000001),
        LocalMachine(0x80000002),
        Users(0x80000003);

        private long value;

        HKEY(long value) {
            this.value = value;
        }

        long getValue() {
            return value;
        }
    }

    public enum AccessRights {
        AllAccess(0xF003F|0x0100),
        Write(0x20006|0x0100),
        Read(0x20019|0x0100);

        private int value;

        AccessRights(int value) {
            this.value = value;
        }

        int getValue() {
            return value;
        }
    }

    private WindowsRegistry(long nativePointer) {
        super(nativePointer);
    }

    public static WindowsRegistry create(HKEY hkey, String key, AccessRights access) throws SystemException {
        LibraryLoader.getInstance().load();
        return new WindowsRegistry(create(hkey.getValue(), key, access.getValue()));
    }

    public static WindowsRegistry open(HKEY hkey, String key, AccessRights access) throws SystemException {
        LibraryLoader.getInstance().load();
        return new WindowsRegistry(open(hkey.getValue(), key, access.getValue()));
    }

    public static void remove(HKEY hkey, String key) throws SystemException {
        LibraryLoader.getInstance().load();
        remove(hkey.getValue(), key);
    }

    public void setValue(String name, String value) throws SystemException {
        final int REG_SZ = 1;
        byte[] temp;
        if (value == null || value.isEmpty()) {
            temp = new byte[0];
        } else {
            temp = value.getBytes(Charset.forName("UTF-16LE"));
        }
        byte[] wcharBytes = Arrays.copyOf(temp, temp.length + 2);
        wcharBytes[wcharBytes.length - 2] = 0;
        wcharBytes[wcharBytes.length - 1] = 0;
        setValue(nativePointer, name, REG_SZ, wcharBytes);
    }

    public void setValue(String name, int value) throws SystemException {
        final int REG_DWORD = 4;
        byte[] val = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(value).array();
        setValue(nativePointer, name, REG_DWORD, val);
    }

    @Override
    public void close() throws SystemException {
        destroy(nativePointer);
    }

    protected native static long create(final long hkey, final String key, int access) throws SystemException;
    protected native static long open(final long hkey, final String key, int access) throws SystemException;
    protected native static void remove(final long hkey, final String key) throws SystemException;

    protected native static void setValue(final long nativePointer, final String name, int type, byte[] value) throws SystemException;
    protected native static void destroy(final long nativePointer) throws SystemException;
}
