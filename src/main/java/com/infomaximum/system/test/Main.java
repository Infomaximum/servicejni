package com.infomaximum.system.test;

import com.infomaximum.system.service.ServiceEntry;
import com.infomaximum.system.SystemException;

public class Main {

    public static final String SERVICE_NAME = "_test_java_service";
    public static final int EXIT_CODE_WRAP = 20000;
    public static final int USER_CODE = 192;
    public static final int EXIT_CODE_MASK_STARTING = 1;
    public static final int EXIT_CODE_MASK_STOPPING = 2;
    public static final int EXIT_CODE_MASK_USER_CODE_ACCEPTED = 4;

    private static int exitCodes = 0;

    static public void main(String[] args) {

        try (ServiceEntry serviceEntry = new ServiceEntry(SERVICE_NAME)) {
            serviceEntry.registerEventHandler(ServiceEntry.Event.STARTING, () -> serviceEntry.setExitCode(buildExitCode(EXIT_CODE_MASK_STARTING)));
            serviceEntry.registerEventHandler(ServiceEntry.Event.STOPPING, () -> serviceEntry.setExitCode(buildExitCode(EXIT_CODE_MASK_STOPPING)));
            serviceEntry.registerEventHandler(USER_CODE, () -> serviceEntry.setExitCode(buildExitCode(EXIT_CODE_MASK_USER_CODE_ACCEPTED)));

            serviceEntry.execute();
        } catch (SystemException e) {
            e.printStackTrace();
        }

        System.exit(0);
    }

    private static int buildExitCode(int code) {
        exitCodes |= code;
        return EXIT_CODE_WRAP + exitCodes;
    }
}
