#pragma once

#include "ServiceManager.h"
#include "String.hpp"
#include "Converter.hpp"
#include "ErrorEx.h"
#include "File.hpp"
#include "CommandExecutor.h"
#include <memory>
#include <thread>
#include <unistd.h>

// TODO: переделать на cpp

// Важно!
// в SAAS версии контейнеры не поддерживают upstart, поэтому используется конфиг в init.d. При креше сервер
// автоматически не перезапускается, за этим должен следить специальный скрипт

namespace croco
{

String GetConfigDirectoryFullPath(const String& serviceName)
{
    String path;

#ifdef APPLICATION_SAAS
    path << "/etc/init.d/" << serviceName;
#else
    path << "/etc/init/" << serviceName << ".conf";
#endif

    return path;
}

String GenerateServiceConfig(const String& serviceName, const String& servicePath)
{
    String config;

#ifdef APPLICATION_SAAS
    config << "#! /bin/sh\n"\
        "PATH=/sbin:/usr/sbin:/bin:/usr/bin\n" \
        "DESC=\"" << serviceName << "\"\n"\
        "NAME=" << serviceName << "\n" \
        "DAEMON=" << servicePath << "\n" \
        "DAEMON_ARGS=\"\"\n" \
        "PIDFILE=/var/run/$NAME.pid\n" \
        "SCRIPTNAME=/etc/init.d/$NAME\n" \
        "\n" \
        "[ -x \"$DAEMON\" ] || exit 0\n" \
        "[ -r /etc/default/$NAME ] && . /etc/default/$NAME\n" \
        ". /lib/init/vars.sh\n" \
        ". /lib/lsb/init-functions\n" \
        "\n" \
        "do_start()\n" \
        "{\n" \
        "	start-stop-daemon --start --quiet --pidfile $PIDFILE --exec $DAEMON --test > /dev/null \\\n" \
        "		|| return 1\n" \
        "	start-stop-daemon --start --quiet --pidfile $PIDFILE --exec $DAEMON -- \\\n" \
        "		$DAEMON_ARGS \\\n" \
        "		|| return 2\n" \
        "}\n" \
        "\n" \
        "do_stop()\n" \
        "{\n" \
        "   start-stop-daemon --stop --oknodo --retry=TERM/30/KILL/5 --pidfile $PIDFILE \n" \
        "	RETVAL=\"$?\"\n" \
        "	[ \"$RETVAL\" = 2 ] && return 2\n" \
        "	start-stop-daemon --stop --quiet --oknodo --retry=0/30/KILL/5 --pidfile $PIDFILE\n" \
        "	[ \"$?\" = 2 ] && return 2\n" \
        "	rm -f $PIDFILE\n" \
        "	return \"$RETVAL\"\n" \
        "}\n" \
        "\n" \
        "do_reload() {\n" \
        "	start-stop-daemon --stop --signal 1 --quiet --pidfile $PIDFILE --name $NAME\n" \
        "	return 0\n" \
        "}\n" \
        "case \"$1\" in\n" \
        "  start)\n" \
        "	[ \"$VERBOSE\" != no ] && log_daemon_msg \"Starting $DESC\" \"$NAME\"\n" \
        "	do_start\n" \
        "	case \"$?\" in\n" \
        "		0|1) [ \"$VERBOSE\" != no ]Exists && log_end_msg 0 ;;\n" \
        "		2) [ \"$VERBOSE\" != no ] && log_end_msg 1 ;;\n" \
        "	esac\n" \
        "	;;\n" \
        "  stop)\n" \
        "	[ \"$VERBOSE\" != no ] && log_daemon_msg \"Stopping $DESC\" \"$NAME\"\n" \
        "	do_stop\n" \
        "	case \"$?\" in\n" \
        "		0|1) [ \"$VERBOSE\" != no ] && log_end_msg 0 ;;\n" \
        "		2) [ \"$VERBOSE\" != no ] && log_end_msg 1 ;;\n" \
        "	esac\n" \
        "	;;\n" \
        "  status)\n" \
        "	status_of_proc \"$DAEMON\" \"$NAME\" && exit 0 || exit $?\n" \
        "	;;\n" \
        "  restart|force-reload)\n" \
        "	log_daemon_msg \"Restarting $DESC\" \"$NAME\"\n" \
        "	do_stop\n" \
        "	case \"$?\" in\n" \
        "	  0|1)\n" \
        "		do_start\n" \
        "		case \"$?\" in\n" \
        "			0) log_end_msg 0 ;;\n" \
        "			1) log_end_msg 1 ;; # Old process is still running\n" \
        "			*) log_end_msg 1 ;; # Failed to start\n" \
        "		esac\n" \
        "		;;\n" \
        "	  *)\n" \
        "		log_end_msg 1\n" \
        "		;;\n" \
        "	esac\n" \
        "	;;\n" \
        "  *)\n" \
        "\n" \
        "	echo \"Usage: $SCRIPTNAME {start|stop|status|restart|force-reload}\" >&2\n" \
        "	exit 3\n" \
        "	;;\n" \
        "esac\n" \
        ":";
#else
    config << "#!upstart\n" \
        "description \"" << serviceName << "\"\n" \
        "\n" \
        "exec '" << servicePath << "'\n" \
        "start on runlevel [2345]\n" \
        "stop on runlevel [016]\n" \
        "respawn\n" \
        "respawn limit 3 20\n" \
        "expect daemon";
#endif

    return config;
}

Service::Service(const String& serviceName1)
    : serviceName(std::move(String() << serviceName1))
{
}

Service::~Service()
{
}

void Service::Start()
{
    StartAsync();

	const std::chrono::milliseconds waitTimeInMs(100);
    const int maxWaitCount = 20;
    for(int i = 0; i < maxWaitCount; ++i)
    {
		std::this_thread::sleep_for(waitTimeInMs);
        if (IsRunning())
            break;
    }
}

void Service::StartAsync()
{
    if (!IsRunning())
    {
        String command;
#ifdef APPLICATION_SAAS
        command << "'" << GetConfigDirectoryFullPath(serviceName) << "' start";
#else
        command << "start " << serviceName;
#endif
        CommandExecutor::Execute(command);
    }
}

void Service::Stop()
{
    if (IsRunning())
    {
        String command;
#ifdef APPLICATION_SAAS
        command << "'" << GetConfigDirectoryFullPath(serviceName) << "' stop";
#else
        command << "stop " << serviceName;
#endif
        CommandExecutor::Execute(command);

		const std::chrono::milliseconds waitTimeInMs(100);
        const int maxWaitCount = 20;
        for(int i = 0; i < maxWaitCount; ++i)
        {
			std::this_thread::sleep_for(waitTimeInMs);
            if (!IsRunning())
                break;
        }
    }
}

bool Service::IsRunning()
{
    return GetPid() != (DWORD)SERVICE_INVALID_PID;
}

bool Service::IsAutoRestarted(bool /*withPostFailCommand*/) const
{
    return true;
}

bool Service::SetAutoRestart(bool autoRestart, const String& /*postFailCommand*/)
{
    return autoRestart;
}

void Service::GetDescription(String& description)
{
    description.Clear();
}

void Service::SetDescription(const String& /*description*/)
{

}

void Service::SendCode(DWORD code)
{
    pid_t processId = GetPid();
    if (processId == SERVICE_INVALID_PID)
        return;

    String command;
    command << "kill -" << code << " " << processId;
    CommandExecutor::Execute(command);
}

DWORD Service::GetPid()
{
    const String command(std::move(String() << "pgrep " << serviceName));
    List<String> output;
    if (!CommandExecutor::Execute(command, output))
    {
        return SERVICE_INVALID_PID;
    }

    const pid_t currentPid = getpid();
    for (DWORD i = 0; i < output.GetCount(); ++i)
    {
        pid_t result;
        if (Converter::ConvertTo(result, *output.ItemAt(i)) && result != currentPid)
        {
            return result;
        }
    }

    return SERVICE_INVALID_PID;
}

std::unique_ptr<Service> Service::Create(const String& serviceName, const String& servicePath, ServiceAccount /*serviceAccount*/, ErrorEx& err)
{
    err.Clear();

    String configFullPath = GetConfigDirectoryFullPath(serviceName);
    if (File::Exists(configFullPath))
    {
        File::Delete(configFullPath);
    }

    String config = GenerateServiceConfig(serviceName, servicePath);
    if (!File::Write(configFullPath, config).IsSuccess())
    {
        err = ErrorEx(ErrorCode::FILE_WRITE_ERROR);
        return nullptr;
    }

#ifdef APPLICATION_SAAS
    String command;
    command << "update-rc.d " << serviceName << " defaults";
    if (!CommandExecutor::Execute(command))
    {
        err = ErrorEx(ErrorCode::NATIVE_ERROR);
        return nullptr;
    }
#endif

    return std::unique_ptr<Service>(new Service(serviceName));
}

std::unique_ptr<Service> Service::Open(const String& serviceName, ErrorEx& err)
{
    err.Clear();

    String configFullPath = GetConfigDirectoryFullPath(serviceName);
    if (!File::Exists(configFullPath))
    {
        err = ErrorEx(ErrorCode::FILE_OPEN_ERROR);
        return nullptr;
    }

    return std::unique_ptr<Service>(new Service(serviceName));
}

ErrorEx Service::Remove(const String& serviceName)
{
    Service	service(serviceName);
    service.Stop();

#ifdef APPLICATION_SAAS
    String command;
    command << "update-rc.d -f " << serviceName << " remove";
    if (!CommandExecutor::Execute(command))
    {
        return ErrorEx(ErrorCode::NATIVE_ERROR);
    }
#endif

    File::Delete(GetConfigDirectoryFullPath(serviceName));

    return ErrorEx();
}


} // namespace croco
