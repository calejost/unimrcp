/*
 * Copyright 2008 Arsen Chaloyan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <windows.h>
#include "unimrcp_server.h"
#include "apt_log.h"

#define WIN_SERVICE_NAME "unimrcp"

static SERVICE_STATUS_HANDLE win_service_status_handle = NULL;
static SERVICE_STATUS win_service_status;

static mrcp_server_t *server = NULL;
static const char *conf_dir = NULL;
static const char *plugin_dir = NULL;

/** SCM state change handler */
static void WINAPI win_service_handler(DWORD control)
{
	apt_log(APT_PRIO_INFO,"Service Handler %d",control);
	switch (control)
	{
		case SERVICE_CONTROL_INTERROGATE:
			break;
		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			if(server) {
				win_service_status.dwCurrentState = SERVICE_STOP_PENDING; 
				if(!SetServiceStatus(win_service_status_handle, &win_service_status)) { 
					apt_log(APT_PRIO_WARNING,"Failed to Set Service Status %d",GetLastError());
				}

				/* shutdown server */
				unimrcp_server_shutdown(server);
			}
			win_service_status.dwCurrentState = SERVICE_STOPPED; 
			win_service_status.dwCheckPoint = 0; 
			win_service_status.dwWaitHint = 0; 
			break;
	}

	if(!SetServiceStatus(win_service_status_handle, &win_service_status)) {
		apt_log(APT_PRIO_WARNING,"Failed to Set Service Status %d",GetLastError());
	}
}

static void WINAPI win_service_main(DWORD argc, LPTSTR *argv)
{
	apt_log(APT_PRIO_INFO,"Service Main");
	win_service_status.dwServiceType = SERVICE_WIN32;
	win_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	win_service_status.dwWin32ExitCode = 0;
	win_service_status.dwServiceSpecificExitCode = 0;
	win_service_status.dwCheckPoint = 0;
	win_service_status.dwWaitHint = 0;

	win_service_status_handle = RegisterServiceCtrlHandler(WIN_SERVICE_NAME, win_service_handler);
	if (win_service_status_handle == (SERVICE_STATUS_HANDLE)0) {
		apt_log(APT_PRIO_WARNING,"Failed to Register Service Control Handler %d",GetLastError());
		return;
	} 

	win_service_status.dwCurrentState = SERVICE_START_PENDING;
	if(!SetServiceStatus(win_service_status_handle, &win_service_status)) {
		apt_log(APT_PRIO_WARNING,"Failed to Set Service Status %d",GetLastError());
	} 

	/* start server */
	server = unimrcp_server_start(conf_dir, plugin_dir);

	win_service_status.dwCurrentState =  server ? SERVICE_RUNNING : SERVICE_STOPPED;
	if(!SetServiceStatus(win_service_status_handle, &win_service_status)) {
		apt_log(APT_PRIO_WARNING,"Failed to Set Service Status %d",GetLastError());
	} 
}

/** Register/install service in SCM */
apt_bool_t uni_service_register(apr_pool_t *pool)
{
	char file_path[MAX_PATH];
	char *bin_path;
	SC_HANDLE sch_service;
	SC_HANDLE sch_manager = OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
	if(!sch_manager) {
		apt_log(APT_PRIO_WARNING,"Failed to Open SCManager %d", GetLastError());
		return FALSE;
	}

	if(!GetModuleFileName(NULL,file_path,MAX_PATH)) {
		return FALSE;
	}
	bin_path = apr_psprintf(pool,"%s --service",file_path);
	sch_service = CreateService(
					sch_manager,
					WIN_SERVICE_NAME,
					"UniMRCP Server",
					GENERIC_EXECUTE,
					SERVICE_WIN32_OWN_PROCESS,
					SERVICE_DEMAND_START,
					SERVICE_ERROR_NORMAL,
					bin_path,0,0,0,0,0);
	if(sch_service) {
		CloseServiceHandle(sch_service);
	}
	else {
		apt_log(APT_PRIO_WARNING,"Failed to Create Service %d", GetLastError());
	}
	CloseServiceHandle(sch_manager);
	return TRUE;
}

/** Unregister/uninstall service from SCM */
apt_bool_t uni_service_unregister()
{
	SC_HANDLE sch_service;
	SC_HANDLE sch_manager = OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
	if(!sch_manager) {
		apt_log(APT_PRIO_WARNING,"Failed to Open SCManager %d", GetLastError());
		return FALSE;
	}

	sch_service = OpenService(sch_manager,WIN_SERVICE_NAME,DELETE|SERVICE_STOP);
	if(sch_service) {
		ControlService(sch_service,SERVICE_CONTROL_STOP,0);
		DeleteService(sch_service);
		CloseServiceHandle(sch_service);
	}
	else {
		apt_log(APT_PRIO_WARNING,"Failed to Open Service %d", GetLastError());
	}
	CloseServiceHandle(sch_manager);
	return TRUE;
}

/** Run SCM service */
apt_bool_t uni_service_run(const char *conf_dir_path, const char *plugin_dir_path, apr_pool_t *pool)
{
	SERVICE_TABLE_ENTRY win_service_table[] = {
		{ WIN_SERVICE_NAME, win_service_main },
		{ NULL, NULL }
	};

	conf_dir = conf_dir_path;
	plugin_dir = plugin_dir_path;

	apt_log(APT_PRIO_INFO,"Run as Service");
	if(!StartServiceCtrlDispatcher(win_service_table)) {
		apt_log(APT_PRIO_WARNING,"Failed to Connect to SCM %d",GetLastError());
	}
	return TRUE;
}
