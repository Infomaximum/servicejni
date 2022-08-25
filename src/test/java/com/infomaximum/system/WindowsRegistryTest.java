package com.infomaximum.system;

import com.infomaximum.system.registry.WindowsRegistry;
import com.infomaximum.system.utils.SystemUtils;
import org.junit.After;
import org.junit.Assert;
import org.junit.Test;

public class WindowsRegistryTest {

    private final String KEY_TEST = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\_test_key_";

    @After
    public void afterTest() throws SystemException {
        if (SystemUtils.IS_OS_WINDOWS) {
            WindowsRegistry.remove(WindowsRegistry.HKEY.LocalMachine, KEY_TEST);
        }
    }

    @Test
    public void run() throws Exception {
        if (!SystemUtils.IS_OS_WINDOWS) {
            return;
        }

        try (WindowsRegistry registry = WindowsRegistry.open(WindowsRegistry.HKEY.LocalMachine, "_unknown_reg", WindowsRegistry.AccessRights.Read)) {
            Assert.fail();
        } catch (SystemException e) {
            Assert.assertTrue(true);
        }

        try (WindowsRegistry registry = WindowsRegistry.open(WindowsRegistry.HKEY.LocalMachine, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", WindowsRegistry.AccessRights.Read)) {
            try {
                registry.setValue("name", 50);
                Assert.fail();
            } catch (SystemException e) {
                Assert.assertTrue(true);
            }
        }

        try (WindowsRegistry registry = WindowsRegistry.create(WindowsRegistry.HKEY.LocalMachine, KEY_TEST, WindowsRegistry.AccessRights.AllAccess)) {
            registry.setValue("integer", 10);
            registry.setValue("string", "русская строка");
        }

        WindowsRegistry.remove(WindowsRegistry.HKEY.LocalMachine, KEY_TEST);
        WindowsRegistry.remove(WindowsRegistry.HKEY.LocalMachine, KEY_TEST);
    }
}
