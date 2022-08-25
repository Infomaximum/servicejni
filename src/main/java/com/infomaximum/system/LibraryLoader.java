package com.infomaximum.system;

import com.infomaximum.system.utils.SystemUtils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

public class LibraryLoader {

    private static final LibraryLoader instance = new LibraryLoader();
    private static volatile boolean initialized = false;

    private static final String SOURCE_FILENAME = "servicelib64.dll";

    private static final String TEMP_DLL_FILE = "infomaximum_servicelib";
    private static final String TEMP_DLL_SUFFIX = ".dll";

    private File tempDllFile;

    private LibraryLoader() {}

    public static LibraryLoader getInstance() {
        return instance;
    }

    public synchronized void load() {
        if (!SystemUtils.IS_OS_WINDOWS) {
            throw new RuntimeException();
        }
        if (!initialized) {
            try {
                copyLibraryFromResourcesToTemp();
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
            System.load(tempDllFile.getAbsolutePath());
            initialized = true;
        }
    }

    private void copyLibraryFromResourcesToTemp() throws IOException {
        if (SystemUtils.IS_OS_WINDOWS) {
            String tmpDirPath = System.getProperty("java.io.tmpdir");
            String mask = TEMP_DLL_FILE + ".*" + TEMP_DLL_SUFFIX;
            removeFilesByMask(tmpDirPath, mask);
        }
        tempDllFile = File.createTempFile(TEMP_DLL_FILE, TEMP_DLL_SUFFIX);
        tempDllFile.deleteOnExit();
        try (InputStream is = getClass().getClassLoader().getResourceAsStream(SOURCE_FILENAME)) {
            if (is == null) {
                throw new RuntimeException(SOURCE_FILENAME + " not found into resources.");
            }
            Files.copy(is, tempDllFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
        }
    }

    private static void removeFilesByMask(final String dirPath, final String mask) {
        File dir = new File(dirPath);
        File[] files = dir.listFiles((dir1, name) -> name.matches(mask));
        if (files != null) {
            for (File file : files) {
                try {
                    file.delete();
                } catch (Throwable ignored) {

                }
            }
        }
    }
}
