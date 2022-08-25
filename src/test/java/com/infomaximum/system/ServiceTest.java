package com.infomaximum.system;

import com.infomaximum.system.service.Service;
import com.infomaximum.system.test.Main;
import com.infomaximum.system.utils.SystemUtils;
import org.junit.After;
import org.junit.Assert;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.nio.file.Paths;

public class ServiceTest {

    @After
    public void afterTest() throws SystemException {
        Service.remove(Main.SERVICE_NAME);
    }

    @Test
    public void read() throws SystemException {
        if (SystemUtils.IS_OS_WINDOWS) {
            readWin();
        } else {
            Assert.fail("Unrealized test");
        }
    }

    @Test
    public void edit() throws SystemException {
        if (SystemUtils.IS_OS_WINDOWS) {
            editWin();
        } else {
            Assert.fail("Unrealized test");
        }
    }

    @Test
    public void removeOldLibraries() throws SystemException, IOException {
        if (SystemUtils.IS_OS_WINDOWS) {
            String suffix = "64.dll";
            String libraryName = "servicelib";
            File.createTempFile(libraryName, suffix);
            File.createTempFile(libraryName, suffix);
            readWin();
            String mask = libraryName + ".*" + suffix;
            File dir = new File(System.getProperty("java.io.tmpdir"));
            File[] files = dir.listFiles((dir1, name) -> name.matches(mask));
            Assert.assertEquals(files.length, 1);
        }
    }

    private void readWin() throws SystemException {
        try (Service service = Service.open("DiagTrack", Service.OpenAccessRights.ReadOnly)) {
            Assert.assertEquals("The Diagnostics Tracking Service enables data collection about functional issues in Windows components.",
                    service.getDescription());
            Assert.assertEquals("C:\\Windows\\System32\\svchost.exe -k utcsvc", service.getExecutablePath());
            Assert.assertTrue(service.getPid() > 0);

            Service.Status status = service.getStatus();
            Assert.assertTrue(status.isRunning);
            Assert.assertEquals(0, status.exitCode);

            try {
                service.stop();
                Assert.fail();
            } catch (SystemException e) {
                Assert.assertTrue(true);
            }
        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }

        try (Service service = Service.open("unknown_service", Service.OpenAccessRights.ReadOnly)) {
            Assert.fail();
        } catch (SystemException e) {
            Assert.assertTrue(true);
        }

        Assert.assertFalse(Service.exists("unknown_service"));
        Assert.assertTrue(Service.exists("Browser"));
    }

    private void editWin() throws SystemException {
       Service.remove(Main.SERVICE_NAME);

        String path = "\"C:\\Program Files\\Java\\jdk1.8.0_144\\bin\\java.exe\" -cp \"" + Paths.get("build/libs").resolve("servicejni.jar").normalize().toAbsolutePath() + "\" com.infomaximum.service.test.Main";
        try (Service service = Service.create(Main.SERVICE_NAME, path, Service.AccountType.LocalSystem)) {
            Service.Status status = service.getStatus();
            Assert.assertFalse(status.isRunning);
            Assert.assertEquals(0, status.exitCode);

            service.start();
            service.startAsync();

            String descEtalon = "Некоторое description";
            service.setDescription(descEtalon);
            Assert.assertEquals(descEtalon, service.getDescription());

            service.sendCode(Main.USER_CODE);

            status = service.getStatus();
            Assert.assertTrue(status.isRunning);

            service.stop();
            status = service.getStatus();
            Assert.assertFalse(status.isRunning);
            assertExistsExitCodeMask(status.exitCode, Main.EXIT_CODE_MASK_STARTING);
            assertExistsExitCodeMask(status.exitCode, Main.EXIT_CODE_MASK_STOPPING);
            assertExistsExitCodeMask(status.exitCode, Main.EXIT_CODE_MASK_USER_CODE_ACCEPTED);
        } catch (SystemException e) {
            throw e;
        }

        Service.remove(Main.SERVICE_NAME);

        Assert.assertFalse(Service.exists(Main.SERVICE_NAME));
    }

    private static void assertExistsExitCodeMask(int exitCode, int mask) {
        int exitCodes = exitCode > Main.EXIT_CODE_WRAP ? exitCode - Main.EXIT_CODE_WRAP : 0;
        Assert.assertEquals(mask, (exitCodes & mask));
    }
}
