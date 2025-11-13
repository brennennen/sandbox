
/** Common */
mod server_metrics;

/** Linux */
#[cfg(target_os = "linux")]
mod linux_server;

#[cfg(target_os = "linux")]
pub struct Server
{
    pub port: u16,
    pub process_request_callback: fn(crate::Request) -> crate::Response,
    pub periodic_callback: fn(),
    pub server_metrics: crate::server::server_metrics::ServerMetrics,
    linux_server: linux_server::LinuxServer
}

#[cfg(target_os = "linux")]
impl Server {
    pub fn new(port: u16,
                process_request_callback: fn(crate::Request) -> crate::Response,
                periodic_callback: fn()
        ) -> Server {
        Server {
            port: port,
            process_request_callback: process_request_callback,
            periodic_callback: periodic_callback,
            server_metrics: crate::server::server_metrics::ServerMetrics::new(),
            linux_server: linux_server::LinuxServer::new(port, process_request_callback, periodic_callback)
        }
    }

    pub fn start(&mut self) {
        self.linux_server.start();
    }
}

/** Windows */
#[cfg(target_os = "windows")]
mod windows_server;

#[cfg(target_os = "windows")]
pub struct Server
{
    pub port: u16,
    pub process_request_callback: fn(crate::Request) -> crate::Response,
    pub periodic_callback: fn(),
    pub server_metrics: crate::server::server_metrics::ServerMetrics,
    windows_server: windows_server::WindowsServer
}

#[cfg(target_os = "windows")]
impl Server {
    pub fn new(port: u16,
                process_request_callback: fn(crate::Request) -> crate::Response,
                periodic_callback: fn()
        ) -> Server {
        Server {
            port: port,
            process_request_callback: process_request_callback,
            periodic_callback: periodic_callback,
            server_metrics: crate::server::server_metrics::ServerMetrics::new(),
            windows_server: windows_server::WindowsServer::new(port, process_request_callback, periodic_callback)
        }
    }

    pub fn start(&mut self) {
        self.windows_server.start();
    }
}
